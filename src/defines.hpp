#pragma once

#include <stdint.h>
#include <string.h>

// floats
typedef float F32;
typedef double F64;
// unsinged ints
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t Usize;
// signed ints
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
// booleans
typedef U8 B8;
typedef U16 B16;
typedef U32 B32;
typedef bool Bool;
// chars
typedef char C8;
typedef wchar_t C16;
// others
typedef U8 Byte;
typedef void NoType;

#if defined(_WIN32)
    #define BASE_OS_WIN32
    #define BASE_PATH_SEPARATOR '\\'
#elif defined(__linux__)
    #define BASE_OS_LINUX
    #define BASE_PATH_SEPARATOR '/'
#else
	#error "OS not supported"
#endif

#if defined(_MSC_VER)
    #define BASE_CC_MSVC
    #define BASE_DEBUGTRAP() __debugbreak()
#elif defined(__clang__)
    #define BASE_CC_CLANG
    #define BASE_DEBUGTRAP() __builtin_debugtrap()
#elif defined(__GNUC__)
    #define BASE_CC_GCC
    #define BASE_DEBUGTRAP() asm("int $3")
#else
    #error "Compiler not supported"
#endif

#if defined(DEBUG)
	#define _ASSERT_ALWAYS(cond, msg) do { if (!(cond)) BASE_DEBUGTRAP(); } while (0)
	#define _ASSERT(cond, msg) _ASSERT_ALWAYS(cond, msg)
#else
	void _AssertRel(const C8* msg, const C8* file, const U32 line);
	#define _ASSERT_ALWAYS(cond, msg) do { if (!(cond)) _AssertRel(msg, __FILE__, __LINE__); } while (0)
	#define _ASSERT(cond, msg)
#endif

#define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#define ASSERT_ALWAYS(cond, msg) _ASSERT_ALWAYS(cond, msg)
#define ASSERT(cond, msg) _ASSERT(cond, msg)

#define TYPE_OF(type) decltype(type)
#define SIZE_OF(type) sizeof(type)
#define OFFSET_OF(type, member) ((Usize) (&(((type*)(0))->member)))
#define ALIGN_OF(type) alignof(type)
#define ARRAY_LEN(arr) (SIZE_OF(arr) / SIZE_OF((arr)[0]))

#define KILOBYTE(v) v * 1024
#define MEGABYTE(v) KILOBYTE(v) * 1024
#define GIGABYTE(v) MEGABYTE(v) * 1024

#define MemoryCopy(dst, src, size) memcpy(dst, src, size)
#define MemorySet(dst, byte, size) memset(dst, byte, size)
#define MemoryZero(dst, size) SetMemory(dst, 0, size)
#define MemoryCompare(a, b, size) (memcmp(a, b, size) == 0)
#define CStringLen(str) strlen(str)

#define ArrayCopy(dst, src) MemoryCopy(dst, src, SIZE_OF(dst))
#define ArrayZero(dst) MemoryZero(dst, SIZE_OF(dst))

#define StructCopy(dst, src) MemoryCopy(&dst, &src, SIZE_OF(dst))
#define StructZero(dst) MemoryZero(&dst, SIZE_OF(dst))

#define Abs(num) (((num) < 0) ? -(num) : (num))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(num, min, max) (((num) < (min)) ? (min) : ((num) > (max)) ? (max) : (num))
#define IsPow2(num) ((num) != 0 && ((num) & ((num) - 1)) == 0)
#define AlignPow2(align, num) (((align) + (num) - 1) & (~(align) - 1))
