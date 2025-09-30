#include "base.hpp"
#include <stdio.h>

S32 EntryPoint(S32 argc, C8* argv[])
{
    HeapAllocator heapAlloc = HeapAllocator();

    String8 hello = "Hello, World!";
    printf("%s\n", hello.CString());

    String8 exePath   = GetExePath(&heapAlloc);
    String8 userDir   = GetUserDir(&heapAlloc);
    String8 configDir = GetConfigDir(&heapAlloc);
    printf(
        "Exe path   = %s\n"
        "User dir   = %s\n"
        "Config dir = %s\n",
        exePath.CString(), userDir.CString(), configDir.CString()
    );

    return 0;
}
