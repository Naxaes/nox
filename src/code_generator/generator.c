#include "generator.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct {
    u8* instructions;
    size_t count;
    u8  current_register;
} Generator;


Register mov_imm64(Generator* generator, NodeLiteral literal) {
    generator->instructions[generator->count++] = Instruction_MovImm64;
    generator->instructions[generator->count++] = generator->current_register;
    generator->instructions[generator->count++] = (literal.value.integer >> 0)  & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 8)  & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 16) & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 24) & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 32) & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 40) & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 48) & 0xFF;
    generator->instructions[generator->count++] = (literal.value.integer >> 56) & 0xFF;
    return generator->current_register++;
}

Register add(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Add;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    generator->current_register--; // We don't need the source register anymore
    return dest;
}

Register mul(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Mul;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    generator->current_register--; // We don't need the source register anymore
    return dest;
}


Register generate_for_expression(Generator* generator, TypedAst ast, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
            assert(0 && "Invalid node kind");
        case NodeKind_Literal:
            return mov_imm64(generator, node->literal);
        case NodeKind_Binary: {
            Register dest   = generate_for_expression(generator, ast, node->binary.left);
            Register source = generate_for_expression(generator, ast, node->binary.right);
            switch (node->binary.op) {
                case '+': {
                    return add(generator, dest, source);
                } break;
                case '*': {
                    return mul(generator, dest, source);
                } break;
                default: {
                    fprintf(stderr, "Unknown binary operator: '%c'\n", node->binary.op);
                    assert(0 && "Invalid node kind");
                    return 0;
                } break;
            }
        } break;
    }
}


Bytecode generate_code(TypedAst ast) {
    Generator generator = {
        .instructions = malloc(sizeof(Instruction) * 1024),
        .count = 0,
        .current_register = 0,
    };

    Node* first = ast.start;
    switch (first->kind) {
        case NodeKind_Invalid: {
            fprintf(stderr, "Unknown node kind: '%d'\n", first->kind);
            free(generator.instructions);
            return (Bytecode) { NULL, 0 };
        } break;
        case NodeKind_Literal:
        case NodeKind_Binary: {
            generate_for_expression(&generator, ast, first);
        } break;
    }

    generator.instructions[generator.count++] = Instruction_Exit;
    return (Bytecode) { generator.instructions, generator.count };
}
