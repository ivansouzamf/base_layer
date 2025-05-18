#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DWORD_MAX 0xffffffffUL

// TODO: convert arguments to unix format
#if defined(WIN_CONSOLE_MODE)
int wmain(int argc, wchar_t *argv[]) {
	(void) argc;
    (void) argv;
	return entry_point(0, nullptr);
}
#elif defined(WIN_WINDOWS_MODE)
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	(void) hInstance;
	(void) hPrevInstance;
	(void) lpCmdLine;
	(void) nShowCmd;
	return entry_point(0, nullptr);
}
#endif

String8 get_exe_path(Allocator allocator) {
	C8 temp_buff[MAX_PATH + 1] = {};
	
	Usize path_size = GetModuleFileNameA(nullptr, temp_buff, MAX_PATH);
	if (path_size == 0) {
		return {};
	}

	return create_string_from_cstring(temp_buff, allocator);
}

static String8 get_env(const C8* env, Allocator allocator) {
	C8 temp_buff[MAX_PATH + 1] = {};
	
	Usize dir_size = GetEnvironmentVariable(env, temp_buff, sizeof(temp_buff));
	if (dir_size == 0) {
		return {};
	}

	return create_string_from_cstring(temp_buff, allocator);
}

String8 get_user_dir(Allocator allocator) {
	return get_env("USERPROFILE", allocator);
}

String8 get_config_dir(Allocator allocator) {
	return get_env("APPDATA", allocator);
}

Slice<Byte> read_entire_file(String8 path, Allocator allocator) {
	HANDLE file = INVALID_HANDLE_VALUE;
	Slice<Byte> file_buff = {};
	LARGE_INTEGER file_size;
	Usize total_read = 0;
	DWORD read = 0;
	BOOL res = FALSE;
	
	C8* path_cstring = clone_string_to_cstring(path, get_temp_allocator());
	file = CreateFileA(path_cstring, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return {};
	}
	
	res = GetFileSizeEx(file, &file_size);
	if (!res) {
		goto exit_and_clean;
	}

	file_buff = create_slice<Byte>((Usize) file_size.QuadPart, allocator);
	
	// NOTE: since 'ReadFile()' only takes a 32bit int as input (DWORD),
	// we have to do multiple calls to it until we actually have read the 
	// entire file, if we want to support files larger than 4gb
	while (total_read != (Usize) file_size.QuadPart) {
		Usize remaining_bytes = (Usize) file_size.QuadPart - total_read;
		DWORD size = (remaining_bytes > DWORD_MAX) ? DWORD_MAX : (DWORD) remaining_bytes;
		res = ReadFile(file, file_buff.data + total_read, size, &read, nullptr);
		if (!res) {
			destroy_slice<Byte>(&file_buff);
			break;
		}
		
		total_read += (Usize) read;
		LARGE_INTEGER offset = { .QuadPart = (LONGLONG) read };
		SetFilePointerEx(file, offset, nullptr, FILE_CURRENT);
	}

	exit_and_clean:
	arena_free_all((Arena_Allocator*) get_temp_allocator().data);
	CloseHandle(file);

	return file_buff;
}