#include "base.h"

// TODO: move all code that require these includes to it's
// own platform implementation
#include <stdio.h>
#include <stdarg.h>

// ====================
// ======= Math =======
// ====================

template <typename T>
T abs(T num) {
    return (num < 0) ? -num : num;
}


// =====================
// ======= Utils =======
// =====================

void copy_memory(void* dest, const void* source, Usize size) {
    Byte* dst = (Byte*) dest;
    Byte* src = (Byte*) source;
    for (Usize i = 0; i < size; i += 4) {
        dst[i + 0] = src[i + 0];
        dst[i + 1] = src[i + 1];
        dst[i + 2] = src[i + 2];
        dst[i + 3] = src[i + 3];
    }
}

void zero_memory(void* dest, Usize size) {
    Byte* dst = (Byte*) dest;
    for (Usize i = 0; i < size; i += 4) {
        dst[i + 0] = 0;
        dst[i + 1] = 0;
        dst[i + 2] = 0;
        dst[i + 3] = 0;
    }
}

Usize cstring_len(const C8* cstring) {
    Usize i = 0;
    while (cstring[i] != '\0') {
        i += 1;
    }

    return i;
}


// =================================
// ======= Custom Allocators =======
// =================================

inline Bool allocator_is_null(Allocator allocator) {
    return allocator.alloc == nullptr && allocator.realloc == nullptr && allocator.free == nullptr;
}

inline void* allocator_alloc(Allocator allocator, Usize size) {
    return allocator.alloc(allocator.data, size);
}

inline void* allocator_realloc(Allocator allocator, void* ptr, Usize size) {
    return allocator.realloc(allocator.data, ptr, size);
}

inline void allocator_free(Allocator allocator, void* ptr) {
    return allocator.free(allocator.data, ptr);
}

Arena_Allocator arena_init(void* buffer, Usize size) {
    Arena_Allocator arena = {
        .current = buffer,
        .buffer = buffer,
        .buffer_size = size,
    };

    return arena;
}

Allocator arena_get_allocator(Arena_Allocator* arena) {
    Allocator allocator = {
        .data = arena,
        .alloc = arena_alloc,
        .realloc = arena_realloc,
        .free = arena_free,
    };

    return allocator;
}

void arena_free_all(Arena_Allocator* arena) {
    arena->current = arena->buffer;
}

void* arena_alloc(void* data, Usize size) {
    Arena_Allocator* arena = (Arena_Allocator*) data;

    Bool cond = (Byte*) arena->current + size <= (Byte*) arena->buffer + arena->buffer_size;
    ASSERT(cond, "Cannot allocate more than the size of the arena");

    void* ptr = arena->current;
    arena->current = (Byte*) arena->current + size;

    return ptr;
}

void* arena_realloc(void* data, void* ptr, Usize size) {
    if (size <= 0) {
        arena_free(data, ptr);
        return nullptr;
    }

    void* new_ptr = arena_alloc(data, size);
    // TODO: copy memory when realocating

    return new_ptr;
}

void arena_free(void* data, void* ptr) {
    // can't free individual components of an arena
    (void) data;
    (void) ptr;
    return;
}


// =======================
// ======= Strings =======
// =======================

String8 init_string(Allocator allocator, C8* data, Usize lenght) {
	String8 string = {
	    .allocator = allocator,
		.data = data,
		.lenght = lenght,
	};

	return string;
}

static inline Usize get_string_len_int(U64 num) {
    Usize lenght = 0;
    while (num > 0) {
        num /= 10;
        lenght += 1;
    }

    return lenght;
}

String8 create_string_from_U64(U64 num, Allocator allocator) {
    if (num == 0) {
        return create_string_from_cstring("0", allocator);
    }

    Usize lenght = get_string_len_int(num);
    String8 string = alloc_string(lenght, allocator);

    for (Usize i = 0; i != lenght; i += 1) {
        C8 char_num = '0' + (C8) (num % 10);
        string.data[i] = char_num;
        num /= 10;
    }

    reverse_string(&string);

    return string;
}

String8 create_string_from_S64(S64 num, Allocator allocator) {
    U64 abs_num = (U64) abs(num);
    Usize lenght = get_string_len_int(abs_num);
    String8 string = {};

    if (num == 0) {
        return create_string_from_cstring("0", allocator);
    } else if (num < 0) {
        lenght += 1;
    }
    string = alloc_string(lenght, allocator);

    for (Usize i = 0; i != string.lenght; i += 1) {
        if (i == string.lenght - 1) {
            string.data[i] = '-';
            break;
        }

        C8 char_num = '0' + (C8) (abs_num % 10);
        string.data[i] = char_num;

        abs_num /= 10;
    }

    reverse_string(&string);

    return string;
}

String8 create_string_from_F64(U64 num) {
    // TODO: implement this
    (void) num;
    return {};
}

String8 create_string_from_cstring(const C8* cstring, Allocator allocator) {
	Usize lenght = cstring_len(cstring);
	String8 string = alloc_string(lenght, allocator);
	string = assign_string(string, cstring);

	return string;
}

String8 clone_string(String8 string, Allocator allocator) {
    String8 new_string = alloc_string(string.lenght, allocator);
    copy_memory(new_string.data, string.data, string.lenght);

    return new_string;
}

C8* clone_string_to_cstring(String8 string, Allocator allocator) {
   	C8* cstring = (C8*) allocator_alloc(allocator, string.lenght + 1);
	copy_memory(cstring, string.data, string.lenght);
	cstring[string.lenght] = '\0';

	return cstring;
}

