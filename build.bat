@echo off
setlocal enabledelayedexpansion

set program_name=base_layer_test.exe
set build_dir=.\build

set sources=../src/*.cpp
set debug_opts=/Od /Zi /DDEBUG
set release_opts=/O2 /GL /DRELEASE
set libraries=libvcruntime.lib libcmt.lib libucrt.lib kernel32.lib user32.lib
set comp_opts=/nologo /FC /W4 /std:c++20 /Zc:threadSafeInit- /GS- /Gs9999999 /GR- /EHa- /arch:AVX2 /Fe:%program_name%
set link_opts=/link %libraries% /NODEFAULTLIB /INCREMENTAL:NO /STACK:0x100000,0x100000

if "%1" == "debug" (
	echo --- Building in debug mode ---
	set comp_opts=%comp_opts% %debug_opts% /DWIN32_CONSOLE_MODE
	set link_opts=%link_opts% /SUBSYSTEM:console
) else if "%1" == "release" (
	echo --- Building in release mode ---
	set comp_opts=%comp_opts% %release_opts% /DWIN32_WINDOWS_MODE
	set link_opts=%link_opts% /SUBSYSTEM:windows
) else if "%1" == "run" (
	echo --- Running %program_name% ---
	%build_dir%\%program_name%
	exit /b
) else (
	echo Invalid command. Use 'debug', 'release' or 'run'
	exit /b 1
)

if not exist "%build_dir%" (
	mkdir %build_dir%
)

pushd %build_dir%
	echo --- Building %program_name% ---
	echo %comp_opts% %sources% %link_opts%
	cl.exe %comp_opts% %sources% %link_opts%
popd

exit /b