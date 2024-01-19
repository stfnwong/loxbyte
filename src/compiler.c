#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	bool verbose;
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
typedef void (*ParseFn)(bool can_assign);


/*
 * Parsing rules
 */
typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

/*
 * Local variable
 */
typedef struct {
	Token name;
	int depth;
} Local;


/*
 * Compiler
 */
typedef struct {
	Local locals[UINT8_COUNT];
	int local_count;
	int scope_depth;
} Compiler;

Parser parser;
Chunk* compiling_chunk;
Compiler* current_compiler = NULL;


// Forward declare some functions
//static void expresion(void);
static ParseRule* get_rule(TokenType type);
static void parse_precedence(Precedence prec);

// Forward declare production functions 
static void statement(void);
static void declaration(void);



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


/*
 * advance()
 * Move the compiler forward by one token, consuming the 
 * token in the process.
 */
static void advance(void)
{
	parser.previous = parser.current;

	while(1)
	{
		parser.current = scan_token();

		if(parser.verbose)
		{
			fprintf(stdout, "[%s]: parser.current: ", __func__);
			print_token(&parser.current);
		}

		if(parser.current.type != TOKEN_ERROR)
			break;

		error_at_current(parser.current.start);
	}
}


/*
 * consume()
 */
static void consume(TokenType type, const char* msg)
{
	if(parser.current.type == type)
	{
		advance();
		return;
	}

	error_at_current(msg);
}


/*
 * check()
 */
static bool check(TokenType type)
{
	return parser.current.type == type;
}


/*
 * match()
 */
static bool match(TokenType type)
{
	if(!check(type))
		return false;
	advance();

	return true;
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


/*
 * init_compiler()
 */
static void init_compiler(Compiler* compiler)
{
	compiler->local_count = 0;
	compiler->scope_depth = 0;
	current_compiler = compiler;
}


/*
 * end_compiler()
 */
static void end_compiler(void)
{
	emit_return();
	// TODO: put this behind verbose switch?
#ifdef DEBUG_PRINT_CODE
	if(!parser.had_error)
		disassemble_chunk(current_chunk(), "code");
#endif /*DEBUG_PRINT_CODE*/
}


/* 
 * begin_scope()
 */
static void begin_scope(void)
{
	current_compiler->scope_depth++;
}

/*
 * end_scope()
 */
static void end_scope(void)
{
	current_compiler->scope_depth--;

	// Pop locals off the stack frame 	
	while(current_compiler->local_count > 0 && 
		  (current_compiler->locals[current_compiler->local_count-1].depth > current_compiler->scope_depth))

	{
		emit_byte(OP_POP);
		current_compiler->local_count--;
	}
}


/*
 * binary()
 *
 * Parser function for infix expressions. The left operand to
 * the infix operator would have been compiled and placed on 
 * the stack already. Once we reach here we compile the right 
 * operand and emit the corresponding bytecode instruction.
 */
static void binary(bool can_assign)
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


static void literal(bool can_assign)
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

	if(parser.verbose)
	{
		fprintf(stdout, "[%s] : parser.previous ", __func__);
		print_token(&parser.previous);
	}

	ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
	if(prefix_rule == NULL)
	{
		error("Expect expression");
		return;
	}

	bool can_assign = (prec <= PREC_ASSIGNMENT);
	prefix_rule(can_assign);   // parse this function consuming input

	while(prec <= get_rule(parser.current.type)->precedence)
	{
		advance();
		ParseFn infix_rule = get_rule(parser.previous.type)->infix;
		infix_rule(can_assign);
	}

	if(can_assign && match(TOKEN_EQUAL))
		error("Invalid assignment target.");
}


/*
 * identifier_constant()
 */
static uint8_t identifier_constant(Token* name)
{
	return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}



/*
 * add_local()
 */
static void add_local(Token name)
{
	if(current_compiler->local_count >= UINT8_COUNT)
	{
		error("Too many local variables in function");
		return;
	}

	Local* local = &current_compiler->locals[current_compiler->local_count];
	local->name = name;
	local->depth = -1; 	// mark as uninitialized
}



/*
 * identifiers_equal()
 */
static bool identifiers_equal(Token* a, Token* b)
{
	if(a->length != b->length)
		return false;

	return memcmp(a->start, b->start, a->length) == 0;
}


/*
 * resolve_local()
 */
static int resolve_local(Compiler* compiler, Token* name)
{
	for(int i = compiler->local_count-1; i > 0; --i)
	{
		Local* local = &compiler->locals[i];
		if(identifiers_equal(name, &local->name))
		{
			if(local->depth == -1)
				error("Can't read local variable in its own initializer");
			return i;
		}
	}

	return -1;
}

