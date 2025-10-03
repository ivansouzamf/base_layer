#pragma once

#include "defines.hpp"
#include <math.h>
// Intrinsics
#if defined(BASE_CC_MSVC)
    #include <intrin.h>
#elif defined(BASE_CC_CLANG) || defined(BASE_CC_GCC)
    #include <x86intrin.h>
#endif

// ====================
// ======= Math =======
// ====================

// TODO: Maybe we should define constants as 'constexpr' instead
// of macros to avoid namespace naming conflicts
#define MATH_PI 3.14159265358979323846264338327950288
#define MATH_TAU 6.28318530717958647692528676655900576
#define MATH_LOG_TEN 2.30258509299404568401799145468436421
#define MATH_LOG_TWO 0.693147180559945309417232121458176568

// *** F32 Implementations ***
inline F32 Round(F32 num) { return roundf(num); }
inline F32 Floor(F32 num) { return floorf(num); }
inline F32 Ceil(F32 num) { return ceilf(num); }

// TODO: Overload 'Mod()' with the '%' operator
inline F32 Exp(F32 num) { return expf(num); }
inline F32 Exp2(F32 num) { return exp2f(num); }
inline F32 Log(F32 num) { return logf(num); }
inline F32 Log2(F32 num) { return log2f(num); }
inline F32 Pow(F32 base, F32 expo) { return powf(base, expo); }
inline F32 Sqrt(F32 num) { return sqrtf(num); }
inline F32 RSqrt(F32 num) { return 1.0f / sqrtf(num); }
inline F32 Cbrt(F32 num) { return cbrtf(num); }
inline F32 Mod(F32 a, F32 b) { return fmodf(a, b); }

inline F32 Sin(F32 radians) { return sinf(radians); }
inline F32 Cos(F32 radians) { return cosf(radians); }
inline F32 Arcsin(F32 sin) { return asinf(sin); }
inline F32 Arccos(F32 cos) { return acosf(cos); }
inline F32 Tan(F32 radians) { return tanf(radians); }
inline F32 Arctan(F32 tan) { return atanf(tan); }
inline F32 Arctan2(F32 y, F32 x) { return atan2f(y, x); }
inline F32 ToRadians(F32 degrees) { return degrees * MATH_TAU / 360.0f; }
inline F32 ToDegrees(F32 radians) { return radians * 360.0f / MATH_TAU; }

// *** F64 Implementations ***
inline F64 Round(F64 num) { return round(num); }
inline F64 Floor(F64 num) { return floor(num); }
inline F64 Ceil(F64 num) { return ceil(num); }

// TODO: Overload 'Mod()' with the '%' operator
inline F64 Exp(F64 num) { return exp(num); }
inline F64 Exp2(F64 num) { return exp2(num); }
inline F64 Log(F64 num) { return log(num); }
inline F64 Log2(F64 num) { return log2(num); }
inline F64 Pow(F64 base, F64 expo) { return pow(base, expo); }
inline F64 Sqrt(F64 num) { return sqrt(num); }
inline F64 RSqrt(F64 num) { return 1.0 / sqrt(num); }
inline F64 Cbrt(F64 num) { return cbrt(num); }
inline F64 Mod(F64 a, F64 b) { return fmod(a, b); }

inline F64 Sin(F64 radians) { return sin(radians); }
inline F64 Cos(F64 radians) { return cos(radians); }
inline F64 Arcsin(F64 sin) { return asin(sin); }
inline F64 Arccos(F64 cos) { return acos(cos); }
inline F64 Tan(F64 radians) { return tan(radians); }
inline F64 Arctan(F64 tan) { return atan(tan); }
inline F64 Arctan2(F64 y, F64 x) { return atan2(y, x); }
inline F64 ToRadians(F64 degrees) { return degrees * MATH_TAU / 360.0; }
inline F64 ToDegrees(F64 radians) { return radians * 360.0 / MATH_TAU; }

union Vec2F
{
    // TODO:
};

union Vec3F
{
    // TODO:
};

union Vec4F
{
    // TODO:
};


// =================================
// ======= Custom Allocators =======
// =================================

struct IAllocator
{
	virtual NoType* Alloc(Usize size) = 0;

	NoType* AllocAligned(Usize aligment, Usize size)
	{
		return Alloc(AlignPow2(aligment, size));
	}

	virtual NoType* Realloc(NoType* ptr, Usize size) = 0;

	NoType* ReallocAligned(Usize aligment, NoType* ptr, Usize size)
	{
		return Realloc(ptr, AlignPow2(aligment, size));
	}

	virtual NoType Free(NoType* ptr) = 0;
};

// Implemented per OS
struct HeapAllocator;

struct ArenaAllocator : IAllocator
{
	ArenaAllocator(NoType* buffer, Usize size);
	ArenaAllocator(IAllocator* allocator, Usize size);
	NoType Release();

	NoType* Alloc(Usize size) override;
	NoType* Realloc(NoType* ptr, Usize size) override;
	NoType Free(NoType* ptr) override;
	NoType FreeAll();

	IAllocator* m_allocator;
	Byte* m_buffer;
	Usize m_size;
	Usize m_current;
};


// =======================
// ======= Strings =======
// =======================

struct String8
{
	String8(IAllocator* allocator, C8* data, Usize length);
	String8(IAllocator* allocator, Usize length);
	String8(const C8* cstring);
	NoType Release();

