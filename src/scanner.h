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
		b8 boolean;
		f32 number;
		u8* string;
	};
};

#define BoolVal(value)   ((Value){VAL_BOOL, value})
#define NilVal           ((Value){VAL_NIL, 0})
#define NumberVal(value) ((Value){VAL_NUMBER, value})
#define StringVal(value) ((Value){VAL_STRING, value})

#define AsBool(value)    ((value).boolean)
#define AsNumber(value)  ((value).number)
#define AsString(value)  ((value).string)

#define IsBool(value)    ((value).type == VAL_BOOL)
#define IsNil(value)     ((value).type == VAL_NIL)
#define IsNumber(value)  ((value).type == VAL_NUMBER)
#define IsString(value)  ((value).type == VAL_STRING)

enum TokenTypeC {
	// Single-character tokens. Probably don't need these
	TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
	TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

	// One or two character tokens.
	TOKEN_BANG, TOKEN_BANG_EQUAL,
	TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER, TOKEN_GREATER_EQUAL,
	TOKEN_LESS, TOKEN_LESS_EQUAL,

	// Literals.
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

	// Keywords.
	TOKEN_AND, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_IF, TOKEN_NIL,
	TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_TRUE, TOKEN_WHILE, TOKEN_RULE,

	TOKEN_ERROR, TOKEN_EOF
};

struct Token {
	TokenTypeC type;
	u8* start;
	i32 length;
	i32 line;
};

struct Scanner {
	u8* start;
	u8* current;
	int line;
};
