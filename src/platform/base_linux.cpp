#include "base_linux.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <pthread.h>
#include <time.h>


namespace Bl
{
// =================================
// ======= Custom Allocators =======
// =================================

HeapAllocator::HeapAllocator()
{
}

NoType* HeapAllocator::Alloc(Usize size)
{
	return malloc(size);
}

NoType* HeapAllocator::Realloc(NoType* ptr, Usize size)
{
	return realloc(ptr, size);
}

NoType HeapAllocator::Free(NoType* ptr)
{
	free(ptr);
}


// =======================
// ======= Threads =======
// =======================

Thread::Thread(ThreadFunc thrdFunc, NoType* data, Bool start)
{
	// TODO: We want the thread to start paused if 'start' is false
	pthread_create(&m_thread, nullptr, thrdFunc, data);
}

NoType Thread::Release()
{
	// Not needed on linux (apparently)
}

NoType Thread::Run()
{
	// TODO:
}

NoType Thread::Stop()
{
	// TODO:
}

NoType Thread::Join()
{
	pthread_join(m_thread, nullptr);
}

NoType Thread::AssignCore(U32 core)
{
	cpu_set_t cpuset = {};
	CPU_SET(static_cast<int>(core), &cpuset);
	pthread_setaffinity_np(m_thread, SIZE_OF(cpuset), &cpuset);
}

NoType Thread::JoinMultiple(Thread* thrds, U32 thrdCount)
{
	for (U32 i = 0; i < thrdCount; i += 1)
	{
		pthread_join(thrds[i].m_thread, nullptr);
	}
}

NoType Thread::Yield()
{
	sched_yield();
}

NoType Thread::Exit(U32 code)
{
	pthread_exit(reinterpret_cast<void*>(code));
}

Mutex::Mutex(U32)
{
	pthread_mutex_init(&m_mutex, nullptr);
}

NoType Mutex::Release()
{
	pthread_mutex_destroy(&m_mutex);
}

NoType Mutex::Lock()
{
	pthread_mutex_lock(&m_mutex);
}

Bool Mutex::TryLock()
{
	return pthread_mutex_trylock(&m_mutex) == 0;
}

NoType Mutex::Unlock()
{
	pthread_mutex_unlock(&m_mutex);
}


// ====================
// ======= Time =======
// ====================

U64 GetPerformanceFrequency()
{
	return 1000000000;
}

U64 GetPerformanceCounter()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return static_cast<U64>((ts.tv_sec * 1000000000) + ts.tv_nsec);
}

NoType NormalSleep(U64 ms)
{
	struct timespec ts = {};
	if (ms >= 1000)
	{
		ts.tv_sec = ms / 1000;
		ts.tv_nsec = (ms - (ts.tv_sec * 1000)) * 1000000;
	}
	else
	{
		ts.tv_nsec = ms * 1000000;
	}

	nanosleep(&ts, nullptr);
}


// ==========================
// ======= Filesystem =======
// ==========================


static FileError _GetFileError(FileError* error)
{
	// TODO: Error handling for file operations
	switch (errno)
	{
		case 0:
			*error = FileError::none;
			break;
		case EISDIR:
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			*error = FileError::notFound;
			break;
		case EACCES:
		case EPERM:
		case EROFS:
			*error = FileError::noPerm;
			break;
		case EEXIST:
			*error = FileError::exists;
			break;
		case EBUSY:
		case ETXTBSY:
			*error = FileError::busy;
			break;
		default:
			*error = FileError::unknown;
			break;
	}

	return *error;
}

File::File()
{
	m_handle = -1;
	m_error = FileError::none;
}

File::File(String8 path, FileFlags flags)
{
	this->Open(path, flags);
}

FileError File::Open(String8 path, FileFlags flags)
{
	int fileFlags = 0;

	if (FlagCheck(flags, FileFlags::rdwr))
	{
		fileFlags |= O_RDWR;
	}
	else if (FlagCheck(flags, FileFlags::read))
	{
		fileFlags |= O_RDONLY;
	}
	else if (FlagCheck(flags, FileFlags::write))
	{
		fileFlags |= O_WRONLY;
	}
	if (FlagCheck(flags, FileFlags::append))
	{
		fileFlags |= O_APPEND;
	}

	// NOTE: Linux does not support mandatory file locking,
	// therefore making it impossible to implement any of
	// the sharing flags.
	if (!FlagCheck(flags, FileFlags::inheritable))
	{
		fileFlags |= O_CLOEXEC;
	}

	// FIXME: 'open()' will fail if path contains a symlink.
	// See docs for 'O_CREAT' and 'O_EXCL'.
	if (FlagCheck(flags, FileFlags::create))
	{
		fileFlags |= O_CREAT | O_EXCL;
	}

	m_handle = open(path.CString(), fileFlags, 0);

	return _GetFileError(&m_error);
}

