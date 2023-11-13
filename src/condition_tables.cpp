#include "condition_tables.h"

//NOTE: leaving bool table and char table separate in case I want to turn the bool table into a bit array
struct BoolTable {
	MemoryArena memory;

	void Init(u8* base_address, i32 total_table_size, i32 pages_to_commit = 1) {
		i32 page_count = total_table_size / PAGE_SIZE;
		ReservePage(base_address, page_count);
		u8* table_memory = (u8*)CommitPage(base_address, pages_to_commit);
		InitializeArena(&memory, total_table_size, table_memory);
	}
	
	b8 QueryCondition(BoolConditionId condition) {
		DASSERT(condition <= memory.used);
		b8 result = *(memory.base + condition);
		return result;
	}

	void SetConditionValue(BoolConditionId condition, b8 value) {
		DASSERT(condition <= memory.used);
		*(memory.base + condition) = value;
	}

	BoolConditionId AddCondition(b8 initial_value) {
		i32 current_page_count = memory.used / PAGE_SIZE;
		i32 next_page_count = (memory.used+1) / PAGE_SIZE;
		if (next_page_count > current_page_count) {
			u8* alloc_addr = memory.base + memory.used;
			CommitPage(alloc_addr, 1);
		}
		u8* index = PushSize(&memory, 1);
		*index = initial_value;
		return (BoolConditionId)memory.used;
	}
};

struct CharTable {
	MemoryArena memory;

	void Init(u8* base_address, i32 total_table_size, i32 pages_to_commit = 1) {
		i32 page_count = total_table_size / PAGE_SIZE;
		ReservePage(base_address, page_count);
		u8* table_memory = (u8*)CommitPage(base_address, pages_to_commit);
		InitializeArena(&memory, total_table_size, table_memory);
	}
	
	b8 QueryCondition(CharConditionId condition) {
		DASSERT(condition <= memory.used);
		b8 result = *(memory.base + condition);
		return result;
	}

	void SetConditionValue(CharConditionId condition, u8 value) {
		DASSERT(condition <= memory.used);
		*(memory.base + condition) = value;
	}

	CharConditionId AddCondition(u8 initial_value) {
		i32 current_page_count = memory.used / PAGE_SIZE;
		i32 next_page_count = (memory.used+1) / PAGE_SIZE;
		if (next_page_count > current_page_count) {
			u8* alloc_addr = memory.base + memory.used;
			CommitPage(alloc_addr, 1);
		}
		u8* index = PushSize(&memory, 1);
		*index = initial_value;
		return (CharConditionId)memory.used;
	}

};

struct StringTable {
	MemoryArena look_aside_memory;
	MemoryArena conditions_memory;
	i32 slot_size;

	//NOTE: pages to commit DOES NOT INCLUDE THE LOOK_ASIDE PAGE
	//Assuming look_aside_size is always 1 page for now
	void Init(u8* base_address, i32 pages_to_commit = 1, i32 look_aside_size = KiloBytes(4)) {
		slot_size = 32; //NOTE: hardcoded for a condition's string value to hold 32 chars at first
		i32 conditions_size = look_aside_size / sizeof(i32) * slot_size;
		i32 look_aside_pages = look_aside_size / PAGE_SIZE;

		u8* look_aside = (u8*)ReserveAndCommitPage(base_address, look_aside_pages);
		InitializeArena(&look_aside_memory, look_aside_size, look_aside);
		base_address = base_address + look_aside_size;

		ReservePage(base_address, conditions_size/PAGE_SIZE);
		u8* conditions = (u8*)CommitPage(base_address, pages_to_commit);
		InitializeArena(&conditions_memory, conditions_size, conditions);
	}

	u8* QueryCondition(StringConditionId condition) {
		i32 condition_in_bytes = condition * sizeof(i32);
		DASSERT(condition_in_bytes <= look_aside_memory.used);
		i32 condition_offset = *(look_aside_memory.base + condition_in_bytes);
		u8* result = conditions_memory.base + condition_offset;
		return result;
	}
	
