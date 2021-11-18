
enum Player_Stats {
	NUM_OF_MATCHES1,
	MATCHES_WON1,
	MATCHES_LOST1,
	POINTS_SCORED1,
	POINTS_LOST1,

	NUM_OF_MATCHES2,
	MATCHES_WON2,
	MATCHES_LOST2,
	POINTS_SCORED2,
	POINTS_LOST2,

	STATS_COUNT,
};

struct {
	char* data;
	unsigned int size;
} typedef String;

struct {
	u32 version;
	unsigned int stats[STATS_COUNT];
} typedef Save_Data;

Save_Data save_data = {};

// ------------ Helper Functions for Stats -----------------------
#include <cassert>

internal void
os_free_file(String s) {
	VirtualFree(s.data, 0, MEM_RELEASE);
}

internal String
os_read_entire_file(const char* file_path) {
	String result = { 0 };

	HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (file_handle == INVALID_HANDLE_VALUE) {
		CloseHandle(file_handle);
		return result;
	}

	DWORD file_size = GetFileSize(file_handle, 0);
	result.size = file_size;
	result.data = (char*)VirtualAlloc(0, result.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


	DWORD bytes_read;
	if (ReadFile(file_handle, result.data, file_size, &bytes_read, 0) && file_size == bytes_read) {
		// Success;

	}
	else {
		// @Incomplete: error message?
		assert(0);
	}

	CloseHandle(file_handle);
	return result;
}

internal String
os_read_save_file() {
	return os_read_entire_file("save.pongsav");
}

internal int
os_write_save_file(String data) {
	int result = false;

	HANDLE file_handle = CreateFileA("save.pongsav", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (file_handle == INVALID_HANDLE_VALUE) {
		assert(0);
		return result;
	}

	DWORD bytes_written;
	result = WriteFile(file_handle, data.data, (DWORD)data.size, &bytes_written, 0) && bytes_written == data.size;

	CloseHandle(file_handle);
	return result;
}

internal void
load_game() {
	String input = os_read_save_file();
	if (input.size) {
		u32 version = *(u32*)input.data;
		save_data = *(Save_Data*)input.data;
	}
}

internal void
save_game() {
	// Do that async
	String data;
	data.data = (char*)&save_data;
	data.size = sizeof(save_data);
	os_write_save_file(data);
}
