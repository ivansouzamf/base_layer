#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// TODO: convert arguments to unix format
#if defined(WIN_CONSOLE_MODE)
int wmain(int argc, wchar_t *argv[]) {
	return entry_point(0, nullptr);
}
#elif defined(WIN_WINDOWS_MODE)
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	return entry_point(0, nullptr);
}
#endif

String8 get_exe_path() {
	C8 temp_buff[MAX_PATH + 1] = {};
	
	Usize path_size = GetModuleFileNameA(nullptr, temp_buff, MAX_PATH);
	if (path_size == 0) {
		return {};
	}

	return create_string_from(temp_buff);
}

static String8 get_env(const C8* env) {
	C8 temp_buff[MAX_PATH + 1] = {};
	
	Usize dir_size = GetEnvironmentVariable(env, temp_buff, sizeof(temp_buff));
	if (dir_size == 0) {
		return {};
	}

	return create_string_from(temp_buff);
}

String8 get_user_dir() {
	return get_env("USERPROFILE");
}

String8 get_config_dir() {
	return get_env("APPDATA");
}

Slice<Byte> read_entire_file(String8 path, Allocator allocator) {
	HANDLE file = INVALID_HANDLE_VALUE;
	Slice<Byte> file_buff = {};
	DWORD file_size = 0;
	BOOL res = FALSE;
	
	// FIXME: convert 'path' to an actual null-terminated string
	// because that could cause problems on the future
	file = CreateFileA(path.data, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return {};
	}
	
	// NOTE: this only supports files up to 4gb since DWORD is 32bit.
	// info about how to get a 64bit integer as result: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
	file_size = GetFileSize(file, nullptr);
	if (file_size == INVALID_FILE_SIZE) {
		goto exit_and_clean;
	}

	file_buff = create_slice<Byte>((Usize) file_size, allocator);
	res = ReadFile(file, file_buff.data, file_size, nullptr, nullptr);
	if (!res) {
		destroy_slice<Byte>(&file_buff);
	}

	exit_and_clean:
	CloseHandle(file);

	return file_buff;
}