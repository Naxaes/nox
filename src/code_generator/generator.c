#include "assert.h"
#include "generator.h"
#include "../parser/visitor.h"

#include <stdlib.h>
#include <stdio.h>


typedef struct {
    const char* name;
    Node* node;
    Instruction** references;
    size_t references_count;
} DeferredBlock;

typedef struct {
    Visitor visitors;
    TypedAst ast;
    
    Instruction* instructions;
    size_t       count;
    Register     current_register;

    Block* current;
    int    fun_block;

    DeferredBlock* deferred_blocks;
    size_t         deferred_blocks_count;

    // Written to by callee, read by caller.
    Register call_reg[8];
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
            .jmp.label = 0xdead,
            .jmp.src   = src,
    };
    return generator->instructions + generator->count++;
}

// NOTE(ted): This must be patched!
Instruction* jmp(Generator* generator) {
    generator->instructions[generator->count] = (Instruction) {
            .type = Instruction_Jmp,
            .jmp.label = 0xdead,
    };
    return generator->instructions + generator->count++;
}

Instruction* call(Generator* generator) {
    generator->instructions[generator->count] = (Instruction) {
            .type = Instruction_Call,
            .call.label = 0xdead,
    };
    return generator->instructions + generator->count++;
}

void ret(Generator* generator) {
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Ret,
    };
}

void push(Generator* generator, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Push,
            .reg.src = src,
    };
}

void pop(Generator* generator, Register dst) {
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Pop,
            .reg.dst = dst,
    };
}

typedef struct {
    i32    parent;
    i32    index;
    Local* local;
} LocalResult;
static LocalResult find_local(const Generator* generator, const char* name) {
    i32 parent = 0;
    Block* current = generator->current;
    while (current != NULL) {
        for (i32 i = 0; i < (i32) current->count; ++i) {
            Local* local = current->locals + i;
            assert((local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl || local->decl->kind == NodeKind_FunParam) && "Invalid node kind");
            if (local->decl->var_decl.name == name) {
                return (LocalResult) { parent, i, local };
            }
        }
        current = generator->ast.block + current->parent;
        parent += 1;
    }

    assert(0 && "Unknown identifier");
    exit(1);
}


/* ---------------------------- GENERATOR VISITOR -------------------------------- */
Register generate_literal(Generator* generator, const NodeLiteral* literal) {
    switch (literal->type) {
        case LiteralType_Boolean:
            return mov_imm64(generator, generator->current_register++, literal->value.integer != 0);
        case LiteralType_Integer:
            return mov_imm64(generator, generator->current_register++, literal->value.integer);
        case LiteralType_Real:
            assert(0 && "not implemented");
        case LiteralType_String:
            return mov_imm64(generator, generator->current_register++, (u64) literal->value.string);
        default:
            assert(0 && "Invalid literal type");
    }
}

