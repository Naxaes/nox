#include "generator.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct {
    const char* name;
    Node* node;
    Instruction** references;
    size_t references_count;
} DeferredBlock;

typedef struct {
    Instruction* instructions;
    size_t       count;
    Register     current_register;

    DeferredBlock* deferred_blocks;
    size_t         deferred_blocks_count;
} Generator;

Register mov_imm64(Generator* generator, Register dst, u64 value) {
    generator->instructions[generator->count++] = (Instruction) {
        .type = Instruction_MovImm64,
        .imm.dst = dst,
        .imm.val = value
    };
    return dst;
}

Register mov_reg(Generator* generator, Register dst, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
        .type = Instruction_Mov,
        .reg.dst = dst,
        .reg.src = src,
    };
    return dst;
}

Register bin_op(Generator* generator, InstructionType binary_op, Register dst, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
        .type = binary_op,
        .reg.dst = dst,
        .reg.src = src,
    };
    return dst;
}

Register store(Generator* generator, Register dst, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Store,
            .reg.dst = dst,
            .reg.src = src,
    };
    return dst;
}

Register load(Generator* generator, Register dst, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Load,
            .reg.dst = dst,
            .reg.src = src,
    };
    return dst;
}

// NOTE(ted): This must be patched!
Instruction* jmp_zero(Generator* generator, Register src) {
    generator->instructions[generator->count] = (Instruction) {
            .type = Instruction_JmpZero,
            .jmp.label = 0xdeadbeef,
            .jmp.src   = src,
    };
    return generator->instructions + generator->count++;
}

// NOTE(ted): This must be patched!
Instruction* jmp(Generator* generator) {
    generator->instructions[generator->count] = (Instruction) {
            .type = Instruction_Jmp,
            .jmp.label = 0xdeadbeef,
    };
    return generator->instructions + generator->count++;
}


/* ---------------------------- GENERATOR VISITOR -------------------------------- */
Register generate_for_expression(Generator* generator, TypedAst ast, Node* node);
void generate_for_statement(Generator* generator, TypedAst ast, Node* node);
Register generate_for_call(Generator* generator, TypedAst ast, NodeCall call);

Register generate_for_literal(Generator* generator, TypedAst ast, NodeLiteral literal) {
    switch (literal.type) {
        case LiteralType_Boolean:
            return mov_imm64(generator, generator->current_register++, literal.value.integer != 0);
        case LiteralType_Integer:
            return mov_imm64(generator, generator->current_register++, literal.value.integer);
        case LiteralType_Real:
            assert(0 && "not implemented");
        case LiteralType_String:
            return mov_imm64(generator, generator->current_register++, (u64) literal.value.string);
        default:
            assert(0 && "Invalid literal type");
    }
}

Register generate_for_identifier(Generator* generator, TypedAst ast, NodeIdentifier identifier) {
    Block* current = ast.block;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl && "Invalid node kind");
        if (local->decl->var_decl.name == identifier.name) {
            load(generator, generator->current_register, i);
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
    static InstructionType binary_op[] = {
            [BinaryOp_Add] = Instruction_Add,
            [BinaryOp_Sub] = Instruction_Sub,
            [BinaryOp_Mul] = Instruction_Mul,
            [BinaryOp_Div] = Instruction_Div,
            [BinaryOp_Mod] = Instruction_Mod,
            [BinaryOp_Lt]  = Instruction_Lt,
            [BinaryOp_Le]  = Instruction_Le,
            [BinaryOp_Eq]  = Instruction_Eq,
            [BinaryOp_Ne]  = Instruction_Ne,
            [BinaryOp_Ge]  = Instruction_Ge,
            [BinaryOp_Gt]  = Instruction_Gt,
    };
    assert(binary.op <= BinaryOp_Gt && "Invalid binary operation");
    InstructionType inst = binary_op[binary.op];
    assert(inst != 0 && "Invalid binary operation");
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
        case NodeKind_Call:
            return generate_for_call(generator, ast, node->call);
        default:
            assert(0 && "not implemented");
    }
}

Register generate_for_assign(Generator* generator, TypedAst ast, NodeAssign assign) {
    Register src = generate_for_expression(generator, ast, assign.expression);
    generator->current_register--;  // Consume the expression register

    Block* current = ast.block;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl->kind == NodeKind_VarDecl && "Invalid node kind");
        if (local->decl->var_decl.name == assign.name) {
            store(generator, i, src);
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

    Instruction* jump_to_else = jmp_zero(generator, condition);
    generate_for_statement(generator, ast, (Node*) if_stmt.then_block);

    if (if_stmt.else_block != NULL) {
        Instruction* jump_to_end = jmp(generator);
        jump_to_else->jmp.label = generator->count;

        generate_for_statement(generator, ast, (Node*) if_stmt.else_block);
        jump_to_end->jmp.label = generator->count;
    } else {
        jump_to_else->jmp.label = generator->count;
    }
}

