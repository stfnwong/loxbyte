/*
 * LOX SCANNER
 */

#ifndef __LOX_SCANNER_H
#define __LOX_SCANNER_H


// All valid Lox tokens
typedef enum {
	// Single character tokens
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_LEFT_BRACE,
	TOKEN_LEFT_PAREN,
	TOKEN_MINUS,
	TOKEN_PLUS,
	TOKEN_RIGHT_BRACE,
	TOKEN_RIGHT_PAREN,
	TOKEN_SEMICOLON,
	TOKEN_SLASH,
	TOKEN_STAR,

	// One or two character tokens
	TOKEN_BANG,
	TOKEN_BANG_EQUAL,
	TOKEN_EQUAL,
	TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
	TOKEN_LESS,
	TOKEN_LESS_EQUAL,

	// Literals
	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_NUMBER,

	// Keywords
	TOKEN_AND,
	TOKEN_CLASS,
	TOKEN_ELSE,
	TOKEN_FALSE,
	TOKEN_FOR,
	TOKEN_FUNC,
	TOKEN_IF,
	TOKEN_NIL,
	TOKEN_OR,
	TOKEN_PRINT,
	TOKEN_RETURN,
	TOKEN_SUPER,
	TOKEN_THIS,
	TOKEN_TRUE,
	TOKEN_VAR,
	TOKEN_WHILE,

	TOKEN_ERROR,
	TOKEN_EOF

} TokenType;


/*
 * Token
 */
typedef struct {
	TokenType type;
	const char* start;
	int length;
	int line;
} Token;


void init_scanner(const char* source);
Token scan_token(void);


void print_token(Token* token);


// Human-readable string representation of TokenType
// These need to be in the same order as the TokenType enum
static const char* token_string[] = {
	",",		// TOKEN_COMMA
	".",		// TOKEN_DOT
	"{",		// TOKEN_LEFT_BRACE
	"(",		// TOKEN_LEFT_PAREN
	"-",		// TOKEN_MINUS
	"+",		// TOKEN_PLUS
	"}",		// TOKEN_RIGHT_BRACE
	")",		// TOKEN_RIGHT_PAREN
	";",		// TOKEN_SEMICOLON
	"/",		// TOKEN_SLASH
	"*",		// TOKEN_STAR
	"!",		// TOKEN_BANG
	"!=",		// TOKEN_BANG_EQUAL
	"=",		// TOKEN_EQUAL
	"==",		// TOKEN_EQUAL_EQUAL
	">",		// TOKEN_GREATER
	">=",		// TOKEN_GREATER_EQUAL
	"<",		// TOKEN_LESS
	"<=",		// TOKEN_LESS_EQUAL
	"IDEN",		// TOKEN_IDENTIFIER
	"STR",		// TOKEN_STRING
	"NUM",		// TOKEN_NUMBER
	"AND",		// TOKEN_AND
	"CLASS",	// TOKEN_CLASS
	"ELSE",	    // TOKEN_ELSE
	"FALSE",	// TOKEN_FALSE
	"FOR",		// TOKEN_FOR
	"FUNC",		// TOKEN_FUNC
	"IF",		// TOKEN_IF
	"NIL",		// TOKEN_NIL
	"OR",		// TOKEN_OR
	"PRINT",	// TOKEN_PRINT
	"RETURN",	// TOKEN_RETURN
	"SUPER",	// TOKEN_SUPER
	"THIS",		// TOKEN_THIS
	"TRUE",		// TOKEN_TRUE
	"VAR",		// TOKEN_VAR
	"WHILE", 	// TOKEN_WHILE
	"ERROR", 	// TOKEN_ERROR
	"EOF", 		// TOKEN_EOF
};


#endif /*__LOX_SCANNER_H*/
