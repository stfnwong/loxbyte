#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"



typedef struct {
	const char* start;
	const char* current;
	int line;
} Scanner;


// Create a new Scanner global in this translation unit
Scanner scanner;

// Token creation helpers
static Token make_token(TokenType type)
{
	Token token;
	
	token.type = type;
	token.start = scanner.start;
	token.length = (int) (scanner.current - scanner.start);
	token.line = scanner.line;

	return token;
}


static Token error_token(const char* msg)
{
	Token token;
	
	token.type = TOKEN_ERROR;
	token.start = msg;
	token.length = (int) strlen(msg);
	token.line = scanner.line;

	return token;
}


// ======== Scanner methods ======== //

static bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

static bool is_alpha(char c)
{
	return (c >= 'A' && c <= 'Z') || 
		   (c >= 'a' && c <= 'z') ||
		   c == '_';
}

static bool is_at_end(void)
{
	return *scanner.current == '\0';
}

/*
 * advance()
 * Advance the character pointer, consuming the current character
 */
static char advance(void)
{
	scanner.current++;
	return scanner.current[-1];
}


/*
 * peek()
 * Look at the current character without consuming.
 */
static char peek(void)
{
	return *scanner.current;
}


/*
 * peek_next()
 * Look at the character after the current character without consuming.
 */
static char peek_next(void)
{
	if(is_at_end())
		return '\0';
	
	return scanner.current[1];
}

/*
 * match()
 * Match a single character against the current character, consuming if its a match.
 * If the current character matches, advance the current character and return true.
 * If the current character does not match, don't advance and return false.
 */
static bool match(char expected)
{
	if(is_at_end())
		return false;

	if(*scanner.current != expected)
		return false;

	scanner.current++; // TODO: call advance and discard return?
	
	return true;
}


/*
 * skip_whitespace()
 */
static void skip_whitespace(void)
{
	while(1)
	{
		char c = peek();
		switch(c)
		{
			case ' ':
			case '\r':
			case '\t': {
				advance();
				break;
		    }
			case '\n': {
				scanner.line++;
				advance();
				break;
		    }
			case '/': {
				if(peek_next() == '\\') {
					while(peek() != '\n' && !is_at_end())
						advance();
				}
				else
					return;
				break;
			}
			default:
				return;
		}
	}
}



/*
 * string()
 * Create a string token
 */
static Token string(void)
{
	while(peek() != '"' && !is_at_end())
	{
		if(peek() == '\n')
			scanner.line++;
		advance();
	}

	if(is_at_end())
		return error_token("Unterminated string");

	// Consume closing quote
	advance();
	
	return make_token(TOKEN_STRING);
}

/*
 * number()
 * Create a numeric token
 */
static Token number(void)
{
	while(is_digit(peek()))
		advance();

	// Look for a fractional part
	if(peek() == '.' && is_digit(peek_next()))
	{
		advance();	// consume '.'
		while(is_digit(peek()))
			advance();
	}
	
	return make_token(TOKEN_NUMBER);
}


/*
 * check_keyword()
 * Implements keyword checking DFA
 */
static TokenType check_keyword(int start, int length, const char* rest, TokenType type)
{
	if((scanner.current - scanner.start == start + length) && memcmp(scanner.start + start, rest, length) == 0)
		return type;

	return TOKEN_IDENTIFIER;
}


/*
 * identifier_type()
 * Resolve the appropriate type for a given identifier. 
 * In particular we wan to resolve keywords to their specialized token types.
 */
static TokenType identifier_type(void)
{
	switch(scanner.start[0])
	{
		case 'a': return check_keyword(1, 2, "nd",   TOKEN_AND);
		case 'c': return check_keyword(1, 4, "lass", TOKEN_CLASS);
		case 'e': return check_keyword(1, 3, "lse",  TOKEN_ELSE);
		case 'f': {
			switch(scanner.start[1])
			{
				case 'a':
					return check_keyword(2, 3, "lse", TOKEN_FALSE);
				case 'o':
					return check_keyword(2, 1, "r",   TOKEN_FOR);
				case 'u':
					return check_keyword(2, 2, "nc",  TOKEN_FUNC);
			}
			break;
		}
		case 'i': return check_keyword(1, 2, "f",     TOKEN_IF);
		case 'n': return check_keyword(1, 2, "il",    TOKEN_IF);
		case 'o': return check_keyword(1, 2, "r",     TOKEN_OR);
		case 'p': return check_keyword(1, 4, "rint",  TOKEN_PRINT);
		case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
		case 's': return check_keyword(1, 4, "uper",  TOKEN_SUPER);
		case 't': {
			if(scanner.current - scanner.start > 1)
			{
				switch(scanner.start[1])
				{
					case 'h':
						return check_keyword(2, 2, "is", TOKEN_THIS);
					case 'r':
						return check_keyword(2, 2, "ue", TOKEN_TRUE);
				}
			}
			break;
		}
		case 'v': return check_keyword(1, 2, "ar",   TOKEN_VAR);
		case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
	}

	return TOKEN_IDENTIFIER;
}

/*
 * identifier()
 * Return an indentifier token
 */
static Token identifier(void)
{
	while(is_alpha(peek()) || is_digit(peek()))
		advance();

	return make_token(identifier_type());
}


void init_scanner(const char* source)
{
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}


Token scan_token(void)
{
	skip_whitespace();

	scanner.start = scanner.current;
	if(is_at_end())
		return make_token(TOKEN_EOF);

	char c = advance();

	if(is_alpha(c))
		return identifier();

	if(is_digit(c))
		return number();

	switch(c)
	{
		// Single character tokens 
		case ',': return make_token(TOKEN_COMMA);
		case '.': return make_token(TOKEN_DOT);
		case '{': return make_token(TOKEN_LEFT_BRACE);
		case '(': return make_token(TOKEN_LEFT_PAREN);
		case '-': return make_token(TOKEN_MINUS);
		case '+': return make_token(TOKEN_PLUS);
		case '}': return make_token(TOKEN_RIGHT_BRACE);
		case ')': return make_token(TOKEN_RIGHT_PAREN);
		case ';': return make_token(TOKEN_SEMICOLON);
		case '/': return make_token(TOKEN_SLASH);
		case '*': return make_token(TOKEN_STAR);

		// One or two character tokens
		case '!': {
			return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	  	}
		case '=': {
			return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		}
		case '<': {
			return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		}
		case '>': {
			return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		}
		case '"': return string();
	}

	return error_token("Unexpected character.");
}



void print_token(Token* token)
{
	fprintf(stdout, "Token(%02d) '%.*s'\n", token->type, token->length, token->start);
}

