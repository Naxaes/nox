#include <stdlib.h>
#include <stdio.h>
#include "generator.h"


Bytecode generate_code(TypedAst ast) {
    u8* instructions = malloc(sizeof(Instruction) * 1024);
    size_t count = 0;

    u8 current_register = 0;

    Node first = ast.nodes[0];
    switch (first.kind) {
        case NodeKind_Invalid: {
            fprintf(stderr, "Unknown node kind: '%d'\n", first.kind);
            free(instructions);
            return (Bytecode) { NULL, 0 };
        } break;
        case NodeKind_Literal: {
            NodeLiteral literal = first.literal;

            instructions[count++] = Instruction_MovImm64;
            instructions[count++] = current_register++;
            instructions[count++] = (literal.value.integer >> 0)  & 0xFF;
            instructions[count++] = (literal.value.integer >> 8)  & 0xFF;
            instructions[count++] = (literal.value.integer >> 16) & 0xFF;
            instructions[count++] = (literal.value.integer >> 24) & 0xFF;
            instructions[count++] = (literal.value.integer >> 32) & 0xFF;
            instructions[count++] = (literal.value.integer >> 40) & 0xFF;
            instructions[count++] = (literal.value.integer >> 48) & 0xFF;
            instructions[count++] = (literal.value.integer >> 56) & 0xFF;
        } break;
    }

    instructions[count++] = Instruction_Exit;
    return (Bytecode) { instructions, count };
}
