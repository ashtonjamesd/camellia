#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>

#include "compile.h"

static void generate_node(Compiler *c, AstNode *node);

Compiler *init_compiler(AstNode **tree, int count, char *exe, int emitAsm, int emitObj) {
    Compiler *c = malloc(sizeof(Compiler));
    if (!c) {
        perror("Error allocating compiler.");
        return NULL;
    }
    
    c->tree = tree;
    c->node_count = count;
    c->exe = exe;
    c->emitAsm = emitAsm;
    c->emitObj = emitObj;

    c->symbol_table = malloc(sizeof(SymbolTable));
    c->symbol_table->capacity = 1;
    c->symbol_table->count = 0;
    c->symbol_table->symbols = malloc(sizeof(Symbol));

    return c;
}

void free_compiler(Compiler *c) {
    if (!c) return;

    for (int i = 0; i < c->symbol_table->count; i++) {
        free(c->symbol_table->symbols[i].name);
    }
    free(c->symbol_table->symbols);
    free(c->symbol_table);
    free(c);
}

static void put(Compiler *c, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(c->file, "  ");
    vfprintf(c->file, format, args);
    fprintf(c->file, "\n");
    va_end(args);
}

static void putf(Compiler *c, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(c->file, format, args);
    fprintf(c->file, "\n");
    va_end(args);
}

static inline void sys_call(Compiler *c) {
    put(c, "syscall");
}

static inline void call(char *func, Compiler *c) {
    put(c, "call %s", func);
}

static void symbol_table_add(SymbolTable *table, const char *name, int offset) {
    if (table->count >= table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, sizeof(Symbol) * table->capacity);
    }
    table->symbols[table->count].name = strdup(name);
    table->symbols[table->count].offset = offset;

    table->count++;
}

static int symbol_table_lookup(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(name, table->symbols[i].name) == 0) {
            return table->symbols[i].offset;
        }
    }

    return 0;
} 

static void generate_return(Compiler *c, AstReturn *ret) {
    if (ret->value->type == AST_LITERAL_INT) {
        put(c, "mov rax, %d", ret->value->as.lit_int->value); 
    }
    else if (ret->value->type == AST_LITERAL_CHAR) {
        put(c, "mov rax, %d", ret->value->as.lit_int->value); 
    }
    else if (ret->value->type == AST_CALL_EXPR) {
        put(c, "call %s", ret->value->as.call->identifier); 
    }
    else if (ret->value->type == AST_BINARY) {
        generate_node(c, ret->value);
    }
    else if (ret->value->type == AST_IDENTIFIER) {
        int offset = symbol_table_lookup(c->symbol_table, ret->value->as.ident->name);
        put(c, "mov rax, [rbp%d]", offset);
    }
}

static void generate_function(Compiler *c, AstFunctionDeclaration *func) {
    fprintf(c->file, "\n%s:\n", func->identifier);
    put(c, "push rbp");
    put(c, "mov rbp, rsp");

    int offset = -8;
    for (int i = 0; i < func->body_count; i++) {
        if (func->body[i]->type == AST_VARIABLE_DECLARATION) {
            symbol_table_add(c->symbol_table, func->body[i]->as.var_dec->identifier, offset);
            offset -= 8;
        }
    }
    put(c, "sub rsp, %d", (-offset));


    for (int i = 0; i < func->body_count; i++) {
        generate_node(c, func->body[i]);

        // skips dead code
        if (func->body[i]->type == AST_RETURN) {
            break;
        }
    }

    put(c, "mov rsp, rbp"); 
    put(c, "pop rbp");
    put(c, "ret"); 
}

static void generate_call_expr(Compiler *c, AstCallExpr *call_expr) {
    call(call_expr->identifier, c);
}

static void generate_binary_expr(Compiler *c, AstBinaryExpr *binary) {
    if (binary->op.type == TOKEN_PLUS) {
        generate_node(c, binary->left);
        put(c, "push rax");
        generate_node(c, binary->right);

        put(c, "pop rbx");
        put(c, "add rax, rbx");
    }
    else if (binary->op.type == TOKEN_MINUS) {
        generate_node(c, binary->left);
        put(c, "push rax");
        generate_node(c, binary->right);

        put(c, "mov rbx, rax");
        put(c, "pop rax");
        put(c, "sub rax, rbx");
    }
    else if (binary->op.type == TOKEN_STAR) {
        generate_node(c, binary->left);
        put(c, "push rax");
        generate_node(c, binary->right);

        put(c, "pop rbx");
        put(c, "imul rax, rbx");
    }
    else {
        fprintf(stderr, "Unsupported binary operator.\n");
    }
}

