#ifndef __DEBUG_H
#define __DEBUG_H

#include "chunk.h"
#include "value.h"


void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instr(Chunk* chunk, int offset);


#endif /*__DEBUG_H*/
