#include "scanner.h"


void InitScanner(u8* src) {
	scanner.start = src;
	scanner.current = src;
	scanner.line = 1;
}

static Token MakeToken(TokenTypeC type) {
	Token token = {
		.type = type,
		.start = scanner.start,
		.length = scanner.current - scanner.start,
		.line = scanner.line
	};
	return token;
}

static Token ErrorToken(char* message) {
	Token token = {
		.type = TOKEN_ERROR,
		.start = (u8*)message,
		.length = StringLength((u8*)message),
		.line = scanner.line
	};
	return token;
}

static inline char Peek() {
	return *(scanner.current);
}

static inline char PeekNext() {
	return *(scanner.current + 1);
}

static inline b8 IsAtEnd() {
	return Peek() == '\0';
}

static inline char ScannerAdvance() {
	char result = Peek();
	scanner.current++;
	return result;
}

static b8 Match(char expected) {
	if (IsAtEnd()) {
		return false;
	}
	if (Peek() != expected) {
		return false;
	}
	scanner.current++;
	return true;
}

static void SkipWhiteSpace() {
	for (;;) {
		char curr = Peek();
		if (IsWhiteSpace(curr)) {
			if (IsEndOfLine(curr)) {
				scanner.line++;
			}
			scanner.current++;
		}
		else if (curr == '/') {
			if (PeekNext() == '/') {
				while (Peek() != '\n' && !IsAtEnd()) {
					scanner.current++;
				}
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}
}

static Token String() {
	while (Peek() != '"' && !IsAtEnd()) {
		if (Peek() == '\n')
			scanner.line++;
		scanner.current++;
	}
	if (IsAtEnd())
		return ErrorToken("Unterminated string.");
	scanner.current++;
	return MakeToken(TOKEN_STRING);
}

static TokenTypeC CheckKeyword(i32 start, i32 length, char* rest, TokenTypeC type) {
	if (scanner.current - scanner.start == start + length && StringsEqual(scanner.start + start, (u8*)rest)) {
		return type;
	}
	return TOKEN_IDENTIFIER;
}

static Token ScannerNumber() {
	while (IsDigit(Peek()))
		scanner.current++;
	if (Peek() == '.' && IsDigit(PeekNext())) {
		scanner.current++;
		while (IsDigit(Peek()))
			scanner.current++;
	}
	return MakeToken(TOKEN_NUMBER);
}

static TokenTypeC IdentifierType() {
	switch (Peek()) {
		case 'a': return CheckKeyword(1, 2, "nd", TOKEN_AND);
		case 'i': return CheckKeyword(1, 1, "f", TOKEN_IF);
		case 'o': return CheckKeyword(1, 1, "r", TOKEN_OR);
		case 'w': return CheckKeyword(1, 4, "hile", TOKEN_WHILE);
		case 't': return CheckKeyword(1, 3, "rue", TOKEN_TRUE);
		//figure out how to handle string of words for the actual tokens like "greater or equals"
		case 'g': return CheckKeyword(1, 6, "reater", TOKEN_GREATER);
		case 'l': return CheckKeyword(1, 3, "ess", TOKEN_LESS);
		case 'e': {
			if (scanner.current - scanner.start > 1) {
				switch (PeekNext()) {
					case 'l': return CheckKeyword(2, 2, "se", TOKEN_ELSE);
					case 'q': return CheckKeyword(2, 4, "uals", TOKEN_ELSE);
				}
			}
		
		}
		case 'n': {
			if (scanner.current - scanner.start > 1) {
				switch (PeekNext()) {
					case 'i': return CheckKeyword(2, 1, "l", TOKEN_NIL);
					case 'o': return CheckKeyword(2, 1, "t", TOKEN_BANG);
				}
			}
		} break; 
		case 'r': {
			if (scanner.current - scanner.start > 1) {
				switch (PeekNext()) {
					case 'e': return CheckKeyword(2, 4, "turn", TOKEN_RETURN);
					case 'u': return CheckKeyword(2, 2, "le", TOKEN_RULE);
				}
			}

		} break;
		case 'f': {
			if (scanner.current - scanner.start > 1) {
				switch (PeekNext()) {
					case 'a': return CheckKeyword(2, 3, "lse", TOKEN_FALSE);
					case 'o': return CheckKeyword(2, 1, "r", TOKEN_FOR);
				}
			}
		} break;
	}
}

static Token Identifier() {
	while (IsAlNum(Peek())) {
		scanner.current++;
	}
	return MakeToken(IdentifierType());
}

static Token ScanToken() {
	scanner.start = scanner.current;

	if (IsAtEnd()) {
		return MakeToken(TOKEN_EOF);
	}
	SkipWhiteSpace();
	char c = ScannerAdvance();
	if (IsAlpha(c))
		return Identifier();
	if (IsDigit(c))
		return ScannerNumber();
	switch (c) {
		case '(': return MakeToken(TOKEN_LEFT_PAREN);
		case ')': return MakeToken(TOKEN_RIGHT_PAREN);
		case '{': return MakeToken(TOKEN_LEFT_BRACE);
		case '}': return MakeToken(TOKEN_RIGHT_BRACE);
		case ';': return MakeToken(TOKEN_SEMICOLON);
		case ',': return MakeToken(TOKEN_COMMA);
		case '.': return MakeToken(TOKEN_DOT);
		case '-': return MakeToken(TOKEN_MINUS);
		case '+': return MakeToken(TOKEN_PLUS);
		case '/': return MakeToken(TOKEN_SLASH);
		case '*': return MakeToken(TOKEN_STAR);
		case '!': return MakeToken(Match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=': return MakeToken(Match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<': return MakeToken(Match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>': return MakeToken(Match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '"': return String();
		case '&': {
			if (Match('&')) {
				return MakeToken(TOKEN_AND);
			}
		} break;
		case '|': {
			if (Match('|')) {
				return MakeToken(TOKEN_OR);
			}
		} break;
	}

	return ErrorToken("Unexpected character.");
}

static Token* ScanTokens() {
	while (!IsAtEnd()) {
		scanner.start = scanner.current;
		ScanToken();
	}

}