static void generate_lit_int(Compiler *c, AstLiteralInt *lit) {
    put(c, "mov rax, %d", lit->value);
}

static void generate_inline_asm(Compiler *c, AstInlineAsmBlock *asm_inl) {
    for (int i = 0; i < asm_inl->line_count; i++) {
        put(c, "%s", asm_inl->lines[i]);
    }
}

static void generate_variable_declaration(Compiler *c, AstVariableDeclaration *var_dec) {
    int stack_offset = symbol_table_lookup(c->symbol_table, var_dec->identifier);

    if (var_dec->value->type == AST_LITERAL_INT) {
        put(c, "mov qword [rbp%d], %d", stack_offset, var_dec->value->as.lit_int->value);
    }
    else if (var_dec->value->type == AST_IDENTIFIER) {
        int source_offset = symbol_table_lookup(c->symbol_table, var_dec->value->as.ident->name);
        put(c, "mov rax, qword [rbp%d]", source_offset);
        put(c, "mov qword [rbp%d], rax", stack_offset);
    }
    else if (var_dec->value->type == AST_BINARY) {
        generate_binary_expr(c, var_dec->value->as.binary);
        put(c, "mov qword [rbp%d], rax", stack_offset);
    }
}

static void generate_assignment(Compiler *c, AstAssignment *assign) {
    int offset = symbol_table_lookup(c->symbol_table, assign->identifier);

    generate_node(c, assign->value);
    put(c, "mov qword [rbp%d], rax", offset);
}

static void generate_if_statement(Compiler *c, AstIfStatement *iff) {
    static int label_counter = 0;
    int end_label = label_counter++;
    int else_label = label_counter++;

    if (iff->condition->type == AST_BINARY) {
        generate_node(c, iff->condition->as.binary->left);
        put(c, "push rax");
        generate_node(c, iff->condition->as.binary->right);
        put(c, "mov rbx, rax");
        put(c, "pop rax");
        put(c, "cmp rax, rbx");

        switch (iff->condition->as.binary->op.type) {
            case TOKEN_GREATER_THAN:
                put(c, "jle .Lelse%d", else_label);
                break;
            case TOKEN_LESS_THAN:
                put(c, "jge .Lelse%d", else_label);
                break;
            case TOKEN_GREATER_THAN_EQUALS:
                put(c, "jl .Lelse%d", else_label);
                break;
            case TOKEN_LESS_THAN_EQUALS:
                put(c, "jg .Lelse%d", else_label);
                break;
            case TOKEN_EQUALS:
                put(c, "jne .Lelse%d", else_label);
                break;
            case TOKEN_NOT_EQUALS:
                put(c, "je .Lelse%d", else_label);
                break;
            default:
                fprintf(stderr, "unsupported binary operator");
                break;
        }
    }
    else {
        generate_node(c, iff->condition);
        put(c, "cmp rax, 0");
        put(c, "je .Lelse%d", else_label);
    }

    for (int i = 0; i < iff->body_count; i++) {
        generate_node(c, iff->body[i]);
    }
    put(c, "jmp .Lend%d", end_label);

    put(c, ".Lelse%d:", else_label);
    for (int i = 0; i < iff->else_body_count; i++) {
        generate_node(c, iff->else_body[i]);
    }

    putf(c, ".Lend%d:", end_label);
}

