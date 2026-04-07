#include "base.hpp"
#include <stdio.h>

S32 EntryPoint(S32 argc, C8* argv[])
{
    Bl::HeapAllocator heapAlloc;

    Bl::String8 hello = "Hello, World!";
    printf("%s\n", hello.CString());

    Bl::String8 exePath   = Bl::GetExePath(&heapAlloc);
    Bl::String8 userDir   = Bl::GetUserDir(&heapAlloc);
    Bl::String8 configDir = Bl::GetConfigDir(&heapAlloc);
    printf(
        "Exe path   = %s\n"
        "User dir   = %s\n"
        "Config dir = %s\n",
        exePath.CString(), userDir.CString(), configDir.CString()
    );

    for (S32 i = 0; i < argc; i += 1)
        printf("Argv[%i] = %s\n", i, argv[i]);

    Bl::String8 fileContent = Bl::ReadEntireFileAsString("./src/base.cpp", &heapAlloc);
    printf(
        "### FILE DUMP ###\n"
        "%s\n"
        "### END DUMP ###\n",
        fileContent.CString()
    );

    Bl::Vec4F myVec1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    Bl::Vec4F myVec2 = 5.0f;
    Bl::Vec4F myVec3 = myVec1 * myVec2;

    return 0;
}
