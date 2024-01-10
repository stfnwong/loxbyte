#include <assert.h>
#include <stdio.h>

#include "scanner.h"



void temp_scanner(void)
{
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




int main(int argc, char *argv[])
{
	return 0;
}
