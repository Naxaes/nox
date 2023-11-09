#include "generator.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct {
    u8* instructions;
    size_t count;
    u8  current_register;
} Generator;


Register mov_imm64(Generator* generator, u8 dest, u64 immediate) {
    generator->instructions[generator->count++] = Instruction_MovImm64;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = (immediate >> 0) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 8) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 16) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 24) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 32) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 40) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 48) & 0xFF;
    generator->instructions[generator->count++] = (immediate >> 56) & 0xFF;
    return dest;
}

Register mov_reg(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Mov;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    return dest;
}

Register add(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Add;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    return dest;
}

Register mul(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Mul;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    return dest;
}

Register store(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Store;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    return dest;
}


Register generate_for_expression(Generator* generator, TypedAst ast, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
        case NodeKind_VarDecl:
        case NodeKind_Block:
            assert(0 && "Invalid node kind");
            return 0;
        case NodeKind_Literal:
            return mov_imm64(generator, generator->current_register++, node->literal.value.integer);
        case NodeKind_Identifier: {
            Block* current = ast.blocks;
            for (size_t i = 0; i < current->count; ++i) {
                Local* local = current->locals + i;
                assert(local->decl.kind == NodeKind_VarDecl && "Invalid node kind");
                if (local->decl.var_decl.name == node->identifier.name) {
                    generator->instructions[generator->count++] = Instruction_Load;
                    generator->instructions[generator->count++] = generator->current_register;
                    generator->instructions[generator->count++] = i;
                    return generator->current_register++;
                }
            }
            fprintf(stderr, "Unknown identifier: '%s'\n", node->identifier.name);
            return 0;
        } break;
        case NodeKind_Binary: {
            Register dest   = generate_for_expression(generator, ast, node->binary.left);
            Register source = generate_for_expression(generator, ast, node->binary.right);
            generator->current_register--;  // Consume the expression register
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

void generate_for_statement(Generator* generator, TypedAst ast, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
            assert(0 && "Invalid node kind");
            break;
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary: {
            generate_for_expression(generator, ast, node);
            generator->current_register--;  // Consume the expression register
        } break;
        case NodeKind_VarDecl: {
            Register dest = generator->current_register++;
            Register src = generate_for_expression(generator, ast, node->var_decl.expression);
            generator->current_register--;  // Consume the expression register
            store(generator, dest, src);
        } break;
        case NodeKind_Block: {
            NodeBlock block = node->block;
            while ((node = *block.nodes++) != NULL) {
                generate_for_statement(generator, ast, node);
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

    Node* node = ast.start;
    generate_for_statement(&generator, ast, node);

    if (generator.current_register > 0) {
        mov_reg(&generator, 0, generator.current_register);
    }
    generator.instructions[generator.count++] = Instruction_Exit;

    for (size_t i = 0; i < generator.count; ++i) {
        printf("%d ", generator.instructions[i]);
    }
    printf("\n");

    return (Bytecode) { generator.instructions, generator.count };
}
