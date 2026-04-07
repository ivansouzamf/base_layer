#include "base.hpp"

namespace Bl
{
// =================================
// ======= Custom Allocators =======
// =================================

ArenaAllocator::ArenaAllocator(NoType* buffer, Usize size)
{
	m_allocator = nullptr;
	m_buffer = static_cast<Byte*>(buffer);
	m_size = size;
	m_current = 0;
}

ArenaAllocator::ArenaAllocator(IAllocator* allocator, Usize size)
{
	m_allocator = allocator;
	m_buffer = static_cast<Byte*>(allocator->Alloc(size));
	m_size = size;
	m_current = 0;
}

NoType ArenaAllocator::Release()
{
	if (m_allocator != nullptr)
	{
		m_allocator->Free(m_buffer);
	}

	m_allocator = nullptr;
	m_buffer = nullptr;
	m_size = 0;
	m_current = 0;
}

NoType* ArenaAllocator::Alloc(Usize size)
{
	ASSERT(size + m_current <= m_size, "Trying to allocate more than the size of the Arena");
	NoType* ptr = &m_buffer[m_current];
	m_current += size;

	return ptr;
}

NoType* ArenaAllocator::Realloc(NoType*, Usize)
{
	ASSERT(false, "ArenaAllocator doesn't support 'Realloc()'");
	return nullptr;
}

NoType ArenaAllocator::Free(NoType*)
{
	// NOTE: 'ArenaAllocator' doesn't do free operations, it can
	// only free the entire buffer
	return;
}

NoType ArenaAllocator::FreeAll()
{
	m_current = 0;
}


// =======================
// ======= Strings =======
// =======================

String8::String8(IAllocator* allocator, C8* data, Usize length)
{
	m_allocator = allocator;
	m_data = data;
	m_length = length;
}

String8::String8(IAllocator* allocator, Usize length)
{
	m_allocator = allocator;
	m_data = static_cast<C8*>(allocator->Alloc(length + 1));
	m_length = length;
}

String8::String8(const C8* cstring)
{
	m_allocator = nullptr;
	m_data = const_cast<C8*>(cstring);
	m_length = CStringLen(cstring);
}

NoType String8::Release()
{
	if (m_allocator != nullptr)
	{
		m_allocator->Free(m_data);
	}

	m_allocator = nullptr;
	m_data = nullptr;
	m_length = 0;
}

NoType String8::operator=(const C8* cstring)
{
	*this = String8(cstring);
}

Bool String8::operator==(String8 string)
{
	return MemoryCompare(m_data, string.m_data, m_length);
}

Bool String8::operator!=(String8 string)
{
	return !MemoryCompare(m_data, string.m_data, m_length);
}

NoType String8::Reverse()
{
	Usize start = 0;
	Usize end = m_length - 1;

	while (start < end)
	{
		C8 temp = m_data[start];
		m_data[start] = m_data[end];
		m_data[end] = temp;
		start += 1;
		end -= 1;
	}
}

String8 String8::Join(String8 string, IAllocator* allocator)
{
	if (allocator == nullptr)
	{
		ASSERT(m_allocator != nullptr, "Missing allocator");
		allocator = m_allocator;
	}

	Usize length = m_length + string.m_length;
	String8 newStr = String8(allocator, length);
	MemoryCopy(newStr.m_data, m_data, m_length);
	MemoryCopy(&newStr.m_data[m_length], string.m_data, string.m_length);
	newStr.m_data[length] = '\0';

	return newStr;
}

String8 String8::Clone(IAllocator* allocator)
{
	if (allocator == nullptr)
	{
		ASSERT(m_allocator != nullptr, "Missing allocator");
		allocator = m_allocator;
	}

	String8 newStr = String8(allocator, m_length);
	MemoryCopy(newStr.m_data, m_data, m_length);

	return newStr;
}


// ====================
// ======= Time =======
// ====================

NoType PreciseSleep(U64 us)
{
	const U64 freq = GetPerformanceFrequency();
	const U64 timeInFreq = us * (freq / 1000000);

	U64 sleepTime = GetPerformanceCounter();

	if (us > 2500)
	{
#if defined(BASE_OS_WIN32)
		NormalSleep((us / 1000) - 1);
#elif defined(BASE_OS_LINUX)
		NormalSleep(us / 1000);
#endif
	}

	U64 currTime = GetPerformanceCounter();
	U64 lastTime = currTime;
	sleepTime = currTime - sleepTime;

	if (sleepTime > timeInFreq)
	{
		return;
	}

	while (currTime - lastTime < timeInFreq - sleepTime)
	{
		currTime = GetPerformanceCounter();
		Thread::Yield();
	}
}

NoType SpinlockSleep(U64 us)
{
	const U64 freq = GetPerformanceFrequency();
	const U64 last = GetPerformanceCounter();
	U64 curr = last;

	while ((curr - last) / (freq / 1000000) < us)
	{
		curr = GetPerformanceCounter();
		Thread::Yield();
	}
}


// ==========================
// ======= Filesystem =======
// ==========================

String8 ReadEntireFileAsString(String8 path, IAllocator* allocator)
{
	Slice<Byte> buffer = ReadEntireFile(path, allocator);
	return String8(allocator, reinterpret_cast<C8*>(buffer.m_data), buffer.Size());
}

String8 GetDirFromPath(String8 path, IAllocator* allocator)
{
	Usize lastSlash = 0;
	for (Usize i = 0; i < path.Length(); i += 1)
	{
		if (path[i] == BASE_PATH_SEPARATOR)
		{
			lastSlash = i;
		}
	}

	String8 dir = path.Clone(allocator);
	dir.m_length = lastSlash;
	dir[lastSlash] = '\0';

	return dir;
}
} // namespacee Bl


// ------- OS Includes -------
S32 EntryPoint(S32 argc, C8* argv[]);
#if defined(BASE_OS_WIN32)
	#include "platform/base_win32.cpp"
#elif defined(BASE_OS_LINUX)
	#include "platform/base_linux.cpp"
#endif

// ------- C++ Garbage -------
extern "C"
{
	void __cxa_pure_virtual()
	{
	}
}
