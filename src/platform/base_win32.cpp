#include "base_win32.hpp"


// =================================
// ======= Custom Allocators =======
// =================================

HeapAllocator::HeapAllocator()
{
}

NoType* HeapAllocator::Alloc(Usize size)
{
	return HeapAlloc(GetProcessHeap(), 0, size);
}

NoType* HeapAllocator::Realloc(NoType* ptr, Usize size)
{
	return HeapReAlloc(GetProcessHeap(), 0, ptr, size);
}

NoType HeapAllocator::Free(NoType* ptr)
{
	HeapFree(GetProcessHeap(), 0, ptr);
}


// =======================
// ======= Threads =======
// =======================

Thread::Thread(ThreadFunc thrdFunc, NoType* data, Bool start)
{
	DWORD flags = (start) ? 0 : CREATE_SUSPENDED;
	m_handle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(uintptr_t) thrdFunc, data, flags, nullptr);
}

NoType Thread::Release()
{
	CloseHandle(m_handle);
}

NoType Thread::Run()
{
	ResumeThread(m_handle);
}

NoType Thread::Stop()
{
	SuspendThread(m_handle);
}

NoType Thread::Join()
{
	WaitForSingleObject(m_handle, INFINITE);
}

NoType Thread::AssignCore(U32 core)
{
	SetThreadIdealProcessor(m_handle, core);
}

NoType Thread::JoinMultiple(Thread* thrds, U32 thrdCount)
{
	const HANDLE* handles = reinterpret_cast<const HANDLE*>(thrds);
	const DWORD numOfThrds = static_cast<DWORD>(thrdCount);
	WaitForMultipleObjects(numOfThrds, handles, TRUE, INFINITE);
}

NoType Thread::Exit(U32 code)
{
	ExitThread(code);
}

Mutex::Mutex(U32 waitSpin)
{
	DWORD spinCount = static_cast<DWORD>(waitSpin);
	InitializeCriticalSectionAndSpinCount(&m_critSec, spinCount);
}

NoType Mutex::Release()
{
	DeleteCriticalSection(&m_critSec);
}

NoType Mutex::Lock()
{
	EnterCriticalSection(&m_critSec);
}

Bool Mutex::TryLock()
{
	return TryEnterCriticalSection(&m_critSec) == TRUE;
}

NoType Mutex::Unlock()
{
	LeaveCriticalSection(&m_critSec);
}


// ==========================
// ======= Filesystem =======
// ==========================

static FileError _GetFileError()
{
	FileError error;

	// TODO: Error handling for file operations
	switch (GetLastError())
	{
		case ERROR_SUCCESS:
			error = FileError::none;
			break;
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_NAME:
		case ERROR_BAD_PATHNAME:
		// TODO: I'm not sure about the two following cases
		case ERROR_CANT_ACCESS_FILE:
		case ERROR_CANT_RESOLVE_FILENAME:
			error = FileError::notFound;
			break;
		case ERROR_ACCESS_DENIED:
		case ERROR_FILE_ENCRYPTED:
		case ERROR_FILE_READ_ONLY:
			error = FileError::noPerm;
			break;
		case ERROR_FILE_EXISTS:
		case ERROR_ALREADY_EXISTS:
			error = FileError::exists;
			break;
		case ERROR_SHARING_VIOLATION:
		case ERROR_PATH_BUSY:
		case ERROR_BUSY:
			error = FileError::busy;
			break;
		default:
			error = FileError::unknown;
			break;
	}

	return error;
}

File::File()
{
	m_error = FileError::none;
	m_handle = INVALID_HANDLE_VALUE;
}

File::File(String8 path, FileFlags flags)
{
	this->Open(path, flags);
}

