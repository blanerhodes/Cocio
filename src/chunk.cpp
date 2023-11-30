#include "chunk.h"

Parser parser;
Chunk* compiling_chunk;

inline Value BoolVal(b32 value) {
	Value v = {
		.type = VAL_BOOL,
		.boolean = value
	};
	return v;
}

inline Value NilVal() {
	Value v = {
		.type = VAL_NUMBER,
		.number = 0
	};
	return v;
}

inline Value NumberVal(f32 value) {
	Value v = {
		.type = VAL_NUMBER,
		.number = value
	};
	return v;
}

inline Value StringVal(u8* value) {
	Value v = {
		.type = VAL_STRING,
		.string = value
	};
	return v;
}

#define DEBUG_TRACE_EXEC
#define DEBUG_PRINT_CODE

void WriteChunk(Chunk* chunk, u8 byte, i32 line) {
	DASSERT(chunk->count+1 <= chunk->capacity);
	*(chunk->code + chunk->count) = byte; 
	*(chunk->lines + chunk->count) = line;
	chunk->count++;
}

void PrintValue(Value value) {
	switch (value.type) {
		case VAL_BOOL: {
			if (AsBool(value)) {
				DDEBUGN("true");
			} else {
				DDEBUGN("false");
			}
		} break;
		case VAL_NIL: {
			DDEBUGN("NIL");
		} break;
		case VAL_NUMBER: {
			DDEBUGN("%.3f", AsNumber(value));
		} break;
		case VAL_STRING: {
			DDEBUGN("%s", AsString(value));
		} break;
		default:
			DDEBUGN("Unknown ValueType");
	}
}

i32 SimpleInstruction(char* name, i32 offset) {
	DDEBUGN("%s", name);
	DDEBUGN("\n");
	return offset + 1;
}

i32 ConstantInstruction(char* name, Chunk* chunk, i32 offset) {
	u8 constant_offset = *(chunk->code + offset + 1);
	DDEBUGN("%-16s %4d '", name, constant_offset);
	PrintValue(*(chunk->constants.values + constant_offset));
	DDEBUGN("'\n");
	return offset + 2;
}

i32 DisassembleInstruction(Chunk* chunk, i32 offset) {
	DDEBUGN("%04d ", offset);
	if (offset > 0 && *(chunk->lines + offset) == *(chunk->lines + offset - 1)) {
		DDEBUGN("   | ");
	} else {
		DDEBUGN("%4d ", *(chunk->lines + offset));
	}
	u8 instruction = *(chunk->code + offset);
	switch (instruction) {
		case OP_CONSTANT:
			return ConstantInstruction("OP_CONSTANT", chunk, offset);
		case OP_ADD:
			return SimpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT:
			return SimpleInstruction("OP_SUBTRACT", offset);
		case OP_MULTIPLY:
			return SimpleInstruction("OP_MULTIPLY", offset);
		case OP_DIVIDE:
			return SimpleInstruction("OP_DIVIDE", offset);
		case OP_NEGATE:
			return SimpleInstruction("OP_NEGATE", offset);
		case OP_RETURN:
			return SimpleInstruction("OP_RETURN", offset);
		default:
			DDEBUG("Unknown opcode %d", instruction);
			return offset + 1;
	}
}

void DisassembleChunk(Chunk* chunk, char* name) {
	DDEBUG("== %s ==", name);
	for (i32 i = 0; i < chunk->count;) {
		i = DisassembleInstruction(chunk, i);
	}
}

#define DEFAULT_VALUE_ARRAY_CAPACITY 512

void InitValueArray(MemoryArena* memory, ValueArray* array) {
	array->values = PushArray(memory, DEFAULT_VALUE_ARRAY_CAPACITY, Value);
	array->capacity = DEFAULT_VALUE_ARRAY_CAPACITY;
	array->count = 0;
}

void WriteValueArray(ValueArray* array, Value value) {
	DASSERT(array->count + 1 <= array->capacity);
	*(array->values + array->count) = value;
	array->count++;
}

