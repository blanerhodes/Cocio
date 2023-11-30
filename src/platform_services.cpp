#include "platform_services.h"

#define PAGE_SIZE KiloBytes(4)

void* ReservePage(void* base_address, u32 page_count) {
	return VirtualAlloc(base_address, page_count*PAGE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
}

void* CommitPage(void* base_address, u32 page_count) {
	return VirtualAlloc(base_address, page_count*PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);
}

void* ReserveAndCommitPage(void* base_address, u32 page_count) {
	return VirtualAlloc(base_address, page_count*PAGE_SIZE, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}

static DebugReadFileResult DebugPlatformReadEntireFile(char* filename) {
    DebugReadFileResult result = {};
    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) {
            DASSERT(file_size.QuadPart <= 0xFFFFFFFF);
            result.contents = VirtualAlloc(0, file_size.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result.contents) {
                DWORD bytes_read;
                if (ReadFile(file_handle, result.contents, file_size.QuadPart, &bytes_read, 0) && file_size.QuadPart == bytes_read) {
                    //read successfully
                    result.contents_size = file_size.QuadPart;
                }
            } else {
                DebugPlatformFreeFileMemory(result.contents);
                result.contents = 0;
            }
        }
        CloseHandle(file_handle);
    }
    return result;
}

static void DebugPlatformFreeFileMemory(void* memory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

static b32 DebugPlatformWriteEntireFile(char* filename, u32 memory_size, void* memory) {
    b32 result = false;
    HANDLE file_handle = CreateFileA(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);

    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if (WriteFile(file_handle, memory, memory_size, &bytes_written, NULL)) {
            //success
            result = bytes_written == memory_size;
        } else {
            //log error
        }
        CloseHandle(file_handle);
    } else {
        //log error
    }
    return result;
}
