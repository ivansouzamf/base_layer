#include "base.hpp"
#include <stdio.h>

S32 EntryPoint(S32 argc, C8* argv[])
{
    String8 hello = "Hello, World!";
    printf("%s\n", hello.CString());
    return 0;
}
