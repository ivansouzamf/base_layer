#include "base.hpp"
#include <stdio.h>

S32 EntryPoint(S32 argc, C8* argv[])
{
    HeapAllocator heapAlloc;

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

    for (S32 i = 0; i < argc; i += 1)
        printf("Argv[%i] = %s\n", i, argv[i]);

    String8 fileContent = ReadEntireFileAsString("./src/base.cpp", &heapAlloc);
    printf(
        "### FILE DUMP ###\n"
        "%s\n"
        "### END DUMP ###\n",
        fileContent.CString()
    );

    return 0;
}
