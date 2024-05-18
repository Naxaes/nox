#pragma once

#include "code_generator/generator.h"

#include <stdio.h>


void disassemble_instruction(Instruction instruction, Label* labels, size_t label_count, FILE* output);
void disassemble(Bytecode code, FILE* output);
