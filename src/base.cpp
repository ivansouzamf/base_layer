#include "base.h"

// TODO: move all code that require these includes to it's
// own platform implementation
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static Allocator string_allocator = {
	.alloc = nullptr,
	.realloc = nullptr,
	.free = nullptr,
};

static inline void check_string_allocator() {
	Bool cond = string_allocator.alloc != nullptr && string_allocator.realloc != nullptr && string_allocator.free != nullptr;
	ASSERT(cond, "string_allocator not setted\n");
}

void set_string_allocator(Allocator allocator) {
	string_allocator = allocator;
	check_string_allocator();
}

Allocator* get_string_allocator() {
	check_string_allocator();
	return &string_allocator;
}

String8 create_string(Usize reserve) {
	check_string_allocator();

	String8 string = {
	    .data = (C8*) string_allocator.alloc(reserve),
	    .len = 0,
	    .reserved = reserve,
	};

	memset(string.data, 0, string.len);

	return string;
}

String8 create_string_from(const C8* in_string) {
	Usize len = strlen(in_string);
	String8 string = create_string(len);
	string = assign_string(string, in_string);

	return string;
}

String8 lit_string(const C8* in_string) {
	Usize len = strlen(in_string);
	String8 string = {
		.data = (C8*) in_string,
		.len = len,
		.reserved = 0,
	};
	
	return string;
}

String8 clone_string(String8 string) {
    String8 new_string = create_string(string.len);
    memcpy(new_string.data, string.data, string.len);

    return new_string;
}

// NOTE: DO NOT USE THIS FOR STRING LITERALS!
// use 'lit_string()' instead
String8 assign_string(String8 string, const C8* in_string) {
	check_string_allocator();
	ASSERT(string.reserved != 0, "Cannot modify a string literal");

	String8 result = string;

	Usize len = strlen(in_string);
	if (len > result.reserved) {
		string_allocator.free(result.data);
		result.data = (C8*) string_allocator.alloc(len);
		result.reserved = len;
	}

	result.len = len;

	memset(result.data, 0, result.len);
	memcpy(result.data, in_string, result.len);

	return result;
}

void append_string(String8* string, String8 in_string) {
	check_string_allocator();
	ASSERT(string->reserved != 0, "Cannot modify a string literal");
	
	if (in_string.len > string->reserved - string->len) {
		Usize new_size = in_string.len + string->len;
		string->data = (C8*) string_allocator.realloc(string->data, new_size);
		string->reserved = new_size;
	}

	memcpy(string->data + string->len, in_string.data, in_string.len);
	string->len += in_string.len;
}

void destroy_string(String8* string) {
	check_string_allocator();

	if (string->data != nullptr && string->reserved > 0) {
		string_allocator.free(string->data);
		string->data = nullptr;
		string->len = 0;
		string->reserved = 0;
	}
}

template <typename T>
Slice<T> create_slice(Usize len, Allocator allocator) {
	Slice<T> slice = {
		.allocator = allocator,
		.data = (T*) allocator.alloc(sizeof(T) * len),
		.len = len,
	};

	return slice;
}

template <typename T>
void destroy_slice(Slice<T>* slice) {
	slice->allocator.free(slice->data);
	slice->data = nullptr;
	slice->len = 0;
}

template <typename T>
T& Slice<T>::operator[](Usize index) {
	ASSERT(index < len, "Trying to access element out of bounds\n");
	return data[index];
}

template <typename T>
Dynamic_Array<T> create_dynamic_array(Usize size, Allocator allocator) {
	Dynamic_Array<T> array = {
		.allocator = allocator,
		.data = (T*) allocator.alloc(sizeof(T) * size),
		.len = 0,
		.reserved = size,
	};

	return array;
}

template <typename T>
void append_dynamic_array(Dynamic_Array<T>* array, T element) {
	if (array->len >= array->reserved) {
		// TODO: maybe we should allocate more than we need when resizing to avoid future
		// allocations
		Usize slots_to_grow = 1;
		Usize new_size = (sizeof(T) * slots_to_grow) + (array->reserved * sizeof(T));
		array->data = (T*) array->allocator.realloc(array->data, new_size);
		array->reserved += slots_to_grow;
	}

	array->data[array->len] = element;
	array->len += 1;
}

template <typename T>
void destroy_dynamic_array(Dynamic_Array<T>* array) {
	array->allocator.free(array->data);
	array->data = nullptr;
	array->len = 0;
	array->reserved = 0;
}

template <typename T>
T& Dynamic_Array<T>::operator[](Usize index) {
	ASSERT(index < len, "Trying to access element out of bounds\n");
	return data[index];
}

void print_fmt(String8 fmt, ...) {
	String8 temp = create_string(fmt.len + 1);
	memcpy(temp.data, fmt.data, fmt.len);
	temp.data[fmt.len] = '\0';

	va_list args;
	va_start(args, fmt);
	vprintf(temp.data, args);
	va_end(args);

	destroy_string(&temp);
}

String8 get_dir_from_path(String8 path) {
	U32 last_slash = 0;
    for (U32 i = 0; i < path.len; i += 1) {
        if (path.data[i] == BASE_PATH_SEPARATOR) {
            last_slash = i;
        }
    }

    // we only clone the string and cap the length
    String8 dir = clone_string(path);
    dir.len = last_slash;

    return dir;
}

String8 read_entire_file_as_string(String8 path) {
	Slice<Byte> file_buff = read_entire_file(path, *get_string_allocator());
	String8 file_string = {
		.data = (C8*) file_buff.data,
		.len = file_buff.len,
		.reserved = file_buff.len,
	};

	return file_string;
}

// OS specific code
#if defined(BASE_OS_LINUX)
	#include "base_linux.cpp"
#elif defined(BASE_OS_WINDOWS)
	#include "base_windows.cpp"
#endif