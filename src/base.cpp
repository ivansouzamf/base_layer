#include "base.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static Allocator string_allocator = {
	.alloc = nullptr,
	.realloc = nullptr,
	.free = nullptr,
};

static void check_string_allocator() {
	if (string_allocator.alloc == nullptr || string_allocator.realloc == nullptr || string_allocator.free == nullptr) {
		fprintf(stderr, "string_allocator not setted\n");
		abort();
	}
}

void set_string_allocator(Allocator allocator) {
	string_allocator = allocator;
	check_string_allocator();
}

String8 create_string(u32 reserve) {
	check_string_allocator();

	String8 string;
    string.data = (c8*) string_allocator.alloc(reserve);
    string.len = 0;
    string.reserved = reserve;

	memset((void*) string.data, 0, string.len);

	return string;
}

String8 create_string_from(const c8* in_string) {
	u32 len = strlen(in_string);
	String8 string = create_string(len);
	strcpy(string.data, in_string);
	string.len = len;

	return string;
}

void assign_string(String8* string, const c8* in_string) {
	check_string_allocator();

	u32 len = strlen(in_string);
	if (len > string->reserved) {
		string_allocator.free((void*) string->data);
		string->data = (c8*) string_allocator.alloc(len);
		string->reserved = len;
	}

	memset((void*) string->data, 0, string->len);
	strcpy(string->data, in_string);
}

void destroy_string(String8* string) {
	check_string_allocator();

	if (string->data != nullptr) {
		string_allocator.free((void*) string->data);
		string->data = nullptr;
		string->len = 0;
	}
}

// NOTE: the only stupid reason that is here is because
// c++ is a bitch
String8::String8() {
}

String8::String8(const c8* in_string) {
    *this = create_string_from(in_string);
}

String8 String8::operator=(const c8* in_string) {
	// TODO: maybe we should trigger an error if the string
	// is not allocated
	if (reserved != 0) {
		assign_string(this, in_string);
	}

	return *this;
}

Buffer read_entire_file(String8 path, Allocator allocator) {
	Buffer file_read = {
		.data = nullptr,
		.size = 0
	};
	usize read = 0;

	FILE* file = fopen(path.data, "rb");
	if (file == nullptr) {
		goto clean_and_exit;
	}

	fseek(file, 0, SEEK_END);
	file_read.size = ftell(file);
	fseek(file, 0, SEEK_SET);

	file_read.data = (byte*) allocator.alloc(file_read.size);
	read = fread(file_read.data, file_read.size, 1, file);
	if ((read * file_read.size) != file_read.size) {
		allocator.free(file_read.data);
		file_read.data = nullptr;
		file_read.size = 0;
	}

	fclose(file);
	clean_and_exit:
	destroy_string(&path);
	return file_read;
}

String8 read_entire_file_as_string(String8 path) {
	check_string_allocator();

	Buffer file_read = read_entire_file(path, string_allocator);
	String8 file_read_string;
	file_read_string.data = (c8*) file_read.data;
	file_read_string.len = (u32) file_read.size;
	file_read_string.reserved = file_read.size;

	return file_read_string;
}
