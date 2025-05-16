#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	return entry_point(argc, argv);
}

String8 get_exe_path() {
	C8 temp_buff[PATH_MAX + 1] = {};

	Usize path_size = readlink("/proc/self/exe", temp_buff, PATH_MAX);
	if (path_size <= 0) {
		return {};
	}

	return create_string_from(temp_buff);
}

String8 get_user_dir() {
	C8* dir = getenv("HOME");
	if (dir == nullptr) {
		return {};
	}

	return create_string_from(dir);
}

String8 get_config_dir() {
	// NOTE: specified here https://specifications.freedesktop.org/basedir-spec/latest/#variables

	C8* dir = getenv("XDG_CONFIG_HOME");
	if (dir != nullptr) {
		return create_string_from(dir);
	}

	String8 final_dir;

	// if $XDG_CONFIG_HOME not setted we default to $HOME/.config
	String8 config_dir = lit_string("/.config");
	String8 home_dir = get_user_dir();
	if (home_dir.len == 0) {
		goto exit_and_clean;
	}

	final_dir = create_string(home_dir.len + config_dir.len);
	append_string(&final_dir, home_dir);
	append_string(&final_dir, config_dir);

	exit_and_clean:
	destroy_string(&home_dir);

	return final_dir;
}

Slice<Byte> read_entire_file(String8 path, Allocator allocator) {
	// FIXME: convert 'path' to an actual null-terminated string
	// because that could cause problems on the future
	FILE* file = fopen(path.data, "rb");
	if (file == nullptr) {
		return {};
	}

	fseek(file, 0, SEEK_END);
	Usize file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	Slice<Byte> file_buff = create_slice<Byte>(file_size, allocator);
	Usize read = fread(file_buff.data, file_buff.len, 1, file);
	if ((read * file_buff.len) != file_buff.len) {
		destroy_slice(&file_buff);
	}

	fclose(file);

	return file_buff;
}