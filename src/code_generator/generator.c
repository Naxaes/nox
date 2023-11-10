#include "generator.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct {
    u8* instructions;
    size_t count;
    u8  current_register;
} Generator;

Register generate_for_expression(Generator* generator, TypedAst ast, Node* node);
void generate_for_statement(Generator* generator, TypedAst ast, Node* node);


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

Register bin_op(Generator* generator, Instruction binary_instruction, Register dst, Register src) {
    generator->instructions[generator->count++] = binary_instruction;
    generator->instructions[generator->count++] = dst;
    generator->instructions[generator->count++] = src;
    return dst;
}

Register store(Generator* generator, Register dest, Register source) {
    generator->instructions[generator->count++] = Instruction_Store;
    generator->instructions[generator->count++] = dest;
    generator->instructions[generator->count++] = source;
    return dest;
}

Register generate_for_literal(Generator* generator, TypedAst ast, NodeLiteral literal) {
    switch (literal.type) {
        case Literal_Boolean:
            return mov_imm64(generator, generator->current_register++, literal.value.integer != 0);
        case Literal_Integer:
            return mov_imm64(generator, generator->current_register++, literal.value.integer);
        case Literal_Real:
            return mov_imm64(generator, generator->current_register++, literal.value.real);
        default:
            assert(0 && "Invalid literal type");
            return 0;
    }
}

Register generate_for_identifier(Generator* generator, TypedAst ast, NodeIdentifier identifier) {
    Block* current = ast.blocks;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl.kind == NodeKind_VarDecl && "Invalid node kind");
        if (local->decl.var_decl.name == identifier.name) {
            generator->instructions[generator->count++] = Instruction_Load;
            generator->instructions[generator->count++] = generator->current_register;
            generator->instructions[generator->count++] = i;
            return generator->current_register++;
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", identifier.name);
    return 0;
}

Register generate_for_binary(Generator* generator, TypedAst ast, NodeBinary binary) {
    Register dest   = generate_for_expression(generator, ast, binary.left);
    Register source = generate_for_expression(generator, ast, binary.right);
    generator->current_register--;  // Consume the expression register
    static Instruction binary_instructions[] = {
            [Binary_Operation_Add] = Instruction_Add,
            [Binary_Operation_Sub] = Instruction_Sub,
            [Binary_Operation_Mul] = Instruction_Mul,
            [Binary_Operation_Div] = Instruction_Div,
            [Binary_Operation_Mod] = Instruction_Mod,
            [Binary_Operation_Lt]  = Instruction_Lt,
            [Binary_Operation_Le]  = Instruction_Le,
            [Binary_Operation_Eq]  = Instruction_Eq,
            [Binary_Operation_Ne]  = Instruction_Ne,
            [Binary_Operation_Ge]  = Instruction_Ge,
            [Binary_Operation_Gt]  = Instruction_Gt,
    };
    assert(binary.op <= Binary_Operation_Gt && "Invalid binary operation");
    Instruction inst = binary_instructions[binary.op];
    assert(inst != Instruction_Invalid && "Invalid binary operation");
    return bin_op(generator, inst, dest, source);
}

Register generate_for_expression(Generator* generator, TypedAst ast, Node* node) {
    assert(node_is_expression(node) && "Invalid node kind");
    switch (node->kind) {
        case NodeKind_Literal:
            return generate_for_literal(generator, ast, node->literal);
        case NodeKind_Identifier:
            return generate_for_identifier(generator, ast, node->identifier);
        case NodeKind_Binary:
            return generate_for_binary(generator, ast, node->binary);
        default:
            assert(0 && "not implemented");
            return 0;
    }
}

Register generate_for_assign(Generator* generator, TypedAst ast, NodeAssign assign) {
    Register src = generate_for_expression(generator, ast, assign.expression);
    generator->current_register--;  // Consume the expression register

    Block* current = ast.blocks;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl.kind == NodeKind_VarDecl && "Invalid node kind");
        if (local->decl.var_decl.name == assign.name) {
            generator->instructions[generator->count++] = Instruction_Store;
            generator->instructions[generator->count++] = i;
            generator->instructions[generator->count++] = src;
            return generator->current_register++;
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", assign.name);
    return 0;
}

void generate_for_var_decl(Generator* generator, TypedAst ast, NodeVarDecl var_decl) {
    Register dest = generator->current_register++;
    Register src  = generate_for_expression(generator, ast, var_decl.expression);
    generator->current_register--;  // Consume the expression register
    store(generator, dest, src);
}

void generate_for_if_stmt(Generator* generator, TypedAst ast, NodeIf if_stmt) {
    /*
     * if condition == 0 goto else
     *    statements
     * jmp end
     * else:
     *    statements
     * end:
     */

    Register condition = generate_for_expression(generator, ast, if_stmt.condition);
    generator->current_register--;  // Consume the expression register

    generator->instructions[generator->count++] = Instruction_JmpZero;
    generator->instructions[generator->count++] = condition;
    u8* jump_to_else = generator->instructions + generator->count;
    generator->instructions[generator->count++] = Instruction_Invalid;

    generate_for_statement(generator, ast, (Node*) if_stmt.then_block);

    if (if_stmt.else_block != NULL) {
        generator->instructions[generator->count++] = Instruction_Jmp;
        u8* jump_to_end = generator->instructions + generator->count;
        generator->instructions[generator->count++] = Instruction_Invalid;

        *jump_to_else = (u8) generator->count;

        generate_for_statement(generator, ast, (Node*) if_stmt.else_block);
        *jump_to_end = (u8) generator->count;
    } else {
        *jump_to_else = (u8) generator->count;
    }

}

void generate_for_statement(Generator* generator, TypedAst ast, Node* node) {
    assert(node_is_statement(node) && "Invalid node kind");
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary: {
            generate_for_expression(generator, ast, node);
            generator->current_register--;  // Consume the expression register
        } break;
        case NodeKind_Assign:
            generate_for_assign(generator, ast, node->assign);
            break;
        case NodeKind_VarDecl:
            generate_for_var_decl(generator, ast, node->var_decl);
            break;
        case NodeKind_If: {
            generate_for_if_stmt(generator, ast, node->if_stmt);
            break;
        }
        case NodeKind_Block: {
            NodeBlock block = node->block;
            while ((node = *block.nodes++) != NULL) {
                generate_for_statement(generator, ast, node);
            }
            break;
        }
        default:
            assert(0 && "not implemented");
            break;
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
    return (Bytecode) { generator.instructions, generator.count };
}
