#include "generator.h"

#include "assert.h"
#include "generator.h"
#include "../parser/visitor.h"
#include "disassembler.h"
#include "logger.h"

#include <stdlib.h>
#include <stdio.h>


#define BP 0
#define SP 1
#define REG_BASE 2


typedef struct {
    const char* name;
    Node* node;
    Instruction** references;
    size_t references_count;
    size_t entry;
} DeferredBlock;

typedef struct {
    Node* node;
    size_t instruction_start;
    size_t instruction_end;
} DebugInfo;

typedef struct {
    Visitor visitors;
    TypedAst ast;
    
    Instruction* instructions;
    size_t       count;
    Register     current_register;

    DebugInfo* debug_info;

    Instruction** return_statements;
    int           return_statements_count;

    Block* current;
    int    fun_block;

    DeferredBlock* deferred_blocks;
    size_t         deferred_blocks_count;
} Generator;

Bytecode generator_to_bytecode(Generator* generator) {
    typed_ast_free(generator->ast);
    for (size_t i = 0; i < generator->deferred_blocks_count; ++i) {
        dealloc(generator->deferred_blocks[i].references);
    }
    dealloc(generator->return_statements);
    dealloc(generator->deferred_blocks);
    dealloc(generator->debug_info);

    return (Bytecode) { generator->instructions, generator->count };
}


Register register_alloc(Generator* generator) {
    return generator->current_register++;
}

void register_free(Generator* generator) {
    assert(generator->current_register >= REG_BASE && "Invalid register count");
    generator->current_register--;
}


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

