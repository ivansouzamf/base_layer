#pragma once

#include <stdint.h>
#include <stddef.h>

// unsinged ints
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
// signed ints
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
// floats
typedef float f32;
typedef double f64;
typedef long double f80;
// others
typedef size_t usize;
typedef char c8;
typedef u8 byte;
typedef bool b8;

// custom allocators
struct Allocator {
	void* (*alloc)(usize size);
	void* (*realloc)(void* ptr, usize size);
	void (*free)(void* ptr);
};

// strings
#define DEFAULT_STRING_SIZE 20
struct String8 {
	c8* data;
	u32 len;
	usize reserved;

	// NOTE: these are all overloads to make string initialization easier.
	// (almost) no crazy c++ stuff
	String8();
	String8(const c8* in_string); // NOTE: the input string has to be null terminated
	String8 operator=(const c8* in_string); // NOTE: the input string has to be null terminated
};

// utils
struct Buffer {
	byte* data;
	usize size;
};

// strings
void set_string_allocator(Allocator allocator);
String8 create_string(u32 reserve = DEFAULT_STRING_SIZE);
String8 create_string_from(const c8* in_string); // NOTE: the input string has to be null terminated
void assign_string(String8* string, const c8* in_string); // NOTE: the input string has to be null terminated
void destroy_string(String8* string);

// utils
Buffer read_entire_file(String8 path, Allocator allocator);
String8 read_entire_file_as_string(String8 path);