NoType File::Close()
{
	if (m_handle != -1)
	{
		close(m_handle);
		m_handle = -1;
		m_error = FileError::none;
	}
}

FileError File::GetError()
{
	return m_error;
}

FileError File::Read(NoType* buff, Usize size)
{
	read(m_handle, buff, size);
	return _GetFileError(&m_error);
}

FileError File::Write(NoType* buff, Usize size)
{
	write(m_handle, buff, size);
	return _GetFileError(&m_error);
}

Usize File::GetSize()
{
	Usize size = static_cast<Usize>(lseek64(m_handle, 0, SEEK_END));
	lseek64(m_handle, 0, SEEK_SET);
	return size;
}

Usize File::GetPos()
{
	Usize offset = static_cast<Usize>(lseek64(m_handle, 0, SEEK_CUR));
	lseek64(m_handle, 0, SEEK_SET);
	return offset;
}

NoType File::SetPos(Usize pos)
{
	lseek64(m_handle, static_cast<off64_t>(pos), SEEK_SET);
}

NoType File::SetPosRelative(Usize pos)
{
	lseek64(m_handle, static_cast<off64_t>(pos), SEEK_CUR);
}

Slice<Byte> ReadEntireFile(String8 path, IAllocator* allocator)
{
	int fd = open(path.CString(), O_RDONLY, 0);
	if (fd == -1)
	{
		return Slice<Byte>(nullptr, nullptr, 0);
	}

	Usize size = static_cast<Usize>(lseek64(fd, 0, SEEK_END));
	lseek64(fd, 0, SEEK_SET);

	Slice<Byte> buffer = Slice<Byte>(allocator, size);
	if (read(fd, buffer.m_data, size) == -1)
	{
		buffer.Release();
	}

	close(fd);
	return buffer;
}

Bool WriteEntireFile(String8 path, Slice<Byte> buffer)
{
	int fd = open(path.CString(), O_WRONLY, 0);
	if (fd == -1) return false;

	ssize_t written = write(fd, buffer.m_data, buffer.Size());

	close(fd);
	return written != -1;
}

String8 GetExePath(IAllocator* allocator)
{
	String8 path = String8(allocator, PATH_MAX);
	Usize pathLen = readlink("/proc/self/exe", path.CString(), PATH_MAX);
	path.m_length = pathLen;
	path.m_data[pathLen] = '\0';

	return path;
}

String8 GetUserDir(IAllocator* allocator)
{
	C8* cdir = getenv("HOME");
	if (cdir == nullptr)
	{
		return String8(nullptr, 0);
	}

	String8 dir = String8(cdir);
	return dir.Clone(allocator);
}

String8 GetConfigDir(IAllocator* allocator)
{
	// NOTE: Specified here https://specifications.freedesktop.org/basedir-spec/latest/#variables
	C8* cdir = getenv("XDG_CONFIG_HOME");
	if (cdir != nullptr)
	{
		String8 dir = String8(cdir);
		return dir.Clone(allocator);
	}

	// If $XDG_CONFIG_HOME isn't setted we default to $HOME/.config
	String8 config = "/.config";
	String8 home = GetUserDir(allocator);
	if (home.Length() == 0)
	{
		return String8(nullptr, 0);
	}

	String8 result = home.Join(config, allocator);

	home.Release();
	return result;
}
} // namespacee Bl


// =============================
// ======= Miscellaneous =======
// =============================

void _AssertRel(const C8* msg, const C8* file, const U32 line)
{
	const C8* yadCmd = "yad --title=\"Error\" --text=\"%s\" --button=OK:0 --fixed --window-icon=error --image=dialog-error --center";
	const C8* zenityCmd = "zenity --text=\"%s\" --error";
	C8* cmd = nullptr;

	C8 finalMsg[512] = {};
	C8 finalCmd[1024] = {};

	snprintf(finalMsg, SIZE_OF(finalMsg), "%s\n%s:%u\n", msg, file, line);

	if (system("yad --help > /dev/null") == 0)
	{
		cmd = const_cast<C8*>(yadCmd);
	}
	else if (system("zenity --help > /dev/null") == 0)
	{
		cmd = const_cast<C8*>(zenityCmd);
	}
	else
	{
		// If we can't display a dialog window just print it on stderr
		fprintf(stderr, "\033[31mError\033[0m: %s\n", finalMsg);
		exit(-1);
	}

	snprintf(finalCmd, SIZE_OF(finalCmd), cmd, finalMsg);
	system(finalCmd);

	exit(-1);
}

int main(int argc, char* argv[])
{
	return EntryPoint(argc, argv);
}
