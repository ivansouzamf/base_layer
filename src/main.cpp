#include "base.cpp"

// only here for malloc
#include <stdlib.h>

S32 entry_point(S32 argc, C8* argv[]) {
    Allocator gpa = {
        malloc,
        realloc,
        free
    };
    set_string_allocator(gpa);

    String8 newline = lit_string("\n");

    String8 hello = lit_string("Hello, World!\n");
    String8 exe_path = get_exe_path();
    String8 exe_dir = get_dir_from_path(exe_path);
    String8 user_dir = get_user_dir();
    String8 config_dir = get_config_dir();

    print_fmt(hello);
    print_fmt(exe_path); print_fmt(newline);
    print_fmt(exe_dir); print_fmt(newline);
    print_fmt(user_dir); print_fmt(newline);
    print_fmt(config_dir); print_fmt(newline);

    String8 myfile = read_entire_file_as_string(lit_string("./build_musl.sh"));

    Slice<U32> myslice = create_slice<U32>(20, gpa);
    for (Usize i = 0; i < myslice.len; i += 1) {
        myslice[i] = (U32) i;
        U32 myint = myslice[i];
    }

    Dynamic_Array<U32> mydynarray = create_dynamic_array<U32>(20, gpa);
    for (U32 i = 0; i < 10; i += 1) {
        append_dynamic_array(&mydynarray, i);
    }

    for (Usize i = 0; i < mydynarray.len; i += 1) {
        mydynarray[i] = (U32) i;
        U32 myint = mydynarray[i];
    }

    return 0;
}