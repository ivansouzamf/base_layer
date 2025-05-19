#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	return entry_point(argc, argv);
}

void assert_release(const C8* message, const C8* file, const U32 line) {
    const C8* yad_cmd = "yad --title=\"Error\" --text=\"%s\" --button=OK:0 --fixed --window-icon=error --image=dialog-error --center";
    const C8* zenity_cmd = "zenity --text=\"%s\" --error";
    C8* cmd;

    C8 final_msg[1024] = {};
    C8 final_cmd[2048] = {};

    snprintf(final_msg, sizeof(final_msg), "%s\n%s:%u\n", message, file, line);

    if (system("zenity --help > /dev/null") == 0) {
        cmd = (C8*) zenity_cmd;
    } else if (system("yad --help > /dev/null") == 0) {
        cmd = (C8*) yad_cmd;
    } else {
        // if we can't display a dialog window just print it
        // on stderr
        fprintf(stderr, "\033[31mError\033[0m: %s\n", final_msg);
        goto exit;
    }

    snprintf(final_cmd, sizeof(final_cmd), cmd, final_msg);
    system(final_cmd);

    exit:
    exit(-1);
}

String8 get_exe_path(Allocator allocator) {
	C8 temp_buff[PATH_MAX + 1] = {};

	Usize path_size = readlink("/proc/self/exe", temp_buff, PATH_MAX);
	if (path_size <= 0) {
		return {};
	}

	return create_string_from_cstring(temp_buff, allocator);
}

String8 get_user_dir(Allocator allocator) {
	C8* dir = getenv("HOME");
	if (dir == nullptr) {
		return {};
	}

	return create_string_from_cstring(dir, allocator);
}

String8 get_config_dir(Allocator allocator) {
	// NOTE: specified here https://specifications.freedesktop.org/basedir-spec/latest/#variables

	C8* dir = getenv("XDG_CONFIG_HOME");
	if (dir != nullptr) {
		return create_string_from_cstring(dir, allocator);
	}

	// if $XDG_CONFIG_HOME not setted we default to $HOME/.config
	String8 config_dir = lit_string("/.config");
	String8 home_dir = get_user_dir(get_temp_allocator());
	if (home_dir.lenght == 0) {
	    return {};
	}

	String8 final_dir = alloc_string(home_dir.lenght + config_dir.lenght, allocator);
	copy_memory(final_dir.data, home_dir.data, home_dir.lenght);
	copy_memory(final_dir.data + home_dir.lenght, config_dir.data, config_dir.lenght);

	arena_free_all((Arena_Allocator*) get_temp_allocator().data);

	return final_dir;
}

Slice<Byte> read_entire_file(String8 path, Allocator allocator) {
	C8* path_cstring = clone_string_to_cstring(path, get_temp_allocator());

	FILE* file = fopen(path_cstring, "rb");
	if (file == nullptr) {
	    arena_free_all((Arena_Allocator*) get_temp_allocator().data);
		return {};
	}

	fseek(file, 0, SEEK_END);
	Usize file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	Slice<Byte> file_buff = create_slice<Byte>(file_size, allocator);
	Usize read = fread(file_buff.data, file_buff.size, 1, file);
	if ((read * file_buff.size) != file_buff.size) {
		destroy_slice(&file_buff);
	}

	arena_free_all((Arena_Allocator*) get_temp_allocator().data);
	fclose(file);

	return file_buff;
}