#pragma once

void* ReservePage(void* base_address, u32 page_count);

void* CommitPage(void* base_address, u32 page_count);

void* ReserveAndCommitPage(void* base_address, u32 page_count);

struct DebugReadFileResult {
    u32 contents_size;
    void* contents;
};

static DebugReadFileResult DebugPlatformReadEntireFile(char* filename);

static void DebugPlatformFreeFileMemory(void* memory);

static b32 DebugPlatformWriteEntireFile(char* filename, u32 memory_size, void* memory);

void Exit(u32 code) {
    ExitProcess(code);
}
