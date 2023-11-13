#include "defines.h"
#include "asserts.h"
#include "core/dmemory.cpp"
#include "core/logger.cpp"
#include "core/dstring.cpp"
#include "platform_services.cpp"
#include "condition_tables.cpp"

//store conditions and their possible values
//if using internal state build permanent table
//if using external state just apply rules to given data

//bools can be bit fields
//strings can be hashmap?
//int/float can stay int/float

/*
KEYWORDS:
	- is
	- not
	- if
	- else
	- do?
	- START
	- END 
	- greater than
	- less than
	- or
	- and
	- equals
	- return
*/


int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR cmd_line, int cmd_show) {
	u8* base_address = (u8*)TeraBytes(2);
	//NOTE: this is probably overkill especially for the bool/char tables
	u32 table_memory_size = MegaBytes(1);
	u32 pages_per_table = table_memory_size / PAGE_SIZE;

	BoolTable bool_table = {};
	bool_table.Init(base_address, table_memory_size);
	base_address = base_address + table_memory_size;

	CharTable char_table = {};
	char_table.Init(base_address, table_memory_size);
	base_address = base_address + table_memory_size;
	

	return 0;
}