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
