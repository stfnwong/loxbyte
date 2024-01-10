#ifndef __LOX_DEBUG_H
#define __LOX_DEBUG_H

#include "chunk.h"
#include "value.h"


void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instr(Chunk* chunk, int offset);


#endif /*__LOX_DEBUG_H*/