FileError File::Open(String8 path, FileFlags flags)
{
	DWORD desiredAccess = 0;
	DWORD shareMode = 0;
	DWORD creationDisposition = 0;
	SECURITY_ATTRIBUTES securityAttributes = { SIZE_OF(SECURITY_ATTRIBUTES), nullptr, FALSE };

	if (FlagCheck(flags, FileFlags::read))
		desiredAccess |= FILE_GENERIC_READ;
	if (FlagCheck(flags, FileFlags::write) || FlagCheck(flags, FileFlags::append))
		desiredAccess |= FILE_GENERIC_WRITE;
	if (FlagCheck(flags, FileFlags::append))
		desiredAccess |= FILE_APPEND_DATA;

	if (FlagCheck(flags, FileFlags::shareRead))
		shareMode |= FILE_SHARE_READ;
	if (FlagCheck(flags, FileFlags::shareWrite))
		shareMode |= FILE_SHARE_WRITE;
	if (FlagCheck(flags, FileFlags::inheritable))
		securityAttributes.bInheritHandle = TRUE;

	if (FlagCheck(flags, FileFlags::create))
		creationDisposition = CREATE_ALWAYS;
	else
		creationDisposition = OPEN_EXISTING;

	m_handle = CreateFileA(
		path.CString(),
		desiredAccess, shareMode,
		&securityAttributes,
		creationDisposition, FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	m_error = _GetFileError();
	return m_error;
}

NoType File::Close()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
		m_error = FileError::none;
		m_handle = INVALID_HANDLE_VALUE;
	}
}

FileError File::GetError()
{
	return m_error;
}

FileError File::Read(NoType* buff, Usize size)
{
	Byte* buffer = static_cast<Byte*>(buff);
	Usize sizetoRead = Clamp(size, 0, this->GetSize());
	Usize originalPos = this->GetPos();

	// NOTE: Since 'ReadFile()' only takes a 32bit int as input (DWORD),
	// we have to do multiple calls to it until we actually have read the
	// entire file, if we want to support files larger than 4gb
	Usize totalRead = 0;
	while (totalRead < sizetoRead)
	{
		Usize remaining = sizetoRead - totalRead;
		DWORD readSize = static_cast<DWORD>(Clamp(remaining, 0, DWORD_MAX));
		if (!ReadFile(m_handle, &buffer[totalRead], readSize, nullptr, nullptr))
		{
			m_error = _GetFileError();
			break;
		}

		totalRead += static_cast<Usize>(readSize);
		this->SetPosRelative(readSize);
	}

	this->SetPos(originalPos);
	return m_error;
}

FileError File::Write(NoType* buff, Usize size)
{
	Byte* buffer = static_cast<Byte*>(buff);
	Usize originalPos = this->GetPos();

	// NOTE: Since 'WriteFile()' only takes a 32bit int as input (DWORD),
	// we have to do multiple calls to it until the entire file is written,
	// if we want to support buffers larger than 4gb
	Usize totalWritten = 0;
	while (totalWritten < size)
	{
		Usize remaining = size - totalWritten;
		DWORD writeSize = static_cast<DWORD>(Clamp(remaining, 0, DWORD_MAX));
		if (!WriteFile(m_handle, &buffer[totalWritten], writeSize, nullptr, nullptr))
		{
			m_error = _GetFileError();
			break;
		}

		totalWritten += static_cast<Usize>(writeSize);
		this->SetPosRelative(writeSize);
	}

	this->SetPos(originalPos);
	return m_error;
}

Usize File::GetSize()
{
	LARGE_INTEGER fileSize = {};
	GetFileSizeEx(m_handle, &fileSize);
	m_error = _GetFileError();
	return static_cast<Usize>(fileSize.QuadPart);
}

Usize File::GetPos()
{
	LONG offset = 0;
	SetFilePointer(m_handle, 0, &offset, FILE_CURRENT);
	m_error = _GetFileError();
	return offset;
}

NoType File::SetPos(Usize pos)
{
	LONG offset = static_cast<LONG>(pos);
	SetFilePointer(m_handle, offset, nullptr, FILE_BEGIN);
	m_error = _GetFileError();
}

NoType File::SetPosRelative(Usize pos)
{
	LONG offset = static_cast<LONG>(pos);
	SetFilePointer(m_handle, offset, nullptr, FILE_CURRENT);
	m_error = _GetFileError();
}