/*
 * delcare_variable()
 */
static void declare_variable(void)
{
	// Global variables are implicitly declared
	if(current_compiler->scope_depth == 0)
		return;

	Token* name = &parser.previous;
	
	// In Lox it is an error to have the same name defined 
	// twice in the same scope. Note that this is not shadowing
	// (where we have the same name in different scopes). 
	// We check that variables are not re-defined here by
	// checking all the variables in the current scope.
	
	for(int i = current_compiler->local_count; i > 0; --i)
	{
		Local* local = &current_compiler->locals[i];
		if(local->depth != -1 && local->depth < current_compiler->scope_depth)
			break;

		if(identifiers_equal(name, &local->name))
			error("Already a variable with that name in scope");
	}

	add_local(*name);
}

/*
 * parse_variable()
 */
static uint8_t parse_variable(const char* err_msg)
{
	consume(TOKEN_IDENTIFIER, err_msg);

	declare_variable();
	if(current_compiler->scope_depth > 0)
		return 0;
	
	return identifier_constant(&parser.previous);
}


// Parsing prefix expressions
static void expression(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}

/*
 * block()
 */
static void block(void)
{
	while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
		declaration();

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/*
 * expression_statement()
 */
static void expression_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression");
	emit_byte(OP_POP);
}


/*
 * print_statement()
 */
static void print_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value");
	emit_byte(OP_PRINT);
}

/*
 * mark_initialized()
 */
static void mark_initialized(void)
{
	// A local varsiable with non-zero depth means that we 
	// have seen and compiled that variables initializer.
	current_compiler->locals[current_compiler->local_count-1].depth = current_compiler->scope_depth;
}


/*
 * define_variable()
 */
static void define_variable(uint8_t global)
{
	// Don't define locals here
	if(current_compiler->scope_depth > 0)
	{
		mark_initialized();
		return;
	}

	emit_bytes(OP_DEFINE_GLOBAL, global);
}


/*
 * var_decl()
 */
static void var_decl(void)
{
	uint8_t global = parse_variable("Expect variable name");

	if(match(TOKEN_EQUAL))
		expression();
	else
		emit_byte(OP_NIL);

	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

	define_variable(global);
}

/*
 * statement()
 * Parse a single statement
 */
static void statement(void)
{
	if(match(TOKEN_PRINT))
		print_statement();
	else if(match(TOKEN_LEFT_BRACE))
	{
		begin_scope();
		block();
		end_scope();
	}
	else
		expression_statement();
}

/*
 * synchronise()
 */
static void synchronise(void)
{
	parser.panic_mode = false;

	while(parser.current.type != TOKEN_EOF)
	{
		// Skip tokens until we reach something that looks like
		// a statement boundary.
		if(parser.previous.type == TOKEN_SEMICOLON)
			return;

		switch(parser.current.type)
		{
			case TOKEN_CLASS:
			case TOKEN_FUNC:
			case TOKEN_VAR:
			case TOKEN_FOR:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
				break;

			default:
				;		// do nothing
		}

		advance();
	}
}

/*
 * declaration()
 */
static void declaration(void)
{
	if(match(TOKEN_VAR))
		var_decl();
	else
		statement();
	
	if(parser.panic_mode)
		synchronise();
}


/*
 * grouping()
 */
static void grouping(bool can_assign)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}



/*
 * number()
 */
static void number(bool can_assign)
{
	double value = strtod(parser.previous.start, NULL);
	emit_constant(NUMBER_VAL(value));
}


/*
 * string()
 * Parse a string token
 */
static void string(bool can_assign)
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


/*
 * named_variable()
 */
static void named_variable(Token name, bool can_assign)
{
	uint8_t get_op, set_op;
	uint8_t arg = resolve_local(current_compiler, &name);

	if(arg != -1)
	{
		get_op = OP_GET_LOCAL;
		set_op = OP_SET_LOCAL;
	}
	else
	{
		get_op = OP_GET_GLOBAL;
		set_op = OP_SET_GLOBAL;
	}

	if(can_assign && match(TOKEN_EQUAL))
	{
		expression();
		emit_bytes(set_op, (uint8_t) arg);
	}
	else
		emit_bytes(get_op, (uint8_t) arg);
}


/*
 * variable()
 */
static void variable(bool can_assign)
{
	named_variable(parser.previous, can_assign);
}


static void unary(bool can_assign)
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
	[TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
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
	Compiler compiler;
	init_compiler(&compiler);
	compiling_chunk = chunk;

	parser.had_error = false;
	parser.panic_mode = false;
	parser.verbose = true;		// TODO: make this settable from shell

	advance();

	while(!match(TOKEN_EOF))
		declaration();

	end_compiler();

	return !parser.had_error;
}