static void generate_while_statement(Compiler *c, AstWhile *whilee) {
    static int while_label = 0;
    int start_label = while_label++;
    int end_label = while_label++;

    putf(c, ".Lwhile_start%d:", start_label);

    if (whilee->condition->type == AST_BINARY) {
        generate_node(c, whilee->condition->as.binary->left);
        put(c, "push rax");
        generate_node(c, whilee->condition->as.binary->right);
        put(c, "mov rbx, rax");
        put(c, "pop rax");
        put(c, "cmp rax, rbx");
        
        switch (whilee->condition->as.binary->op.type) {
            case TOKEN_EQUALS:
                put(c, "jne .Lwhile_end%d", end_label); 
                break;
            case TOKEN_NOT_EQUALS:
                put(c, "je .Lwhile_end%d", end_label); 
                break;
            case TOKEN_LESS_THAN:
                put(c, "jge .Lwhile_end%d", end_label); 
                break;
            case TOKEN_GREATER_THAN:
                put(c, "jle .Lwhile_end%d", end_label); 
                break;
            case TOKEN_LESS_THAN_EQUALS:
                put(c, "jg .Lwhile_end%d", end_label); 
                break;
            case TOKEN_GREATER_THAN_EQUALS:
                put(c, "jl .Lwhile_end%d", end_label); 
                break;
            default:
                fprintf(stderr, "Unsupported comparison operator in while condition.\n");
                return;
        }
    }
    else {
        generate_node(c, whilee->condition);
        put(c, "cmp rax, 0");
        put(c, "je .Lwhile_end%d", end_label);
    }

    for (int i = 0; i < whilee->body_count; i++) {
        generate_node(c, whilee->body[i]);
    }

    put(c, "jmp .Lwhile_start%d", start_label);
    putf(c, ".Lwhile_end%d:", end_label);
}

static void generate_node(Compiler *c, AstNode *node) {
    if (node->type == AST_FUNCTION) {
        generate_function(c, node->as.func);
    }
    else if (node->type == AST_RETURN) {
        generate_return(c, node->as.ret);
    }
    else if (node->type == AST_CALL_EXPR) {
        generate_call_expr(c, node->as.call);
    }
    else if (node->type == AST_BINARY) {
        generate_binary_expr(c, node->as.binary);
    }
    else if (node->type == AST_LITERAL_INT) {
        generate_lit_int(c, node->as.lit_int);
    }
    else if (node->type == AST_INLINE_ASM_BLOCK) {
        generate_inline_asm(c, node->as.asm_inl);
    }
    else if (node->type == AST_VARIABLE_DECLARATION) {
        generate_variable_declaration(c, node->as.var_dec);
    }
    else if (node->type == AST_IDENTIFIER) {
        int offset = symbol_table_lookup(c->symbol_table, node->as.ident->name);
        put(c, "mov rax, qword [rbp%d]", offset);
    }
    else if (node->type == AST_ASSIGNMENT) {
        generate_assignment(c, node->as.assign);
    }
    else if (node->type == AST_IF) {
        generate_if_statement(c, node->as.iff);
    }
    else if (node->type == AST_WHILE) {
        generate_while_statement(c, node->as.whilee);
    }
    else {
        printf("Unknown node in compiler.");
    }
}

static void asm_init(Compiler *c) {
    putf(c, "section .data");
    putf(c, "section .text");
    put(c, "global _start\n");

    putf(c, "_start:");
    call("main", c);
    put(c, "mov rdi, rax");
    put(c, "mov rax, 60");

    sys_call(c);
}

static int check_main(Compiler *c) {
    int has_entry_point = 0;
    for (int i = 0; i < c->node_count; i++) {
        if (c->tree[i]->type == AST_FUNCTION) {
            if (strcmp(c->tree[i]->as.func->identifier, "main") == 0) {
                has_entry_point = 1;
                break;
            }
        }
    }

    return has_entry_point;
}

void compile(Compiler *c) {
    const char* asmFile = "out.asm";
    const char* objectFile = "out.o";

    FILE *fptr = fopen(asmFile, "w");
    if (!fptr) {
        fprintf(stderr, "error: could not open file '%s'\n", asmFile);
        return;
    }
    c->file = fptr;

    int has_entry_point = check_main(c);
    if (!has_entry_point) {
        printf("'main' not defined.\n");
        return;
    }

    asm_init(c);

    for (int i = 0; i < c->node_count; i++) {
        generate_node(c, c->tree[i]);
    }

    fclose(c->file);

    char command[256];
    snprintf(command, sizeof(command), "nasm -f elf64 %s", asmFile);
    if (system(command) != 0) {
        fprintf(stderr, "NASM assembly failed.\n");
    }
    
    snprintf(command, sizeof(command), "ld %s -o %s", objectFile, c->exe);
    if (system(command) != 0) {
        fprintf(stderr, "Linking failed.\n");
    }

    if (!c->emitObj) {
        snprintf(command, sizeof(command), "rm %s", objectFile);
        system(command);
    }

    if (!c->emitAsm) {
        snprintf(command, sizeof(command), "rm %s", asmFile);
        system(command);
    }
}