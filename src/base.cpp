#include "base.hpp"


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

ArenaAllocator::~ArenaAllocator()
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
	ASSERT(size <= m_current, "Trying to allocate more than the size of the Arena");
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

String8::~String8()
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
	this->~String8();
	*this = String8(cstring);
}

NoType String8::operator=(String8 string)
{
	this->~String8();
	m_allocator = string.m_allocator;
	m_data = string.m_data;
	m_length = string.m_length;
}

Bool String8::operator==(String8 string)
{
	return MemoryCompare(m_data, string.m_data, m_length);
}

Bool String8::operator!=(String8 string)
{
	return !MemoryCompare(m_data, string.m_data, m_length);
}

C8& String8::operator[](Usize i)
{
	ASSERT(i < m_length, "Trying to access element out of bounds");
	return m_data[i];
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

C8* String8::CString()
{
	return m_data;
}

Usize String8::Length()
{
	return m_length;
}


// ------- OS Includes -------
S32 EntryPoint(S32 argc, C8* argv[]);
#if defined(BASE_OS_WIN32)
    #include "base_win32.inl"
#elif defined(BASE_OS_LINUX)
    #include "base_linux.inl"
#endif


// ------- C++ Garbage -------
extern "C"
{
    void __cxa_pure_virtual()
    {
    }
}
