#pragma once

#include <pthread.h>


// =================================
// ======= Custom Allocators =======
// =================================

struct HeapAllocator : IAllocator
{
    HeapAllocator();

	NoType* Alloc(Usize size) override;
	NoType* Realloc(NoType* ptr, Usize size) override;
	NoType Free(NoType* ptr) override;
};


// =======================
// ======= Threads =======
// =======================

struct Thread
{
	Thread(ThreadFunc thrdFunc, NoType* data, Bool start = false);
	~Thread();

	NoType Run();
	NoType Stop();
	NoType Join();
	NoType AssignCore(U32 core);

	static NoType JoinMultiple(Thread* thrds, U32 thrdCount);
	static NoType Exit(U32 code = 0);

	pthread_t m_thread;
};

struct Mutex
{
	Mutex(U32 waitSpin = 32);
	~Mutex();

	NoType Lock();
	Bool TryLock();
	NoType Unlock();

	pthread_mutex_t m_mutex;
};
