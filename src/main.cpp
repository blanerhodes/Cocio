#include "defines.h"
#include "asserts.h"
#include "core/dmemory.cpp"
#include "core/logger.cpp"
#include "core/dstring.cpp"
#include "platform_services.cpp"
#include "condition_tables.cpp"
#include "scanner.cpp"

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

static RuleTable rule_table;
static BoolTable bool_table;
static CharTable char_table;
static FloatTable float_table;
static StringTable string_table;

void OpenGate3() {
	if (bool_table.QueryCondition(GATE_1_OPEN) && bool_table.QueryCondition(GATE_2_OPEN)) {
		bool_table.SetConditionValue(GATE_3_OPEN, true);
	}
}

void ModChar3Value() {
	if (char_table.QueryCondition(CHAR1_VALUE) == 'a' && char_table.QueryCondition(CHAR2_VALUE) == 'b') {
		char_table.SetConditionValue(CHAR3_VALUE, 'd');
	}
}

void ModFloat3Value() {
	if (float_table.QueryCondition(FLOAT1_VALUE) == 1.0f && float_table.QueryCondition(FLOAT2_VALUE) == 2.0f) {
		float_table.SetConditionValue(FLOAT3_VALUE, 4.0f);
	}
}

void ModStringValue() {
	u8* str1 = string_table.QueryCondition(STRING1_VALUE);
	u8* str2 = string_table.QueryCondition(STRING2_VALUE);
	if (StringsEqual(str1, (u8*)"string1") && StringsEqual(str2, (u8*)"string2")) {
		string_table.SetConditionValue(STRING3_VALUE, (u8*)"string4");
	}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR cmd_line, int cmd_show) {
	//u8* base_address = (u8*)TeraBytes(2);
	//NOTE: this is probably overkill especially for the bool/char tables
	u32 table_memory_size = MegaBytes(1);
	u32 pages_per_table = table_memory_size / PAGE_SIZE;

	rule_table.Init(0, table_memory_size);
	rule_table.AddRule(OpenGate3);
	rule_table.AddRule(ModChar3Value);
	rule_table.AddRule(ModFloat3Value);
	rule_table.AddRule(ModStringValue);
	
	bool_table.Init(0, table_memory_size);
	bool_table.AddCondition(true);
	bool_table.AddCondition(true);
	bool_table.AddCondition(false);
	rule_table.RunRule(OPEN_GATE_3);

	char_table.Init(0, table_memory_size);
	char_table.AddCondition('a');
	char_table.AddCondition('b');
	char_table.AddCondition('c');
	rule_table.RunRule(CHANGE_CHAR3);

	float_table.Init(0, table_memory_size);
	float_table.AddCondition(1.0f);
	float_table.AddCondition(2.0f);
	float_table.AddCondition(3.0f);
	rule_table.RunRule(CHANGE_FLOAT3);

	string_table.Init(0);
	string_table.AddCondition("string1");
	string_table.AddCondition("string2");
	string_table.AddCondition("string3");
	rule_table.RunRule(CHANGE_STRING);
	u8* result = string_table.QueryCondition(STRING3_VALUE);

	Scanner scanner = {};
	char* test = "test";
	scanner.Init((u8*)test, 4);
	i32 line = 01;
	for (;;) {
		Token token = scanner.ScanToken();
		if (token.line != line) {
			DINFO("%4d", token.line);
			line = token.line;
		}
		else {
			DINFO("   | ");
		}
		DINFO("%2d '%.*s'\n", token.type, token.length, token.start);
	}

	return 0;
}