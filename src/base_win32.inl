#define WIN32_LEAN_AND_MEAN
#include <windows.h>


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

NoType Stop()
{
    SuspendThread(m_handle);
}

NoType Join()
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