	NoType operator=(const C8* cstring);
	NoType operator=(String8 string);
	Bool operator==(String8 string);
	Bool operator!=(String8 string);
	C8& operator[](Usize i);

	NoType Reverse();
	String8 Join(String8 string, IAllocator* allocator = nullptr);
	String8 Clone(IAllocator* allocator = nullptr);
	C8* CString();
	Usize Length();

	IAllocator* m_allocator;
	C8* m_data;
	Usize m_length;
};


// ===============================
// ======= Data Structures =======
// ===============================

template <typename T>
struct DynArray
{
	DynArray(IAllocator* allocator, Usize size)
	{
		m_allocator = allocator;
		m_data = static_cast<T*>(allocator->Alloc(SIZE_OF(T) * size));
		m_length = 0;
		m_reserved = size;
	}

	NoType Release()
	{
		m_allocator->Free(m_data);
		m_allocator = nullptr;
		m_data = nullptr;
		m_length = 0;
		m_reserved = 0;
	}


	Bool operator==(DynArray arr)
	{
		return MemoryCompare(m_data, arr.m_data, SIZE_OF(T) * m_length);
	}

	Bool operator!=(DynArray arr)
	{
		return !MemoryCompare(m_data, arr.m_data, SIZE_OF(T) * m_length);
	}

	T& operator[](Usize i)
	{
		ASSERT(i < m_length, "Trying to access element out of bounds");
		return m_data[i];
	}


	NoType Reserve(Usize size)
	{
		if (size > m_reserved)
		{
			T* temp = static_cast<T*>(m_allocator->Alloc(SIZE_OF(T) * size));
			MemoryCopy(temp, m_data, m_length);
			m_allocator->Free(m_data);
			m_data = temp;
		}
	}

	NoType Shrink()
	{
		if (m_reserved > m_length)
		{
			m_allocator->Realloc(m_data, SIZE_OF(T) * m_length);
			m_reserved = m_length;
		}
	}

	NoType Append(T value)
	{
		m_length += 1;
		if (m_reserved < m_length)
		{
			// TODO: Maybe we should just pre allocate
			// a larger size to avoid future allocations
			const Usize slotsToGrow = 0;
			this->Reserve(m_length + slotsToGrow);
		}

		m_data[m_length - 1] = value;
	}

	Usize Length()
	{
		return m_length;
	}

	Usize Reserved()
	{
		return m_reserved;
	}

	IAllocator* m_allocator;
	T* m_data;
	Usize m_length;
	Usize m_reserved;
};

template <typename T>
struct Slice
{
    Slice(IAllocator* allocator, T* data, Usize size)
    {
        m_allocator = allocator;
        m_data = data;
        m_size = size;
    }

	Slice(IAllocator* allocator, Usize size)
	{
		m_allocator = allocator;
		m_data = static_cast<T*>(allocator->Alloc(SIZE_OF(T) * size));
		m_size = size;
	}

	Slice(DynArray<T> arr)
	{
	    // TODO: I'm not sure if we should inherit the allocator
		m_allocator = nullptr;
		m_data = arr.m_data;
		m_size = arr.m_reserved;
	}

	NoType Release()
	{
		if (m_allocator != nullptr)
		{
			m_allocator->Free(m_data);
		}

		m_allocator = nullptr;
		m_data = nullptr;
		m_size = 0;
	}


	Bool operator==(Slice slice)
	{
		return MemoryCompare(m_data, slice.m_data, SIZE_OF(T) * m_size);
	}

	Bool operator!=(Slice slice)
	{
		return !MemoryCompare(m_data, slice.m_data, SIZE_OF(T) * m_size);
	}

	T& operator[](Usize i)
	{
		ASSERT(i < m_size, "Trying to access element out of bounds");
		return m_data[i];
	}


	Slice Clone(IAllocator* allocator = nullptr)
	{
		if (allocator == nullptr)
		{
		    ASSERT(m_allocator != nullptr, "Missing allocator");
			allocator = m_allocator;
		}

		Slice newSlice = Slice(allocator, m_size);
		MemoryCopy(newSlice.m_data, m_data, SIZE_OF(T) * m_size);

		return newSlice;
	}

	Usize Size()
	{
		return m_size;
	}

	IAllocator* m_allocator;
	T* m_data;
	Usize m_size;
};


// =======================
// ======= Threads =======
// =======================

// Implemented per OS
typedef NoType* (*ThreadFunc)(NoType* data);
struct Thread;
struct Mutex;

#if 0
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
};

struct Mutex
{
    Mutex(U32 waitSpin = 32);
    NoType Release();

    NoType Lock();
    Bool TryLock();
    NoType Unlock();
};
#endif


// ==========================
// ======= Filesystem =======
// ==========================

Slice<Byte> ReadEntireFile(String8 path, IAllocator* allocator);
String8 ReadEntireFileAsString(String8 path, IAllocator* allocator);
Bool WriteEntireFile(String8 path, Slice<Byte> buffer);
String8 GetDirFromPath(String8 path, IAllocator* allocator = nullptr);
String8 GetExePath(IAllocator* allocator);
String8 GetUserDir(IAllocator* allocator);
String8 GetConfigDir(IAllocator* allocator);


// ------- OS Includes -------
#if defined(BASE_OS_WIN32)
    #include "platform/base_win32.hpp"
#elif defined(BASE_OS_LINUX)
    #include "platform/base_linux.hpp"
#endif
