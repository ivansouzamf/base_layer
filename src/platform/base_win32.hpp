#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DWORD_MAX 0xffffffffUL


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


// ==========================
// ======= Filesystem =======
// ==========================

struct File
{
	File();
	File(String8 path, FileFlags flags = FileFlags::rdonly);
	FileError Open(String8 path, FileFlags flags);
	NoType Close();
	FileError GetError();

	FileError Read(NoType* buff, Usize size);
	FileError Write(NoType* buff, Usize size);

	Usize GetSize();
	Usize GetPos();
	NoType SetPos(Usize pos);
	NoType SetPosRelative(Usize pos);

	HANDLE m_handle;
	FileError m_error;
};

struct Directory
{
};