Slice<Byte> ReadEntireFile(String8 path, IAllocator* allocator)
{
	HANDLE file = CreateFileA(
		path.CString(),
		GENERIC_READ, FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (file == INVALID_HANDLE_VALUE)
		return Slice<Byte>(nullptr, nullptr, 0);

	LARGE_INTEGER _fileSize;
	if (!GetFileSizeEx(file, &_fileSize))
	{
		CloseHandle(file);
		return Slice<Byte>(nullptr, nullptr, 0);
	}

	Usize fileSize = static_cast<Usize>(_fileSize.QuadPart);
	Slice<Byte> buffer = Slice<Byte>(allocator, fileSize);

	// NOTE: Since 'ReadFile()' only takes a 32bit int as input (DWORD),
	// we have to do multiple calls to it until we actually have read the
	// entire file, if we want to support files larger than 4gb
	Usize totalRead = 0;
	while (totalRead < fileSize)
	{
		Usize remaining = fileSize - totalRead;
		DWORD size = static_cast<DWORD>(Clamp(remaining, 0, DWORD_MAX));
		if (!ReadFile(file, &buffer.m_data[totalRead], size, nullptr, nullptr))
		{
			buffer.Release();
			break;
		}

		totalRead += static_cast<Usize>(size);
		LARGE_INTEGER offset = { .QuadPart = static_cast<LONGLONG>(size) };
		SetFilePointerEx(file, offset, nullptr, FILE_CURRENT);
	}

	CloseHandle(file);
	return buffer;
}

Bool WriteEntireFile(String8 path, Slice<Byte> buffer)
{
	HANDLE file = CreateFileA(
		path.CString(),
		GENERIC_WRITE, FILE_SHARE_WRITE,
		nullptr,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	// NOTE: Since 'WriteFile()' only takes a 32bit int as input (DWORD),
	// we have to do multiple calls to it until the entire file is written,
	// if we want to support buffers larger than 4gb
	Usize totalWritten = 0;
	while (totalWritten < buffer.Size())
	{
		Usize remaining = buffer.Size() - totalWritten;
		DWORD size = static_cast<DWORD>(Clamp(remaining, 0, DWORD_MAX));
		if (!WriteFile(file, &buffer.m_data[totalWritten], size, nullptr, nullptr))
			break;

		totalWritten += static_cast<Usize>(size);
		LARGE_INTEGER offset = { .QuadPart = static_cast<LONGLONG>(size) };
		SetFilePointerEx(file, offset, nullptr, FILE_CURRENT);
	}

	CloseHandle(file);
	return totalWritten == buffer.Size();
}

String8 GetExePath(IAllocator* allocator)
{
	String8 path = String8(allocator, MAX_PATH);
	if (!GetModuleFileNameA(nullptr, path.CString(), MAX_PATH))
		path.Release();

	return path;
}

static String8 _GetEnv(const C8* env, IAllocator* allocator)
{
	String8 result = String8(allocator, MAX_PATH);
	if (GetEnvironmentVariable(env, result.CString(), MAX_PATH) == 0)
		result.Release();

	return result;
}

String8 GetUserDir(IAllocator* allocator)
{
	return _GetEnv("USERPROFILE", allocator);
}

String8 GetConfigDir(IAllocator* allocator)
{
	return _GetEnv("APPDATA", allocator);
}


// =============================
// ======= Miscellaneous =======
// =============================

void _AssertRel(const C8* msg, const C8* file, const U32 line)
{
	C8 finalMsg[512] = {};
	wsprintfA(finalMsg, "%s\n%s:%u\n", msg, file, line);
	MessageBox(nullptr, finalMsg, nullptr, MB_OK | MB_ICONERROR | MB_TASKMODAL);

	ExitProcess(1);
}

#if defined(WIN32_CONSOLE_MODE)
int main(int argc, char* argv[])
{
	return EntryPoint(argc, argv);
}
#elif defined(WIN32_WINDOWS_MODE)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	(void) hInstance;
	(void) hPrevInstance;
	(void) lpCmdLine;
	(void) nShowCmd;
	return EntryPoint(0, nullptr);
}
#endif
