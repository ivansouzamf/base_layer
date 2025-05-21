@echo off
setlocal enabledelayedexpansion

set program_name=base_layer_test.exe
set build_dir=.\build

set sources=../src/main.cpp
set common_opts=/nologo /FC /W4 /Wall /wd4711 /wd5045 /GR- /EHa- /std:c++14 /arch:AVX2 /Fe:%program_name%
set debug_opts=/Od /Zi /DWIN_CONSOLE_MODE /DDEBUG
set release_opts=/O2 /DWIN_WINDOWS_MODE /DRELEASE
set libraries=kernel32.lib user32.lib ucrt.lib vcruntime.lib msvcrt.lib
set link_opts=/link %libraries% /NODEFAULTLIB /INCREMENTAL:NO

if "%1" == "debug" (
    echo --- Building in debug mode ---
    set mode_opts=%debug_opts%
    set link_opts=%link_opts% /SUBSYSTEM:console
) else if "%1" == "release" (
    echo --- Building in release mode ---
    set mode_opts=%release_opts%
    set link_opts=%link_opts% /SUBSYSTEM:windows
) else if "%1" == "run" (
    echo --- Running %program_name% ---
    %build_dir%\%program_name%
    goto :end
) else (
    echo Invalid command. Use 'debug', 'release' or 'run'
    goto :end
)

if not exist "%build_dir%" (
    mkdir %build_dir%
)

pushd %build_dir%
cl.exe %common_opts% %mode_opts% %sources% %link_opts%
popd

:end