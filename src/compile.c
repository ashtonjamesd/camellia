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
    vfprintf(c->file, format, args);
    fprintf(c->file, "\n");
    va_end(args);
}

static inline void sys_call(Compiler *c) {
    put(c, "  int 80h");
}

static inline void call(char *func, Compiler *c) {
    put(c, "  call %s", func);
}

void generate_return(Compiler *c, AstReturn *ret) {
    
}

void generate_function(Compiler *c, AstFunctionDeclaration *func) {
    fprintf(c->file, "\n%s:\n", func->identifier);

    for (int i = 0; i < func->count; i++) {
        generate_node(c, func->body[i]);
    }

    put(c, "  ret"); 
}

void generate_node(Compiler *c, AstNode *node) {
    if (node->type == AST_FUNCTION) {
        generate_function(c, node->as.func);
    }
    else if (node->type == AST_RETURN) {
        generate_return(c, node->as.ret);
    }
}

void asm_init(Compiler *c) {
    put(c, "section .data");
    put(c, "section .text");
    put(c, "global _start\n");

    put(c, "_start:");
    call("main", c);
    put(c, "  mov eax, 1");
    put(c, "  mov ebx, 1");
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
}