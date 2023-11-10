#pragma once

#include "code_generator/generator.h"

#include <stdio.h>


void disassemble_instruction(Instruction instruction, FILE* output);
void disassemble(Bytecode code, FILE* output);
