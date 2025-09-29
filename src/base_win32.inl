#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DWORD_MAX 0xffffffffUL


// =================================
// ======= Custom Allocators =======
// =================================

HeapAllocator::HeapAllocator()
{
	m_heap = GetProcessHeap();
}

NoType* HeapAllocator::Alloc(Usize size)
{
	return HeapAlloc(m_heap, 0, size);
}

NoType* HeapAllocator::Realloc(NoType* ptr, Usize size)
{
	return HeapReAlloc(m_heap, 0, ptr, size);
}

NoType HeapAllocator::Free(NoType* ptr)
{
	HeapFree(m_heap, 0, ptr);
}


// =======================
// ======= Threads =======
// =======================

Thread::Thread(ThreadFunc thrdFunc, NoType* data, Bool start)
{
   	DWORD flags = (start) ? 0 : CREATE_SUSPENDED;
	m_handle = CreateThread(nullptr, 0, thrdFunc, data, flags, nullptr);
}

Thread::~Thread()
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

Mutex::~Mutex()
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
    if (GetFileSizeEx(file, &_fileSize))
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
            buffer.~Slice();
            break;
        }

        totalRead += static_cast<Usize>(size);
        LARGE_INTEGER offset = { .QuadPart = static_cast<LONGLONG>(size) };
        SetFilePointerEx(file, offset, nullptr, FILE_CURRENT);
    }

    CloseHandle(handle);
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
        DWORD size = static_cast<DWORD>(remaining, 0, DWORD_MAX);
        if (!WriteFile(file, &buffer.m_data[totalWritten], size, nullptr, nullptr))
            return false;

        totalWritten += static_cast<Usize>(size);
        LARGE_INTEGER offset = { .QuadPart = static_cast<LONGLONG>(size) };
        SetFilePointerEx(file, offset, nullptr, FILE_CURRENT);
    }

    CloseHandle(file);
    return true;
}

String8 GetExePath(IAllocator* allocator)
{
    String8 path = String8(allocator, MAX_PATH);
    if (!GetModuleFileNameA(nullptr, path.CString(), MAX_PATH))
        path.~String8();

    return path;
}

static String8 _GetEnv(const C8* env, IAllocator* allocator)
{
    String8 result = String8(allocator, MAX_PATH);
    if (GetEnvironmentVariable(env, result.CString(), MAX_PATH) == 0)
        result.~String8();

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
	C8 finalMsg[512];
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
