# Bytecode intepreter from "Crafting Interpreters"
#
# OUTPUT DIRS
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
TEST_DIR=test
ASM_DIR=asm
TEST_BIN_DIR=$(BIN_DIR)/test
PROGRAM_DIR=programs

# Tool options
CC=gcc
OPT=-O0
CFLAGS=-Wall -g2 -std=c99 -D_REENTRANT $(OPT) -fPIC -shared
TESTFLAGS=
LDFLAGS=-pthread
LIBS= 
TEST_LIBS=-lcheck

# style for assembly output
ASM_STYLE=intel

# Object targets
INCS=-I$(SRC_DIR)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.hpp)
# Unit tests 
TEST_SOURCES  = $(wildcard $(TEST_DIR)/*.c)
# Tools (program entry points)
PROGRAM_SOURCES = $(wildcard $(PROGRAM_DIR)/*.c)


# Objects
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c  $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ 

# Objects, but output as assembly
$(ASSEM_OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -S $< -o $(ASM_DIR)/$@.asm -masm=$(ASM_STYLE)


# Unit tests 
TEST_OBJECTS  := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)
$(TEST_OBJECTS): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ 

# ==== TEST TARGETS ==== #
TESTS=test_scanner test_table test_compiler

$(TESTS): $(TEST_OBJECTS) $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o\
		-o $(TEST_BIN_DIR)/$@ $(LIBS) $(TEST_LIBS)


# ==== PROGRAM TARGETS ==== #
PROGRAMS = clox
PROGRAM_OBJECTS := $(PROGRAM_SOURCES:$(PROGRAM_DIR)/%.c=$(OBJ_DIR)/%.o)

$(PROGRAM_OBJECTS): $(OBJ_DIR)/%.o : $(PROGRAM_DIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(PROGRAMS): $(OBJECTS) $(PROGRAM_OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJ_DIR)/$@.o \
		$(INCS) -o $@ $(LIBS)

# Main targets 
#
.PHONY: all test programs clean


all : test programs

test : $(OBJECTS) $(TESTS)

programs : $(PROGRAMS)

assem : $(ASSEM_OBJECTS)

clean:
	@rm -fv *.o $(OBJ_DIR)/*.o 
	# Clean test programs
	@rm -fv $(TEST_BIN_DIR)/test_*

print-%:
	@echo $* = $($*)