Register uny_op(Generator* generator, InstructionType unary_op, Register dst, Register src) {
    generator->instructions[generator->count++] = (Instruction) {
        .type = unary_op,
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
    return src;
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

static Local* find_local(const Generator* generator, const char* name) {
    Block* current = generator->current;
    while (current != NULL) {
        for (i32 i = 0; i < (i32) current->count; ++i) {
            Local* local = current->locals + i;
            assert(
                (local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl || local->decl->kind == NodeKind_FunParam || local->decl->kind == NodeKind_Struct)
                && "Invalid node kind"
            );

            switch (local->decl->kind) {
                case NodeKind_VarDecl:
                    if (local->decl->var_decl.name == name) {
                        return local;
                    }
                    break;
                case NodeKind_FunDecl:
                    if (local->decl->fun_decl.name == name)
                        return local;
                    break;
                case NodeKind_FunParam:
                    if (local->decl->fun_param.name == name)
                        return local;
                    break;
                case NodeKind_Struct:
                    if (local->decl->struct_decl.name == name)
                        return local;
                    break;
                default:
                    assert(0 && "Invalid node kind");
            }
        }
        current = (current->parent == -1) ? NULL : generator->ast.block + current->parent;
    }

    assert(0 && "Unknown identifier");
    exit(1);
}


/* ---------------------------- GENERATOR VISITOR -------------------------------- */
// Adds 1 scratch register.
Register generate_literal(Generator* generator, const NodeLiteral* literal) {
    switch (literal->type) {
        case LiteralType_Boolean: {
            Register dst = register_alloc(generator);
            return mov_imm64(generator, dst, literal->value.integer != 0);
        } break;
        case LiteralType_Integer: {
            Register dst = register_alloc(generator);
            return mov_imm64(generator, dst, literal->value.integer);
        } break;
        case LiteralType_Real: {
            Register dst = register_alloc(generator);
            return mov_imm64(generator, dst, literal->value.integer);
        } break;
        case LiteralType_String: {
            Register dst = register_alloc(generator);
            return mov_imm64(generator, dst, (u64) literal->value.string);
        } break;
        default:
            assert(0 && "Invalid literal type");
            return 0;
    }
}

// Adds 1 scratch register.
Register generate_identifier(Generator* generator, const NodeIdentifier* identifier) {
    Local* local = find_local(generator, identifier->name);
    if (local != NULL) {
        Register dst = register_alloc(generator);
        return load(generator, dst, local->decl->var_decl.decl_offset);
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", identifier->name);
    return 0;
}

// Adds 1 scratch register.
Register generate_unary(Generator* generator, const NodeUnary* node) {
    static const InstructionType unary_op[] = {
            [UnaryOp_Not] = Instruction_Not,
            [UnaryOp_Neg] = Instruction_Neg,
    };
    assert(node->op <= UNARY_OP_LAST && "Invalid unary operation");
    InstructionType inst = unary_op[node->op];
    assert(inst != 0 && "Invalid unary operation");

    Register dst = register_alloc(generator);
    Register src = (Register) visit(generator, node->expr);
    Register reg = uny_op(generator, inst, dst, src);

    // NOTE(ted): Both left and right adds a new scratch register.
    //            We can free the first (left) one.
    register_free(generator);

    return reg;
}

// Adds 1 scratch register.
Register generate_binary(Generator* generator, const NodeBinary* binary) {
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
            [BinaryOp_And] = Instruction_And,
            [BinaryOp_Or]  = Instruction_Or,
    };
    assert(binary->op <= BINARY_OP_LAST && "Invalid binary operation");
    InstructionType inst = binary_op[binary->op];
    assert(inst != 0 && "Invalid binary operation");

    Register dst = (Register) visit(generator, binary->left);
    Register src = (Register) visit(generator, binary->right);
    Register reg = bin_op(generator, inst, dst, src);

    // NOTE(ted): Both left and right adds a new scratch register.
    //            We can free the first (left) one.
    register_free(generator);

    return reg;
}

// Should not increase any scratch registers.
Register generate_assign(Generator* generator, const NodeAssign* assign) {
    Register src = (Register) visit(generator, assign->expression);

    Local* local = find_local(generator, assign->name);
    if (local != NULL) {
        if (local->decl->kind == NodeKind_FunParam) {
            assert(0 && "not implemented");
            // TODO: Need to push.
            // return store(generator, generator->call_reg[local.index], src);
        } else {
            // Free the scratch register.
            register_free(generator);
            return store(generator, local->decl->var_decl.decl_offset, src);
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", assign->name);
    return 0;
}

Register generate_var_decl(Generator* generator, const NodeVarDecl* var_decl) {
    Local*   dst = find_local(generator, var_decl->name);
    Register src = (Register) visit(generator, var_decl->expression);

    // Free the scratch register.
    register_free(generator);

    return store(generator, dst->decl->var_decl.decl_offset, src);
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
    Register condition = (Register) visit(generator, if_stmt->condition);
    register_free(generator);  // Consume the expression register

    Register start_register = generator->current_register;
    Instruction* jump_to_else = jmp_zero(generator, condition);
    visit(generator, (Node*) if_stmt->then_block);

    if (if_stmt->else_block != NULL) {
        Instruction* jump_to_end = jmp(generator);
        jump_to_else->jmp.label = (i32) generator->count;

        Register current = generator->current_register;
        generator->current_register = start_register;
        visit(generator, (Node*) if_stmt->else_block);
        jump_to_end->jmp.label = (i32) generator->count;
        generator->current_register = current;
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

    Register condition = (Register) visit(generator, while_stmt->condition);
    register_free(generator);  // Consume the expression register

    Instruction* jump_to_else = jmp_zero(generator, condition);
    visit(generator, (Node*) while_stmt->then_block);

    Instruction* jump_to_start = jmp(generator);
    jump_to_start->jmp.label = (Register) start;

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

// TODO: FIX
Register generate_access(Generator* generator, const NodeAccess* node) {
    Local* variable = find_local(generator, node->left->identifier.name);
    assert(variable->decl->kind == NodeKind_VarDecl && "Invalid node kind");

    NodeId object_id = node_id(&generator->ast.tree, node->left);
    TypeId type = generator->ast.types[object_id];
    TypeInfo type_info = generator->ast.type_info[type];

    Local* struct_ = find_local(generator, type_info.name);
    assert(struct_->decl->kind == NodeKind_Struct && "Invalid node kind");
    NodeStruct* struct_decl = &struct_->decl->struct_decl;

    for (i32 i = 0; i < struct_decl->count; ++i) {
        NodeStructField* field = (NodeStructField*) struct_decl->nodes[i];
        if (field->name == node->right->identifier.name) {
            Register dst = register_alloc(generator);
            return load(generator, dst, variable->decl->var_decl.decl_offset + field->offset);
        }
    }
    assert(0 && "Unknown field");
}

Register generate_call(Generator* generator, const NodeCall* fn_call) {
    Local* result;
    if (strcmp(fn_call->name, "print") != 0) {
        result = find_local(generator, fn_call->name);
        assert(result->decl->kind == NodeKind_FunDecl && "Not a function");
    }

    for (i32 i = 0; i < fn_call->count; ++i) {
        Node* arg = fn_call->args[i];
        Register reg = (Register)(size_t)visit(generator, arg);

        push(generator, REG_BASE + i);
        mov_reg(generator, REG_BASE + i, reg);
    }

    for (i32 i = 0; i < fn_call->count; ++i) {
        // NOTE(ted): The argument registers are free to use after the call.
        register_free(generator);
    }

    Instruction* label;
    if (strcmp(fn_call->name, "print") == 0) {
        generator->instructions[generator->count++] = (Instruction) {
            .type = Instruction_Print,
        };
        // NOTE(ted): Restore the registers used for arguments.
        for (i32 i = fn_call->count-1; i >= 0; --i) {
            pop(generator, REG_BASE + i);
        }
        // NOTE(ted): 'print' does not return a value.
        return -1;
    } else {
        label = call(generator);
        generator->current_register += 1;
    }

    // NOTE(ted): Restore the registers used for arguments, except the return register.
    for (i32 i = fn_call->count-1; i >= 1; --i) {
        pop(generator, REG_BASE + i);
    }

    // NOTE(ted): If the function has a return value, then store it in a new register.
    Register dst = -1;
    if (result->decl->fun_decl.return_type) {
        dst = mov_reg(generator, generator->current_register++, REG_BASE);
    }

    // NOTE(ted): Restore the return register.
    if (fn_call->count > 0)
        pop(generator, REG_BASE);

    // NOTE(ted): Iterating backwards, so we find the last function declaration.
    for (int i = (int)generator->deferred_blocks_count-1; i >= 0; --i) {
        DeferredBlock* deferred = generator->deferred_blocks + i;
        if (deferred->name == fn_call->name) {
            if (deferred->entry != (size_t)-1) {
                label->call.label = (i32) deferred->entry;
            } else {
                deferred->references[deferred->references_count++] = label;
            }
            return dst;
        }
    }
    assert(0 && "Unknown function");
    return 0;
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

Register generate_fun_body(Generator* generator, const NodeFunBody* node) {
    Block* current = generator->current;
    generator->current = generator->ast.block + node->id;
    for (i32 i = 0; i < node->count; ++i) {
        visit(generator, node->nodes[i]);
    }
    generator->current = current;
    return -1;
}



Register generate_deferred(Generator* generator, const NodeFunDecl* node) {
    // Prologue
    push(generator, BP);
    mov_reg(generator, BP, SP);

    // Allocate space for parameters
    bin_op(generator, Instruction_Add_Imm, SP, node->param_count);
    // Allocate space for locals
    // TODO: This is not correct, we need to allocate space for locals of different sizes.
    bin_op(generator, Instruction_Add_Imm, SP, node->body->decls);

    Block* current = generator->current;
    generator->current = generator->ast.block + node->body->id;

    for (i32 i = 0; i < node->param_count; ++i) {
        NodeFunParam* param = node->params[i];
        Local* local = find_local(generator, param->name);
        assert(local->decl->kind == NodeKind_FunParam && "Invalid node kind");
        store(generator, local->decl->fun_param.offset, REG_BASE + i);
    }

    for (i32 i = 0; i < node->body->count; ++i) {
        visit(generator, node->body->nodes[i]);
    }
    generator->current = current;

    for (int i = 0; i < generator->return_statements_count; ++i) {
        Instruction* return_statement = generator->return_statements[i];
        return_statement->jmp.label = (i32) generator->count;
    }
    generator->return_statements_count = 0;

    // Epilogue
    mov_reg(generator, SP, BP);
    pop(generator, BP);
    ret(generator);
    return -1;
}

Register generate_fun_decl(Generator* generator, const NodeFunDecl* fun_decl) {
    generator->deferred_blocks[generator->deferred_blocks_count++] = (DeferredBlock) {
        .name = fun_decl->name,
        .node = (Node*) fun_decl,
        .references = alloc(sizeof(Instruction*) * 256),
        .references_count = 0,
        .entry = -1,
    };
    return -1;
}

Register generate_return_stmt(Generator* generator, const NodeReturn* node) {
    Register src = (Register) visit(generator, node->expression);
    register_free(generator);  // Consume the expression register

    mov_reg(generator, REG_BASE, src);
    Instruction* jump = jmp(generator);
    generator->return_statements[generator->return_statements_count++] = jump;

    return -1;
}

Register generate_type(Generator* generator, const NodeType* node) {
    (void) generator;
    (void) node;
    return -1;
}

Register generate_init_arg(Generator* generator, const NodeInitArg* node) {
    Register dst = register_alloc(generator);
    Register src = (Register) visit(generator, node->expr);
    register_free(generator);
    push(generator, src);
    pop(generator, dst);
    return dst;
}

Register generate_init(Generator* generator, const NodeInit* node) {
    Register start = (Register) -1;
    for (i32 i = 0; i < node->count; ++i) {
        if (start == (Register) -1) {
            start = (Register) visit(generator, (Node*) node->args[i]);
        } else {
            visit(generator, (Node*) node->args[i]);
            register_free(generator);
        }
    }
    return start;
}

Register generate_struct_field(Generator* generator, const NodeStructField* node) {
    (void) generator;
    (void) node;
    return -1;
}

Register generate_struct_decl(Generator* generator, const NodeStruct* node) {
    (void) generator;
    (void) node;
    return -1;
}

Register generate_module(Generator* generator, const NodeModule* node) {
    Block* current = generator->current;
    generator->current = generator->ast.block;
    for (i32 i = 0; i < node->decl_count; ++i) {
        visit(generator, node->decls[i]);
    }
    for (i32 i = 0; i < node->stmt_count; ++i) {
        visit(generator, node->stmts[i]);
    }
    generator->current = current;
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
        .instructions = alloc(sizeof(Instruction) * 1024),
        .count = 0,
        .fun_block = 0,
        .current_register = REG_BASE,
        .debug_info = alloc(sizeof(DebugInfo) * 4096),
        .return_statements = alloc(sizeof(Instruction*) * 1024),
        .return_statements_count = 0,
        .current = ast.block,
        .deferred_blocks = alloc(sizeof(DeferredBlock) * 1024),
        .deferred_blocks_count = 0,
    };

    Node* node = ast.tree.start;

    if (node->kind == NodeKind_Module) {
        // Allocate space for locals
        // TODO: This is not correct, we need to allocate space for locals of different sizes.
        bin_op(&generator, Instruction_Add_Imm, SP, node->module.global_count);
    }
    visit(&generator, node);
    if (node->kind == NodeKind_Module) {
        // TODO: This is not correct, we need to allocate space for locals of different sizes.
        bin_op(&generator, Instruction_Add_Imm, SP, -node->module.global_count);
    }
    mov_reg(&generator, REG_BASE, generator.current_register-1);
    generator.instructions[generator.count++] = (Instruction) {
        .type = Instruction_Exit,
    };

    for (size_t i = 0; i < generator.deferred_blocks_count; ++i) {
        DeferredBlock* deferred = &generator.deferred_blocks[i];
        assert(deferred->node->kind == NodeKind_FunDecl && "Invalid node kind");
        NodeFunDecl* fun_decl = &deferred->node->fun_decl;
        for (size_t j = 0; j < deferred->references_count; ++j) {
            Instruction* instruction = deferred->references[j];
            instruction->call.label = (i32) generator.count;
        }
        deferred->entry = (i32) generator.count;

        generator.current_register = REG_BASE;
        generate_deferred(&generator, fun_decl);
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

    return generator_to_bytecode(&generator);
}