Register generate_identifier(Generator* generator, const NodeIdentifier* identifier) {
    LocalResult result = find_local(generator, identifier->name);
    if (result.local != NULL) {
        if (result.local->decl->kind == NodeKind_FunParam) {
            return load(generator, generator->current_register++, generator->call_reg[result.index]);
        } else {
            return load(generator, generator->current_register++, result.index);
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", identifier->name);
    return 0;
}

Register generate_binary(Generator* generator, const NodeBinary* binary) {
    Register dest   = (Register)(size_t)visit(generator, binary->left);
    Register source = (Register)(size_t)visit(generator, binary->right);
    generator->current_register--;  // Consume the expression register
    static const InstructionType binary_op[] = {
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
    assert(binary->op <= BinaryOp_Gt && "Invalid binary operation");
    InstructionType inst = binary_op[binary->op];
    assert(inst != 0 && "Invalid binary operation");
    Register reg = bin_op(generator, inst, dest, source);
    return reg;
}

Register generate_assign(Generator* generator, const NodeAssign* assign) {
    Register src = (Register)(size_t)visit(generator, assign->expression);
    generator->current_register--;  // Consume the expression register

    LocalResult local = find_local(generator, assign->name);
    if (local.local != NULL) {
        if (local.local->decl->kind == NodeKind_FunParam) {
            assert(0 && "not implemented");
            // TODO: Need to push.
            // return store(generator, generator->call_reg[local.index], src);
        } else {
            return store(generator, local.index, src);
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", assign->name);
    return 0;
}

Register generate_var_decl(Generator* generator, const NodeVarDecl* var_decl) {
    LocalResult dest = find_local(generator, var_decl->name);
    Register src  = (Register)(size_t)visit(generator, var_decl->expression);
    generator->current_register--;  // Consume the expression register
    store(generator, dest.index, src);
    generator->current_register++;  // Increase the register count
    return dest.index;
}

Register generate_if_stmt(Generator* generator, const NodeIf* if_stmt) {
    /*
     * if condition == 0 goto else
     *    statements
     * jmp end
     * else:
     *    statements
     * end:
     */
    Register condition = (Register)(size_t)visit(generator, if_stmt->condition);
    generator->current_register--;  // Consume the expression register

    Instruction* jump_to_else = jmp_zero(generator, condition);
    visit(generator, (Node*) if_stmt->then_block);

    if (if_stmt->else_block != NULL) {
        Instruction* jump_to_end = jmp(generator);
        jump_to_else->jmp.label = (i32) generator->count;

        visit(generator, (Node*) if_stmt->else_block);
        jump_to_end->jmp.label = (i32) generator->count;
    } else {
        jump_to_else->jmp.label = (i32) generator->count;
    }

    return -1;
}

Register generate_while_stmt(Generator* generator, const NodeWhile* while_stmt) {
    /*
     * start:
     * if condition == 0 goto end
     *    statements
     *    jmp start
     * end
     */
    size_t start = generator->count;

    Register condition = (Register)(size_t)visit(generator, while_stmt->condition);
    generator->current_register--;  // Consume the expression register

    Instruction* jump_to_else = jmp_zero(generator, condition);
    visit(generator, (Node*) while_stmt->then_block);

    Instruction* jump_to_start = jmp(generator);
    jump_to_start->jmp.label = start;

    if (while_stmt->else_block != NULL) {
        assert(0 && "not implemented");

//        Instruction* jump_to_end = jmp(generator);
//        jump_to_else->jmp.label = generator->count;
//
//        visit(generator, (Node*) while_stmt->else_block);
//        jump_to_end->jmp.label = generator->count;
    } else {
        jump_to_else->jmp.label = (i32) generator->count;
    }
    return -1;
}

Register generate_call(Generator* generator, const NodeCall* fn_call) {
    if (strcmp(fn_call->name, "print") == 0) {
        Register src = (Register)(size_t)visit(generator, fn_call->args[0]);
        generator->instructions[generator->count++] = (Instruction) {
                .type = Instruction_Print,
                .call.label = src,
        };
        return 0;
    }

    LocalResult result = find_local(generator, fn_call->name);
    assert(result.local->decl->kind == NodeKind_FunDecl && "Not a function");
    NodeFunDecl fun_decl = result.local->decl->fun_decl;

    for (i32 i = 0; i < fn_call->count; ++i) {
        Node* arg = fn_call->args[i];
        Register reg = (Register)(size_t)visit(generator, arg);
        mov_reg(generator, generator->call_reg[i], reg);
    }

    Instruction* label = call(generator);

    generator->current_register -= fn_call->count;  // Consume the expression register

    // TODO: Assuming all functions return one value for now.
    if (fun_decl.return_type != NULL)
        generator->current_register += 1;

    for (size_t i = 0; i < generator->deferred_blocks_count; ++i) {
        DeferredBlock* deferred = generator->deferred_blocks + i;
        if (deferred->name == fn_call->name) {
            deferred->references[deferred->references_count++] = label;
            return 0;
        }
    }
    assert(0 && "Unknown function");
}

Register generate_fun_param(Generator* generator, const NodeFunParam* node) {
    (void) generator;
    (void) node;
    return -1;
}

Register generate_block(Generator* generator, const NodeBlock* node) {
    Block* current = generator->current;
    generator->current = generator->ast.block + node->id;
    for (i32 i = 0; i < node->count; ++i) {
        visit(generator, node->nodes[i]);
    }
    generator->current = current;
    return -1;
}

Register generate_fun_decl(Generator* generator, const NodeFunDecl* fun_decl) {
    generator->deferred_blocks[generator->deferred_blocks_count++] = (DeferredBlock) {
        .name = fun_decl->name,
        .node = (Node*) fun_decl,
        .references = malloc(sizeof(Instruction*) * 12),
        .references_count = 0,
    };
    return -1;
}

Register generate_return_stmt(Generator* generator, const NodeReturn* node) {
    Register src = (Register)(size_t)visit(generator, node->expression);
    generator->current_register--;  // Consume the expression register

    if (src > 0) {
        mov_reg(generator, 0, src);
    }
    generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Ret,
    };
    return src;
}

Register generate_type(Generator* generator, const NodeType* node) {
    (void) generator;
    (void) node;
    return -1;
}

Bytecode generate_code(TypedAst ast) {
    Generator generator = {
        .visitors = {
#define X(upper, lower, flags, body) .visit_##lower = (Visit##upper##Fn) generate_##lower,
                ALL_NODES(X)
        },
#undef X
        .ast = ast,
        .instructions = malloc(sizeof(Instruction) * 1024),
        .count = 0,
        .fun_block = 0,
        .current_register = 0,
        .current = ast.block,
        .deferred_blocks = malloc(sizeof(DeferredBlock) * 1024),
        .deferred_blocks_count = 0,
        .call_reg = { 8, 9, 10, 11, 12, 13, 14, 15 }
    };

    Node* node = ast.start;
    visit(&generator, node);

    generator.instructions[generator.count++] = (Instruction) {
        .type = Instruction_Exit,
    };

    for (size_t i = 0; i < generator.deferred_blocks_count; ++i) {
        DeferredBlock* deferred = generator.deferred_blocks + i;
        assert(deferred->node->kind == NodeKind_FunDecl && "Invalid node kind");
        for (size_t j = 0; j < deferred->references_count; ++j) {
            Instruction* instruction = deferred->references[j];
            instruction->call.label = (i32) generator.count;
        }

        generator.current_register = 0;
        visit(&generator, (Node*)deferred->node->fun_decl.block);
        if (generator.instructions[generator.count - 1].type != Instruction_Ret) {
            ret(&generator);
        }
    }

    // Check all labels are patched
    for (size_t i = 0; i < generator.count; ++i) {
        Instruction* instruction = generator.instructions + i;
        if (instruction->type == Instruction_JmpZero || instruction->type == Instruction_Jmp || instruction->type == Instruction_Call) {
            assert(instruction->jmp.label != 0xdead && "Invalid label");
        }
    }

    return (Bytecode) { generator.instructions, generator.count };
}