// NOTE: DO NOT USE THIS FOR STRING LITERALS!
// use 'lit_string()' instead
String8 assign_string(String8 string, const C8* in_string) {
    ASSERT(!allocator_is_null(string.allocator), "Trying to modify non dynamic string");

	Usize lenght = cstring_len(in_string);
	if (lenght > string.lenght) {
		string.data = (C8*) allocator_realloc(string.allocator, string.data, lenght);
	}

	string.lenght = lenght;
	copy_memory(string.data, in_string, string.lenght);

	return string;
}

void append_string(String8* string, String8 in_string) {
	ASSERT(!allocator_is_null(string->allocator), "Trying to modify non dynamic string");

	Usize new_size = in_string.lenght + string->lenght;
	string->data = (C8*) allocator_realloc(string->allocator, string->data, new_size);

	copy_memory(string->data + string->lenght, in_string.data, in_string.lenght);
	string->lenght += in_string.lenght;
}

void reverse_string(String8* string) {
    Usize start = 0;
    Usize end = string->lenght - 1;

    while (start < end) {
        C8 temp = string->data[start];
        string->data[start] = string->data[end];
        string->data[end] = temp;
        start += 1;
        end -= 1;
    }
}

String8 alloc_string(Usize lenght, Allocator allocator) {
	String8 string = {
	    .allocator = allocator,
	    .data = (C8*) allocator_alloc(allocator, lenght),
	    .lenght = lenght,
	};

	zero_memory(string.data, string.lenght);

	return string;
}

void free_string(String8* string) {
	ASSERT(!allocator_is_null(string->allocator), "Trying to free non dynamic string");

	if (string->data != nullptr) {
		allocator_free(string->allocator, string->data);
		string->data = nullptr;
		string->lenght = 0;
	}
}


// ===============================
// ======= Data Structures =======
// ===============================

template <typename T>
Slice<T> create_slice(Usize size, Allocator allocator) {
	Slice<T> slice = {
		.allocator = allocator,
		.data = (T*) allocator_alloc(allocator, sizeof(T) * size),
		.size = size,
	};

	return slice;
}

template <typename T>
void destroy_slice(Slice<T>* slice) {
	allocator_free(slice->allocator, slice->data);
	slice->data = nullptr;
	slice->size = 0;
}

template <typename T>
T& Slice<T>::operator[](Usize index) {
	ASSERT(index < size, "Trying to access element out of bounds");
	return data[index];
}

template <typename T>
Dynamic_Array<T> create_dynamic_array(Usize size, Allocator allocator) {
	Dynamic_Array<T> array = {
		.allocator = allocator,
		.data = (T*) allocator_alloc(allocator, sizeof(T) * size),
		.lenght = 0,
		.reserved = size,
	};

	return array;
}

template <typename T>
void append_dynamic_array(Dynamic_Array<T>* array, T element) {
	if (array->lenght >= array->reserved) {
		// TODO: maybe we should allocate more than we need when resizing to avoid future
		// allocations
		Usize slots_to_grow = 1;
		Usize new_size = (sizeof(T) * slots_to_grow) + (array->reserved * sizeof(T));
		array->data = (T*) allocator_realloc(array->allocator, array->data, new_size);
		array->reserved += slots_to_grow;
	}

	array->data[array->lenght] = element;
	array->lenght += 1;
}

template <typename T>
void destroy_dynamic_array(Dynamic_Array<T>* array) {
	allocator_free(array->allocator, array->data);
	array->data = nullptr;
	array->lenght = 0;
	array->reserved = 0;
}

template <typename T>
T& Dynamic_Array<T>::operator[](Usize index) {
	ASSERT(index < lenght, "Trying to access element out of bounds");
	return data[index];
}


// =====================
// ======= Utils =======
// =====================

void print_fmt(String8 fmt, ...) {
    C8* fmt_cstring = clone_string_to_cstring(fmt, get_temp_allocator());

	va_list args;
	va_start(args, fmt);
	vprintf(fmt_cstring, args);
	va_end(args);

	arena_free_all((Arena_Allocator*) get_temp_allocator().data);
}

String8 get_dir_from_path(String8 path, Allocator allocator) {
	U32 last_slash = 0;
    for (U32 i = 0; i < path.lenght; i += 1) {
        if (path.data[i] == BASE_PATH_SEPARATOR) {
            last_slash = i;
        }
    }

    // we only clone the string and cap the length
    String8 dir = clone_string(path, allocator);
    dir.lenght = last_slash;

    return dir;
}

String8 read_entire_file_as_string(String8 path, Allocator allocator) {
	Slice<Byte> file_buff = read_entire_file(path, allocator);
	String8 file_string = {
		.data = (C8*) file_buff.data,
		.lenght = file_buff.size,
	};

	return file_string;
}


// =================================
// ======= Global Allocators =======
// =================================

static Allocator g_temp_allocator = {
    .data = nullptr,
    .alloc = nullptr,
    .realloc = nullptr,
    .free = nullptr,
};

void init_temp_allocator(Arena_Allocator* arena) {
    g_temp_allocator = arena_get_allocator(arena);
}

Allocator get_temp_allocator() {
    return g_temp_allocator;
}


// OS specific code
#if defined(BASE_OS_LINUX)
	#include "base_linux.cpp"
#elif defined(BASE_OS_WINDOWS)
	#include "base_windows.cpp"
#endif