i32 AddConstant(Chunk* chunk, Value value) {
	WriteValueArray(&chunk->constants, value);
	return chunk->constants.count - 1;
}

void InitChunk(MemoryArena* memory, Chunk* chunk) {
	chunk->count = 0;
	chunk->code = PushSize(memory, DEFAULT_CHUNK_SIZE);
	chunk->capacity = DEFAULT_CHUNK_SIZE;
	chunk->lines = (i32*)PushArray(memory, DEFAULT_CHUNK_SIZE, i32);
	InitValueArray(memory, &chunk->constants);
}

void InitVM(VM* vm) {
	vm->chunk = 0;
	vm->ip = 0;
	vm->stack_top = vm->stack;
}

void ResetStack(VM* vm) {
	vm->stack_top = vm->stack;
}

void Push(VM* vm, Value value) {
	*(vm->stack_top) = value;
	vm->stack_top++;
}

Value Pop(VM* vm) {
	vm->stack_top--;
	return *(vm->stack_top);
}

InterpretResult Run(VM* vm) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (*(vm->chunk->constants.values + READ_BYTE()))
#define BINARY_OP(op) \
	do { \
		f32 b = AsNumber(Pop(vm)); \
		f32 a = AsNumber(Pop(vm)); \
		Push(vm, NumberVal(a op b)); \
	} while (false)
	
	for (;;) {
#ifdef DEBUG_TRACE_EXEC
		DDEBUGN("          ");
		for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
			DDEBUGN("[ ");
			PrintValue(*slot);
			DDEBUGN(" ]");
		}
		DDEBUGN("\n");
		DisassembleInstruction(vm->chunk, (i32)(vm->ip - vm->chunk->code));
#endif
		u8 instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				Push(vm, constant);
			} break;
			case OP_ADD: 
				BINARY_OP(+); break;
			case OP_SUBTRACT: 
				BINARY_OP(-); break;
			case OP_MULTIPLY: 
				BINARY_OP(*); break;
			case OP_DIVIDE: 
				BINARY_OP(/); break;
			case OP_NEGATE: {
				(vm->stack_top-1)->number = -(vm->stack_top-1)->number;
			} break;
			case OP_RETURN: {
				PrintValue(Pop(vm));
				DDEBUGN("\n");
				return INTERPRET_OK;
			}
		}
	}
#undef READ_CONSTANT
#undef READ_BYTE
#undef BINARY_OP
}

void ErrorAt(Token* token, u8* message) {
	if (parser.panic_mode)
		return;
	parser.panic_mode = true;
	DDEBUGN("[line %d] Error ", token->line);
	if (token->type == TOKEN_EOF) {
		DDEBUGN(" at end");
	}
	else if (token->type == TOKEN_ERROR) {

	}
	else {
		DDEBUGN(" at '%.*s'", token->length, token->start);
	}
	DDEBUG(": %s", message);
	parser.had_error = true;
}

void Error(u8* message) {
	ErrorAt(&parser.previous, message);
}

void ErrorAtCurrent(u8* message) {
	ErrorAt(&parser.current, message);
}

void ParserAdvance() {
	parser.previous = parser.current;

	for (;;) {
		parser.current = ScanToken();
		if (parser.current.type != TOKEN_ERROR)
			break;
		ErrorAtCurrent(parser.current.start);
	}
}

void Consume(TokenTypeC type, u8* message) {
	if (parser.current.type == type) {
		ParserAdvance();
		return;
	}
	ErrorAtCurrent(message);
}

Chunk* CurrentChunk() {
	return compiling_chunk;
}

void EmitByte(u8 byte) {
	WriteChunk(CurrentChunk(), byte, parser.previous.line);
}

void EmitBytes(u8 byte1, u8 byte2) {
	EmitByte(byte1);
	EmitByte(byte2);
}

void EmitReturn() {
	EmitByte(OP_RETURN);
}

void EndCompiler() {
	EmitReturn();
#ifdef DEBUG_PRINT_CODE
	if (!parser.had_error) {
		DisassembleChunk(CurrentChunk(), "code");
	}
#endif
}

