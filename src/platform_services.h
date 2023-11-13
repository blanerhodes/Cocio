#pragma once

void* ReservePage(void* base_address, u32 page_count);

void* CommitPage(void* base_address, u32 page_count);

void* ReserveAndCommitPage(void* base_address, u32 page_count);