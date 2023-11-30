#pragma once

enum ValueType {
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
	VAL_STRING
};

struct Value {
	ValueType type;
	union {
		b32 boolean;
		f32 number;
		u8* string;
	};
};

struct ValueArray {
	i32 capacity;
	i32 count;
	Value* values;
};

struct Chunk {
	i32 count;
	i32 capacity;
	u8* code;
	i32* lines;
	ValueArray constants;
};

enum OpCode {
	OP_CONSTANT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
	OP_RETURN
};

#define STACK_MAX 256
struct VM {
	Chunk* chunk;
	u8* ip;
	Value stack[STACK_MAX];
	Value* stack_top;
};

enum InterpretResult {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
};

struct Parser {
	Token current;
	Token previous;
	b8 had_error;
	b8 panic_mode;
};

enum Precedence {
	PREC_NONE,
	PREC_ASSIGNMNET,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY
};

typedef void (*ParseFn)();

struct ParseRule {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
};

#define DEFAULT_CHUNK_SIZE 1024

#define AsBool(value)    ((value).boolean)
#define AsNumber(value)  ((value).number)
#define AsString(value)  ((value).string)

#define IsBool(value)    ((value).type == VAL_BOOL)
#define IsNil(value)     ((value).type == VAL_NIL)
#define IsNumber(value)  ((value).type == VAL_NUMBER)
#define IsString(value)  ((value).type == VAL_STRING)

