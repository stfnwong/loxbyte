#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"


static char* read_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file [%s]\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	char* buffer = malloc(file_size + 1);
	if(buffer == NULL) {
		fprintf(stderr, "Not enough memory for buffer of (%ld bytes)", file_size);
		exit(74);
	}

	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
	if(bytes_read < file_size) {
		fprintf(stderr, "Failed to read file [%s] (read %ld bytes, expected %ld bytes)\n", path, bytes_read, file_size);
		exit(74);
	}

	buffer[bytes_read] = '\0';

	fclose(file);
	
	return buffer;
}


static void repl(void)
{
	char line[1024];

	while(1)
	{
		fprintf(stdout, "> ");

		if(!fgets(line, sizeof(line), stdin))
		{
			fprintf(stdout, "\n");
			break;
		}

		interpret(line);
	}
}


static void run_file(const char* path)
{
	char* source = read_file(path);
	InterpResult result = interpret(source);
	free(source);

	if(result == INTERPRET_COMPILE_ERROR)
		exit(65);
	if(result == INTERPRET_RUNTIME_ERROR)
		exit(70);
}





int main(int argc, char *argv[])
{
	init_vm();

	if(argc == 1) {
		repl();
	}
	else if(argc == 2) {
		run_file(argv[1]);
	}
	else 
		fprintf(stderr, "Usage: clox: [path]\n");

	free_vm();

	return 0;
}