	//TODO: fix length comparison to resize only if new value is bigger than the slot NOT the current value's length
	//      maybe fix it with a one byte header holding the count?
	void SetConditionValue(StringConditionId condition, u8* value, i32 size) {
		u32 condition_in_bytes = condition * sizeof(i32);
		DASSERT(condition_in_bytes <= look_aside_memory.used);
		u32 value_length = StringLength((u8*)value);
		u32 condition_offset = *(look_aside_memory.base + condition_in_bytes);
		u8* curr_condition = conditions_memory.base + condition_offset;
		u32 curr_condition_length = StringLength(value);
		
		if (value_length > curr_condition_length) {
			//shift all offsets by length diff
			i32 last_condition_length = StringLength(QueryCondition((StringConditionId)(look_aside_memory.used / sizeof(condition))));
			i32 length_diff = value_length - curr_condition_length;
		    i32* look_aside_int_ptr = (i32*)look_aside_memory.base;
			for (i32 index = condition+1; index*sizeof(i32) < look_aside_memory.used; index++) {
				look_aside_int_ptr[index] = look_aside_int_ptr[index]+length_diff;
			}
			//if last offset is off the end of committed memory, commit another page
			i32 curr_condition_page_count = conditions_memory.used / PAGE_SIZE;
			i32 next_condition_page_count = (conditions_memory.used + length_diff) / PAGE_SIZE;
			if (next_condition_page_count > curr_condition_page_count) {
				u8* alloc_addr = conditions_memory.base + next_condition_page_count*PAGE_SIZE;
				CommitPage(alloc_addr, 1);
			}
			MemMove(curr_condition, curr_condition+length_diff, conditions_memory.used-condition_offset);
		}
		StringCopy(value, curr_condition);
	}

	StringConditionId AddCondition(u8* initial_value) {
		i32 value_length = StringLength(initial_value);
		i32* new_look_aside = PushType(&look_aside_memory, i32);
		i32 curr_condition_page_count = conditions_memory.used / PAGE_SIZE;
		i32 next_condition_page_count = (conditions_memory.used + value_length) / PAGE_SIZE;
		if (next_condition_page_count > curr_condition_page_count) {
			u8* alloc_addr = conditions_memory.base + next_condition_page_count*PAGE_SIZE;
			CommitPage(alloc_addr, 1);
		}
		*new_look_aside = conditions_memory.used;
		u8* new_condition = PushSize(&conditions_memory, value_length);
		StringCopy(initial_value, new_condition);
		return (StringConditionId)*new_look_aside;
	}
};

struct FloatTable {
	MemoryArena memory;
	
	void Init(u8* base_address, i32 total_table_size, i32 pages_to_commit = 1) {
		i32 page_count = total_table_size / PAGE_SIZE;
		ReservePage(base_address, page_count);
		u8* table_memory = (u8*)CommitPage(base_address, pages_to_commit);
		InitializeArena(&memory, total_table_size, table_memory);
	}

	f32 QueryCondition(FloatConditionId condition) {
		DASSERT(condition*sizeof(f32) <= memory.used);
		f32* base_ptr = (f32*)memory.base;
		f32 result = *(base_ptr+condition);
		return result;
	}

	void SetConditionValue(FloatConditionId condition, f32 value) {
		DASSERT(condition*sizeof(f32) <= memory.used);
		f32* base_ptr = (f32*)memory.base;
		*(base_ptr+condition) = value;
	}

	FloatConditionId AddCondition(f32 initial_value) {
		i32 curr_page_count = memory.used / PAGE_SIZE;
		i32 next_page_count = (memory.used+sizeof(f32)) / PAGE_SIZE;
		if (next_page_count > curr_page_count) {
			u8* page_alloc_addr = memory.base + next_page_count*PAGE_SIZE;
			CommitPage(page_alloc_addr, 1);
		}
		f32* new_condition = PushType(&memory, f32);
		*new_condition = initial_value;
		return (FloatConditionId)(memory.used / sizeof(f32));
	}
};
