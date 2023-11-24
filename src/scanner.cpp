#include "scanner.h"

void InitScanner(Scanner* scanner, u8* src) {
	scanner->start = src;
	scanner->current = src;
	scanner->line = 1;
}

Token MakeToken(Scanner* scanner, TokenTypeC type) {
	Token token = {
		.type = type,
		.start = scanner->start,
		.length = scanner->current - scanner->start,
		.line = scanner->line
	};
	return token;
}

Token ErrorToken(Scanner* scanner, char* message) {
	Token token = {
		.type = TOKEN_ERROR,
		.start = (u8*)message,
		.length = StringLength((u8*)message),
		.line = scanner->line
	};
	return token;
}

inline char Peek(Scanner* scanner) {
	return *(scanner->current);
}

inline char PeekNext(Scanner* scanner) {
	return *(scanner->current + 1);
}

inline b8 IsAtEnd(Scanner* scanner) {
	return Peek(scanner) == '\0';
}

inline char Advance(Scanner* scanner) {
	char result = Peek(scanner);
	scanner->current++;
	return result;
}

b8 Match(Scanner* scanner, char expected) {
	if (IsAtEnd(scanner)) {
		return false;
	}
	if (Peek(scanner) != expected) {
		return false;
	}
	scanner->current++;
	return true;
}

void SkipWhiteSpace(Scanner* scanner) {
	for (;;) {
		char curr = Peek(scanner);
		if (IsWhiteSpace(curr)) {
			if (IsEndOfLine(curr)) {
				scanner->line++;
			}
			scanner->current++;
		}
		else if (curr == '/') {
			if (PeekNext(scanner) == '/') {
				while (Peek(scanner) != '\n' && !IsAtEnd(scanner)) {
					scanner->current++;
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

Token String(Scanner* scanner) {
	while (Peek(scanner) != '"' && !IsAtEnd(scanner)) {
		if (Peek(scanner) == '\n')
			scanner->line++;
		scanner->current++;
	}
	if (IsAtEnd(scanner))
		return ErrorToken(scanner, "Unterminated string.");
	scanner->current++;
	return MakeToken(scanner, TOKEN_STRING);
}

TokenTypeC CheckKeyword(Scanner* scanner, i32 start, i32 length, char* rest, TokenTypeC type) {
	if (scanner->current - scanner->start == start + length && StringsEqual(scanner->start + start, (u8*)rest)) {
		return type;
	}
	return TOKEN_IDENTIFIER;
}

Token Number(Scanner* scanner) {
	while (IsDigit(Peek(scanner)))
		scanner->current++;
	if (Peek(scanner) == '.' && IsDigit(PeekNext(scanner))) {
		scanner->current++;
		while (IsDigit(Peek(scanner)))
			scanner->current++;
	}
	return MakeToken(scanner, TOKEN_NUMBER);
}

TokenTypeC IdentifierType(Scanner* scanner) {
	switch (Peek(scanner)) {
		case 'a': return CheckKeyword(scanner, 1, 2, "nd", TOKEN_AND);
		case 'i': return CheckKeyword(scanner, 1, 1, "f", TOKEN_IF);
		case 'o': return CheckKeyword(scanner, 1, 1, "r", TOKEN_OR);
		case 'w': return CheckKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
		case 't': return CheckKeyword(scanner, 1, 3, "rue", TOKEN_TRUE);
		//figure out how to handle string of words for the actual tokens like "greater or equals"
		case 'g': return CheckKeyword(scanner, 1, 6, "reater", TOKEN_GREATER);
		case 'l': return CheckKeyword(scanner, 1, 3, "ess", TOKEN_LESS);
		case 'e': {
			if (scanner->current - scanner->start > 1) {
				switch (PeekNext(scanner)) {
					case 'l': return CheckKeyword(scanner, 2, 2, "se", TOKEN_ELSE);
					case 'q': return CheckKeyword(scanner, 2, 4, "uals", TOKEN_ELSE);
				}
			}
		
		}
		case 'n': {
			if (scanner->current - scanner->start > 1) {
				switch (PeekNext(scanner)) {
					case 'i': return CheckKeyword(scanner, 2, 1, "l", TOKEN_NIL);
					case 'o': return CheckKeyword(scanner, 2, 1, "t", TOKEN_BANG);
				}
			}
		} break; 
		case 'r': {
			if (scanner->current - scanner->start > 1) {
				switch (PeekNext(scanner)) {
					case 'e': return CheckKeyword(scanner, 2, 4, "turn", TOKEN_RETURN);
					case 'u': return CheckKeyword(scanner, 2, 2, "le", TOKEN_RULE);
				}
			}

		} break;
		case 'f': {
			if (scanner->current - scanner->start > 1) {
				switch (PeekNext(scanner)) {
					case 'a': return CheckKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
					case 'o': return CheckKeyword(scanner, 2, 1, "r", TOKEN_FOR);
				}
			}
		} break;
	}
}

Token Identifier(Scanner* scanner) {
	while (IsAlNum(Peek(scanner))) {
		scanner->current++;
	}
	return MakeToken(scanner, IdentifierType(scanner));
}

Token ScanToken(Scanner* scanner) {
	scanner->start = scanner->current;

	if (IsAtEnd(scanner)) {
		return MakeToken(scanner, TOKEN_EOF);
	}

	char c = Advance(scanner);
	if (IsAlpha(c))
		return Identifier(scanner);
	if (IsDigit(c))
		return Number(scanner);
	switch (c) {
		case '(': return MakeToken(scanner, TOKEN_LEFT_PAREN);
		case ')': return MakeToken(scanner, TOKEN_RIGHT_PAREN);
		case '{': return MakeToken(scanner, TOKEN_LEFT_BRACE);
		case '}': return MakeToken(scanner, TOKEN_RIGHT_BRACE);
		case ';': return MakeToken(scanner, TOKEN_SEMICOLON);
		case ',': return MakeToken(scanner, TOKEN_COMMA);
		case '.': return MakeToken(scanner, TOKEN_DOT);
		case '-': return MakeToken(scanner, TOKEN_MINUS);
		case '+': return MakeToken(scanner, TOKEN_PLUS);
		case '/': return MakeToken(scanner, TOKEN_SLASH);
		case '*': return MakeToken(scanner, TOKEN_STAR);
		case '!': return MakeToken(scanner, Match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=': return MakeToken(scanner, Match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<': return MakeToken(scanner, Match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>': return MakeToken(scanner, Match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '"': return String(scanner);
		case '&': {
			if (Match(scanner, '&')) {
				return MakeToken(scanner, TOKEN_AND);
			}
		} break;
		case '|': {
			if (Match(scanner, '|')) {
				return MakeToken(scanner, TOKEN_OR);
			}
		} break;
	}

	return ErrorToken(scanner, "Unexpected character.");
}

Token* ScanTokens(Scanner* scanner) {
	while (!IsAtEnd(scanner)) {
		scanner->start = scanner->current;
		ScanToken(scanner);
	}

}

