#include <stdio.h>

#include "compiler.h"
#include "scanner.h"



void compile(const char* source)
{
	init_scanner(source);
	
	int line = -1;

	while(1)
	{
		Token token = scan_token();
		if(token.line != line)
		{
			fprintf(stdout, "%4d ", token.line);
			line = token.line;
		}
		else
			fprintf(stdout, "  |   ");

		fprintf(stdout, "%2d '%.*s'\n", token.type, token.length, token.start);

		if(token.type == TOKEN_EOF)
			break;
	}
}