void generate_for_while_stmt(Generator* generator, TypedAst ast, NodeWhile while_stmt) {
    /*
     * start:
     * if condition == 0 goto end
     *    statements
     *    jmp start
     * end
     */
    size_t start = generator->count;

    Register condition = generate_for_expression(generator, ast, while_stmt.condition);
    generator->current_register--;  // Consume the expression register

    Instruction* jump_to_else = jmp_zero(generator, condition);
    generate_for_statement(generator, ast, (Node*) while_stmt.then_block);

    Instruction* jump_to_start = jmp(generator);
    jump_to_start->jmp.label = start;

    if (while_stmt.else_block != NULL) {
        assert(0 && "not implemented");

        Instruction* jump_to_end = jmp(generator);
        jump_to_else->jmp.label = generator->count;

        generate_for_statement(generator, ast, (Node*) while_stmt.else_block);
        jump_to_end->jmp.label = generator->count;
    } else {
        jump_to_else->jmp.label = generator->count;
    }
}

Register generate_for_call(Generator* generator, TypedAst ast, NodeCall call) {
    if (strcmp(call.name, "print") == 0) {
        Register src = generate_for_expression(generator, ast, call.args[0]);
        generator->instructions[generator->count++] = (Instruction) {
                .type = Instruction_Print,
                .call.label = src,
        };
        return 0;
    }

    assert(call.args == NULL && "not implemented");
    Instruction* jump = jmp(generator);

    for (size_t i = 0; i < generator->deferred_blocks_count; ++i) {
        DeferredBlock* deferred = generator->deferred_blocks + i;
        if (deferred->name == call.name) {
            deferred->references[deferred->references_count++] = jump;
            return generator->current_register++;
        }
    }
    assert(0 && "Unknown function");
}

void generate_for_fun_decl(Generator* generator, TypedAst ast, NodeFunDecl fun_decl) {
    assert(fun_decl.params == NULL && "not implemented");

    generator->deferred_blocks[generator->deferred_blocks_count++] = (DeferredBlock) {
        .name = fun_decl.name,
        .node = (Node*) fun_decl.block,
        .references = malloc(sizeof(Instruction*) * 12),
        .references_count = 0,
    };
}

void generate_for_statement(Generator* generator, TypedAst ast, Node* node) {
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary:
        case NodeKind_Call: {
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
        case NodeKind_While: {
            generate_for_while_stmt(generator, ast, node->while_stmt);
            break;
        }
        case NodeKind_Block: {
            NodeBlock block = node->block;
            Block* parent = ast.block;
            ast.block = parent + block.id;
            while ((node = *block.nodes++) != NULL) {
                generate_for_statement(generator, ast, node);
            }
            ast.block = parent;
            break;
        }
        case NodeKind_FunDecl: {
            generate_for_fun_decl(generator, ast, node->fun_decl);
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
        .deferred_blocks = malloc(sizeof(DeferredBlock) * 1024),
        .deferred_blocks_count = 0,
    };

    Node* node = ast.start;
    generate_for_statement(&generator, ast, node);

    if (generator.current_register > 0) {
        mov_reg(&generator, 0, generator.current_register);
    }
    generator.instructions[generator.count++] = (Instruction) {
        .type = Instruction_Exit,
    };

    size_t return_label = generator.count - 1;
    for (size_t i = 0; i < generator.deferred_blocks_count; ++i) {
        DeferredBlock* deferred = generator.deferred_blocks + i;
        for (size_t j = 0; j < deferred->references_count; ++j) {
            Instruction* instruction = deferred->references[j];
            instruction->jmp.label = generator.count;
        }
        generate_for_statement(&generator, ast, deferred->node);
        jmp(&generator)->jmp.label = return_label;
    }

    // Check all labels are patched
    for (size_t i = 0; i < generator.count; ++i) {
        Instruction* instruction = generator.instructions + i;
        if (instruction->type == Instruction_JmpZero || instruction->type == Instruction_Jmp) {
            assert(instruction->jmp.label != 0xdeadbeef && "Invalid label");
        }
    }

    return (Bytecode) { generator.instructions, generator.count };
}
