#include "base.cpp"

// only here for malloc
#include <stdlib.h>

void* malloc_alloc(void* data, Usize size);
void* malloc_realloc(void* data, void* ptr, Usize size);
void malloc_free(void* data, void* ptr);

S32 entry_point(S32 argc, C8* argv[]) {
    Allocator gpa = {
        .data = nullptr,
        .alloc = malloc_alloc,
        .realloc = malloc_realloc,
        .free = malloc_free,
    };
    Usize size = KILOBYTE(2);
    void* buffer = allocator_alloc(gpa, size);
    Arena_Allocator temp_arena = arena_init(buffer, size);
    init_temp_allocator(&temp_arena);
    
    (void) argc;
    (void) argv;

    String8 newline = lit_string("\n");

    String8 hello = lit_string("Hello, World!\n");
    String8 exe_path = get_exe_path(gpa);
    String8 exe_dir = get_dir_from_path(exe_path, gpa);
    String8 user_dir = get_user_dir(gpa);
    String8 config_dir = get_config_dir(gpa);

    print_fmt(hello);
    print_fmt(exe_path); print_fmt(newline);
    print_fmt(exe_dir); print_fmt(newline);
    print_fmt(user_dir); print_fmt(newline);
    print_fmt(config_dir); print_fmt(newline);

    String8 u64_string = create_string_from_U64(123456, gpa);
    String8 S64_string = create_string_from_S64(-654321, gpa);

    String8 myfile = read_entire_file_as_string(lit_string("./build.sh"), gpa);

    Slice<U32> myslice = create_slice<U32>(20, gpa);
    for (Usize i = 0; i < myslice.size; i += 1) {
        myslice[i] = (U32) i;
        U32 myint = myslice[i];
        (void) myint;
    }

    Dynamic_Array<U32> mydynarray = create_dynamic_array<U32>(20, gpa);
    for (U32 i = 0; i < 10; i += 1) {
        append_dynamic_array(&mydynarray, i);
    }

    for (Usize i = 0; i < mydynarray.lenght; i += 1) {
        mydynarray[i] = (U32) i;
        U32 myint = mydynarray[i];
        (void) myint;
    }

    return 0;
}

inline void* malloc_alloc(void* data, Usize size) {
    (void) data;
    return malloc(size);
}
inline void* malloc_realloc(void* data, void* ptr, Usize size) {
    (void) data;
    return realloc(ptr, size);
}
inline void malloc_free(void* data, void* ptr) {
    (void) data;
    return free(ptr);
}