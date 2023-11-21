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
		i32 result = memory.used - 1;
		return (BoolConditionId)result;
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
	
	u8 QueryCondition(CharConditionId condition) {
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

	//NOTE: pages to commit DOES NOT INCLUDE THE LOOK_ASIDE PAGE
	//Assuming look_aside_size is always 1 page for now
	void Init(u8* base_address, i32 pages_to_commit = 1, i32 look_aside_size = KiloBytes(4)) {
		i32 conditions_size = look_aside_size / sizeof(i32) * 32; //NOTE: hardcoding default slot size of 32
		i32 look_aside_pages = look_aside_size / PAGE_SIZE;

		u8* look_aside = (u8*)ReserveAndCommitPage(base_address, look_aside_pages);
		InitializeArena(&look_aside_memory, look_aside_size, look_aside);
		base_address = base_address + look_aside_size+1;

		u8* conditions = (u8*)ReservePage(0, conditions_size/PAGE_SIZE);
		conditions = (u8*)CommitPage(conditions, pages_to_commit);
		InitializeArena(&conditions_memory, conditions_size, conditions);
	}

	u8* QueryCondition(StringConditionId condition) {
		i32 condition_in_bytes = condition * sizeof(i32);
		DASSERT(condition_in_bytes <= look_aside_memory.used);
		i32 condition_offset = *(look_aside_memory.base + condition_in_bytes);
		u8* result = conditions_memory.base + condition_offset;
		return result;
	}
	
	void SetConditionValue(StringConditionId condition, u8* value) {
		u32 condition_in_bytes = condition * sizeof(i32);
		DASSERT(condition_in_bytes <= look_aside_memory.used);
		u8 value_length = StringLength((u8*)value);
		u32 condition_table_offset = *(look_aside_memory.base + condition_in_bytes);
		u8* curr_condition = conditions_memory.base + condition_table_offset;
		u8 slot_size = *(curr_condition-1);
		
		if (value_length > slot_size) {
			*(curr_condition - 1) = value_length;
			//shift all offsets by length diff
			i32 length_diff = value_length - slot_size;
		    i32* look_aside_ptr = (i32*)look_aside_memory.base;
			i32 look_aside_used_slots = look_aside_memory.used / sizeof(i32);
			for (i32 index = condition+1; index < look_aside_used_slots; index++) {
				look_aside_ptr[index] = look_aside_ptr[index]+length_diff;
			}
			//if last offset is off the end of committed memory, commit another page
			i32 curr_condition_page_count = conditions_memory.used / PAGE_SIZE;
			i32 next_condition_page_count = (conditions_memory.used + length_diff) / PAGE_SIZE;
			if (next_condition_page_count > curr_condition_page_count) {
				u8* alloc_addr = conditions_memory.base + next_condition_page_count*PAGE_SIZE;
				CommitPage(alloc_addr, 1);
			}
			u32 next_condition_table_offset = *(look_aside_memory.base + condition_in_bytes + sizeof(i32));
			u8* next_condition = conditions_memory.base + next_condition_table_offset;
			MemMove(next_condition+length_diff, next_condition, conditions_memory.used-next_condition_table_offset);
		}
		StringCopy(value, curr_condition);
	}


	//NOTE: there is a size header for each string as the first byte before the start of the string, that's why all the +1s are there
	StringConditionId AddCondition(u8* initial_value) {
		u8 value_length = StringLength(initial_value);
		i32* new_look_aside = PushType(&look_aside_memory, i32);
		i32 curr_condition_page_count = conditions_memory.used / PAGE_SIZE;
		i32 next_condition_page_count = (conditions_memory.used + value_length) / PAGE_SIZE;
		if (next_condition_page_count > curr_condition_page_count) {
			u8* alloc_addr = conditions_memory.base + next_condition_page_count*PAGE_SIZE;
			CommitPage(alloc_addr, 1);
		}
		*new_look_aside = conditions_memory.used+1;
		u8* new_condition = PushSize(&conditions_memory, value_length+1);
		*new_condition = value_length;
		StringCopy(initial_value, new_condition+1, value_length);
		return (StringConditionId)*new_look_aside;
	}

	StringConditionId AddCondition(char* initial_value) {
		return AddCondition((u8*)initial_value);
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

typedef void RuleFunc(void);

struct RuleTable {
	MemoryArena memory;

	void Init(u8* base_address, i32 total_table_size, i32 pages_to_commit = 1) {
		i32 page_count = total_table_size / PAGE_SIZE;
		void* table_memory = ReservePage(base_address, page_count);
		CommitPage(table_memory, pages_to_commit);
		InitializeArena(&memory, total_table_size, (u8*)table_memory);
	}

	void RunRule(RuleId rule) {
		u32 rule_in_bytes = rule * sizeof(void*);
		DASSERT(rule_in_bytes <= memory.used);
		RuleFunc* func = *((RuleFunc**)(memory.base + rule_in_bytes));
		func();
	}

	RuleId AddRule(RuleFunc* func) {
		i32 curr_page_count = memory.used / PAGE_SIZE;
		i32 next_page_count = (memory.used + sizeof(RuleFunc*)) / PAGE_SIZE;
		if (next_page_count > curr_page_count) {
			u8* page_alloc_addr = memory.base + next_page_count * PAGE_SIZE;
			CommitPage(page_alloc_addr, 1);
		}
		RuleFunc** func_slot = PushType(&memory, RuleFunc*);
		*func_slot = func;
		i32 result = memory.used / sizeof(RuleFunc*) - 1;
		return (RuleId)result;
	}
};
