#pragma once

#include <stdint.h>
#include <stddef.h>

// ==================================
// ======= Macros && Typedefs =======
// ==================================

#if !defined(__x86_64__) && !defined(_WIN64)
	#error "32 bit is not supported"
#endif

#if defined(__linux__)
	#define BASE_OS_LINUX
#elif defined(_WIN32)
	#define BASE_OS_WINDOWS
#else
	#error "OS not supported"
#endif

#if defined(DEBUG)
	#if defined(BASE_OS_LINUX)
		#define ASSERT_CALL asm("int $3")
	#elif defined(BASE_OS_WINDOWS)
		#define ASSERT_CALL __debugbreak()
	#endif
	// TODO: maybe log or print the msg to stderr (or debug output on windows)
	#define ASSERT(cond, msg) do { if (!(cond)) ASSERT_CALL; } while (0)
#else
	// TODO: implement ASSERT for release builds
	#define ASSERT(cond, msg) do { if (!(cond)){} } while (0)
#endif

#if defined(BASE_OS_LINUX)
	#define BASE_PATH_SEPARATOR '/'
#elif defined(BASE_OS_WINDOWS)
	#define BASE_PATH_SEPARATOR '\\'
#endif

#define KILOBYTE(v) v * 1024
#define MEGABYTE(v) KILOBYTE(v) * 1024
#define GIGABYTE(v) MEGABYTE(v) * 1024

// unsinged ints
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
// signed ints
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
// floats
typedef float F32;
typedef double F64;
typedef long double F80;
// others
typedef size_t Usize;
typedef bool Bool;
typedef char C8;
typedef U8 Byte;


// ====================
// ======= Math =======
// ====================

template <typename T>
T abs(T num);


// =====================
// ======= Utils =======
// =====================

void copy_memory(void* dest, const void* source, Usize size);
void zero_memory(void* dest, Usize size);
Usize cstring_len(const C8* cstring);


// =================================
// ======= Custom Allocators =======
// =================================

typedef void* (*Allocator_Alloc)(void* data, Usize size);
typedef void* (*Allocator_Realloc)(void* data, void* ptr, Usize size);
typedef void (*Allocator_Free)(void* data, void* ptr);

struct Allocator {
	void* data;
	Allocator_Alloc alloc;
	Allocator_Realloc realloc;
	Allocator_Free free;
};

Bool allocator_is_null(Allocator allocator);
void* allocator_alloc(Allocator allocator, Usize size);
void* allocator_realloc(Allocator allocator, void* ptr, Usize size);
void allocator_free(Allocator allocator, void* ptr);

struct Arena_Allocator {
    void* current;
    void* buffer;
    Usize buffer_size;
};

Arena_Allocator arena_init(void* buffer, Usize size);
Allocator arena_get_allocator(Arena_Allocator* arena);
void arena_free_all(Arena_Allocator* arena);
void* arena_alloc(void* data, Usize size);
void* arena_realloc(void* data, void* ptr, Usize size);
void arena_free(void* data, void* ptr);


// =======================
// ======= Strings =======
// =======================

struct String8 {
    Allocator allocator;
	C8* data;
	Usize lenght;
};

#define lit_string(cstring) init_string({}, (C8*) cstring, sizeof(cstring) - 1)
String8 init_string(Allocator allocator, C8* data, Usize len);
String8 create_string_from_U64(U64 num, Allocator allocator);
String8 create_string_from_S64(S64 num, Allocator allocator);
String8 create_string_from_F64(U64 num, Allocator allocator);
String8 create_string_from_cstring(const C8* cstring, Allocator allocator);
String8 clone_string(String8 string, Allocator allocator);
C8* clone_string_to_cstring(String8 string, Allocator allocator);
String8 assign_string(String8 string, const C8* in_string);
void append_string(String8* string, String8 in_string);
void reverse_string(String8* string);
String8 alloc_string(Usize lenght, Allocator allocator);
void free_string(String8* string);


// ===============================
// ======= Data Structures =======
// ===============================

template <typename T>
struct Slice {
	Allocator allocator;
	T* data;
	Usize size;

	T& operator[](Usize index);
};

template <typename T>
Slice<T> create_slice(Usize size, Allocator allocator);
template <typename T>
void destroy_slice(Slice<T>* slice);

template <typename T>
struct Dynamic_Array {
	Allocator allocator;
	T* data;
	Usize lenght;
	Usize reserved;

	T& operator[](Usize index);
};

template <typename T>
Dynamic_Array<T> create_dynamic_array(Usize size, Allocator allocator);
template <typename T>
void append_dynamic_array(Dynamic_Array<T>* array, T element);
template <typename T>
void destroy_dynamic_array(Dynamic_Array<T>* array);


// =====================
// ======= Utils =======
// =====================

void print_fmt(String8 fmt, ...);
String8 get_dir_from_path(String8 path, Allocator allocator);
String8 get_exe_path(Allocator allocator);
String8 get_user_dir(Allocator allocator);
String8 get_config_dir(Allocator allocator);
Slice<Byte> read_entire_file(String8 path, Allocator allocator);
String8 read_entire_file_as_string(String8 path, Allocator allocator);


// =================================
// ======= Global Allocators =======
// =================================

void init_temp_allocator(void* buffer);
Allocator get_temp_allocator();

// entry point definition
S32 entry_point(S32 argc, C8* argv[]);