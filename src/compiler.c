#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"


#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif /*DEBUG_PRINT_CODE*/


/*
 * Parser structure
 */
typedef struct {
	Token current;
	Token previous;
	bool had_error;
	bool panic_mode;
} Parser;


/*
 * Precedence 
 */
typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT, 	// =
	PREC_OR,			// or
	PREC_AND,  			// and
	PREC_EQUALITY,      // ==
	PREC_COMPARISON,    // <, >, <=, >=
	PREC_TERM,			// + -
	PREC_FACTOR,		// * /
	PREC_UNARY,			// ! -
	PREC_CALL,			// . ()
	PREC_PRIMARY
} Precedence;


// ParseFn signature takes no arguments and returns nothing.
typedef void (*ParseFn)();


/*
 * Parsing rules
 */
typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;


Parser parser;
Chunk* compiling_chunk;


// Forward declare some functions
//static void expresion(void);
static ParseRule* get_rule(TokenType type);
static void parse_precedence(Precedence prec);


static Chunk* current_chunk(void)
{
	return compiling_chunk;
}


static void error_at(Token* token, const char* msg)
{
	parser.panic_mode = true;

	fprintf(stderr, "[line %d] Error ", token->line);

	if(token->type == TOKEN_EOF)
		fprintf(stderr, "at end");
	else if(token->type == TOKEN_ERROR) {}  // Do nothing
	else
		fprintf(stderr, " at '%.*s'", token->length, token->start);

	fprintf(stderr, ": %s\n", msg);
	parser.had_error = true;
}


static void error(const char* msg)
{
	error_at(&parser.previous, msg);
}

static void error_at_current(const char* msg)
{
	error_at(&parser.current, msg);
}


static void advance(void)
{
	parser.previous = parser.current;

	while(1)
	{
		parser.current = scan_token();
		fprintf(stdout, "[%s]: parser.current: ", __func__);
		print_token(&parser.current);
		if(parser.current.type != TOKEN_ERROR)
			break;

		error_at_current(parser.current.start);
	}
}


static void consume(TokenType type, const char* msg)
{
	if(parser.current.type == type)
	{
		advance();
		return;
	}

	error_at_current(msg);
}


// Emit Bytecodes
static uint8_t make_constant(Value value)
{
	int constant = add_constant(current_chunk(), value);
	if(constant > UINT8_MAX)
	{
		error("Too many constants in one chunk");
		return 0;
	}

	return (uint8_t) constant;
}

static void emit_byte(uint8_t byte)
{
	write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t b1, uint8_t b2)
{
	emit_byte(b1);
	emit_byte(b2);
}

static void emit_return(void)
{
	emit_byte(OP_RETURN);
}

static void emit_constant(Value value)
{
	emit_bytes(OP_CONSTANT, make_constant(value));
}


static void end_compiler(void)
{
	emit_return();
#ifdef DEBUG_PRINT_CODE
	if(!parser.had_error)
		disassemble_chunk(current_chunk(), "code");
#endif /*DEBUG_PRINT_CODE*/
}


/*
 * binary()
 *
 * Parser function for infix expressions. The left operand to
 * the infix operator would have been compiled and placed on 
 * the stack already. Once we reach here we compile the right 
 * operand and emit the corresponding bytecode instruction.
 */
static void binary(void)
{
	// Remember operator
	TokenType op_type = parser.previous.type;

	// Compile right operand 
	ParseRule* rule = get_rule(op_type);
	parse_precedence((Precedence)(rule->precedence + 1));

	// Emit the operator instruction
	switch(op_type)
	{
		case TOKEN_BANG_EQUAL:
			emit_bytes(OP_EQUAL, OP_NOT);
			break;

		case TOKEN_EQUAL_EQUAL:
			emit_byte(OP_EQUAL);
			break;

		case TOKEN_GREATER:
			emit_byte(OP_GREATER);
			break;

		case TOKEN_GREATER_EQUAL:
			emit_bytes(OP_LESS, OP_NOT);
			break;

		case TOKEN_LESS:
			emit_byte(OP_LESS);
			break;

		case TOKEN_LESS_EQUAL:
			emit_bytes(OP_GREATER, OP_NOT);
			break;

		case TOKEN_PLUS:
			emit_byte(OP_ADD);
			break;

		case TOKEN_MINUS:
			emit_byte(OP_SUB);
			break;

		case TOKEN_STAR:
			emit_byte(OP_MUL);
			break;

		case TOKEN_SLASH:
			emit_byte(OP_DIV);
			break;

		default:
			return;		// unreachable
	}
}


static void literal(void)
{
	switch(parser.previous.type)
	{
		case TOKEN_FALSE:
			emit_byte(OP_FALSE);
			break;
		case TOKEN_TRUE:
			emit_byte(OP_TRUE);
			break;
		case TOKEN_NIL:
			emit_byte(OP_NIL);
			break;
		default:
			return;		// unreachabl
	}
}

/*
 * parse_precedence()
 *
 * This is the core of the Pratt parsing algorithm.
 * TODO: write up a description
 */
static void parse_precedence(Precedence prec)
{
	// Parse infix operations 
	advance();

	fprintf(stdout, "[%s] : parser.previous ", __func__);
	print_token(&parser.previous);

	ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
	if(prefix_rule == NULL)
	{
		error("Expect expression");
		return;
	}

	prefix_rule();   // parse this function consuming input

	while(prec <= get_rule(parser.current.type)->precedence)
	{
		advance();
		ParseFn infix_rule = get_rule(parser.previous.type)->infix;
		infix_rule();
	}
}



// Parsing prefix expressions
static void expression(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}



static void grouping(void)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}


static void number(void)
{
	double value = strtod(parser.previous.start, NULL);
	emit_constant(NUMBER_VAL(value));
}


static void string(void)
{
	emit_constant(
			OBJ_VAL(
				copy_string(
					parser.previous.start + 1, parser.
					previous.length - 2
					)
				)
			);
}


static void unary(void)
{
	TokenType op_type = parser.previous.type;

	// Compile this operand
	parse_precedence(PREC_UNARY);

	// Emit the operators instruction
	switch(op_type)
	{
		case TOKEN_BANG:
			emit_byte(OP_NOT);
			break;
		case TOKEN_MINUS:
			emit_byte(OP_NEGATE);
			break;
		default:
			return;			// Unreachable
	}
}


// Parsing Rules
ParseRule rules[] = {
	[TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
	[TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
	[TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
	[TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
	[TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
	[TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
	[TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
	[TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
	[TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
	[TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
	[TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
	[TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FUNC]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
	[TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
	[TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static ParseRule* get_rule(TokenType type)
{
	return &rules[type];
}



bool compile(const char* source, Chunk* chunk)
{
	init_scanner(source);
	
	compiling_chunk = chunk;
	parser.had_error = false;
	parser.panic_mode = false;

	advance();

	expression();
	consume(TOKEN_EOF, "Expect end of expression");
	end_compiler();

	return !parser.had_error;
}
