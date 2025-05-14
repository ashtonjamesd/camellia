#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>

#include "compile.h"

void generate_node(Compiler *c, AstNode *node);

Compiler *init_compiler(AstNode **tree, int count) {
    Compiler *c = malloc(sizeof(Compiler));
    if (!c) {
        perror("Error allocating compiler.");
        return NULL;
    }
    
    c->tree = tree;
    c->node_count = count;

    return c;
}

void free_compiler(Compiler *c) {
    if (!c) return;
    free(c);
}

void put(Compiler *c, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(c->file, "  ");
    vfprintf(c->file, format, args);
    fprintf(c->file, "\n");
    va_end(args);
}

void putf(Compiler *c, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(c->file, format, args);
    fprintf(c->file, "\n");
    va_end(args);
}

static inline void sys_call(Compiler *c) {
    put(c, "int 0x80");
}

static inline void call(char *func, Compiler *c) {
    put(c, "call %s", func);
}

void generate_return(Compiler *c, AstReturn *ret) {
    if (ret->value->type == AST_LITERAL_INT) {
        put(c, "mov eax, %d", ret->value->as.lit_int->value); 
    }
    else if (ret->value->type == AST_LITERAL_CHAR) {
        put(c, "mov eax, %d", ret->value->as.lit_int->value); 
    }
    else if (ret->value->type == AST_CALL_EXPR) {
        put(c, "call %s", ret->value->as.call->identifier); 
    }
    else if (ret->value->type == AST_BINARY) {
        generate_node(c, ret->value);
    }
}

void generate_function(Compiler *c, AstFunctionDeclaration *func) {
    fprintf(c->file, "\n%s:\n", func->identifier);

    for (int i = 0; i < func->count; i++) {
        generate_node(c, func->body[i]);
    }

    put(c, "ret"); 
}

void generate_call_expr(Compiler *c, AstCallExpr *call_expr) {
    call(call_expr->identifier, c);
}

void generate_binary_expr(Compiler *c, AstBinaryExpr *binary) {
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

void generate_lit_int(Compiler *c, AstLiteralInt *lit) {
    put(c, "mov rax, %d", lit->value);
}

void generate_node(Compiler *c, AstNode *node) {
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
}

void asm_init(Compiler *c) {
    putf(c, "section .data");
    putf(c, "section .text");
    put(c, "global _start\n");

    putf(c, "_start:");
    call("main", c);
    put(c, "mov ebx, eax");
    put(c, "mov eax, 1");
    sys_call(c);
}

int check_main(Compiler *c) {
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
    struct stat st = {0};
    if (stat("out/", &st) == -1) {
        mkdir("out/", 0777);
    }
    FILE *fptr = fopen("out/out.asm", "w");
    if (!fptr) {
        perror("Error opening file");
        return;
    }
    c->file = fptr;

    int has_entry_point = check_main(c);
    if (!has_entry_point) {
        printf("'main' function not defined.");
        return;
    }

    asm_init(c);

    for (int i = 0; i < c->node_count; i++) {
        generate_node(c, c->tree[i]);
    }

    fclose(c->file);
    if (system("nasm -f elf64 out/out.asm") != 0) {
        fprintf(stderr, "NASM assembly failed.\n");
    }
    if (system("ld out/out.o -o out/main") != 0) {
        fprintf(stderr, "Linking failed.\n");
    }
    system("./out/main");
    system("rm out/out.o");
    // system("rm out/out.asm");
}