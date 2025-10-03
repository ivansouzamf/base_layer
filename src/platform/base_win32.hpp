#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// =======================
// ======= Threads =======
// =======================

struct Thread
{
	Thread(ThreadFunc thrdFunc, NoType* data, Bool start = false);
	NoType Release();

	NoType Run();
	NoType Stop();
	NoType Join();
	NoType AssignCore(U32 core);

	static NoType JoinMultiple(Thread* thrds, U32 thrdCount);
	static NoType Exit(U32 code = 0);

	HANDLE m_handle;
};

struct Mutex
{
	Mutex(U32 waitSpin = 32);
	NoType Release();

	NoType Lock();
	Bool TryLock();
	NoType Unlock();

	CRITICAL_SECTION m_critSec;
};