static void Expression();
static ParseRule* GetRule(TokenTypeC type);
static void ParsePrecedence(Precedence precedence);


b8 Compile(u8* src, Chunk* chunk) {
	InitScanner(src);
	compiling_chunk = chunk;
	parser.had_error = false;
	parser.panic_mode = false;
	ParserAdvance();
	Expression();
	Consume(TOKEN_EOF, (u8*)"Expect end of expression.");
	EndCompiler();
	return !parser.had_error;
}

InterpretResult Interpret(VM* vm, MemoryArena* memory, u8* src) {
	Chunk chunk;
	InitChunk(memory, &chunk);

	if (!Compile(src, &chunk)) {
		return INTERPRET_COMPILE_ERROR;
	}
	return INTERPRET_OK;

	vm->chunk = &chunk;
	vm->ip = vm->chunk->code;
	InterpretResult result = Run(vm);
	return result;
}

void Repl(VM* vm, MemoryArena* memory, u8* data) {
	u8 line[1024];
	u8* seek_ptr = data;
	for (;;) {
		DDEBUGN("> ");
		if ((data += GetLine(data, line)) == 0) {
			DDEBUGN("\n");
			break;
		}
		Interpret(vm, memory, data);
	}
}

void RunFile(VM* vm, MemoryArena* memory, char* path) {
	DebugReadFileResult file = DebugPlatformReadEntireFile(path);
	
	u8* data = (u8*)file.contents;
	InterpretResult result = Interpret(vm, memory, data);

	if (result == INTERPRET_COMPILE_ERROR)
		Exit(65);
	if (result == INTERPRET_RUNTIME_ERROR)
		Exit(70);
}

u8 MakeConstant(Value value) {
	i32 constant = AddConstant(CurrentChunk(), value);
	if (constant > UINT8_MAX) {
		Error((u8*)"Too many constants in one chunk.");
		return 0;
	}
	return (u8)constant;
}

void EmitConstant(Value value) {
	EmitBytes(OP_CONSTANT, MakeConstant(value));
}

void ParserNumber() {
	f32 value = StringToF32(parser.previous.start);
	EmitConstant(NumberVal(value));
}

void Grouping() {
	Expression();
	Consume(TOKEN_RIGHT_PAREN, (u8*)"Expect ') after expression.");
}

void Unary() {
	TokenTypeC operator_type = parser.previous.type;
	ParsePrecedence(PREC_UNARY);
	switch (operator_type) {
		case TOKEN_MINUS: EmitByte(OP_NEGATE); break;
		default: return;
	}
}

void Binary() {
	TokenTypeC operator_type = parser.previous.type;
	ParseRule* rule = GetRule(operator_type);
	ParsePrecedence((Precedence)(rule->precedence + 1));
	switch (operator_type) {
		case TOKEN_PLUS: EmitByte(OP_ADD); break;
		case TOKEN_MINUS: EmitByte(OP_SUBTRACT); break;
		case TOKEN_STAR: EmitByte(OP_MULTIPLY); break;
		case TOKEN_SLASH: EmitByte(OP_DIVIDE); break;
		default: return;
	}
}

static void ParsePrecedence(Precedence precedence) {
	ParserAdvance();
	ParseFn PrefixRule = GetRule(parser.previous.type)->prefix;
	if (PrefixRule == NULL) {
		Error((u8*)"Expect expression.");
		return;
	}
	PrefixRule();

	while (precedence <= GetRule(parser.current.type)->precedence) {
		ParserAdvance();
		ParseFn InfixRule = GetRule(parser.previous.type)->infix;
		InfixRule();
	}
}

static void Expression() {
	ParsePrecedence(PREC_ASSIGNMNET);
}


ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {Grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {Unary,    Binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     Binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     Binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     Binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {ParserNumber,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  //[TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  //[TOKEN_FUN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  //[TOKEN_SUPER] = {NULL,     NULL,   PREC_NONE},
  //[TOKEN_THIS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  //[TOKEN_VAR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RULE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static ParseRule* GetRule(TokenTypeC type) {
	return &rules[type];
}
