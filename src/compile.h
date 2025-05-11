#ifndef COMPILE_H
#define COMPILE_H

#include "ast.h"

typedef struct {
    AstNode **tree;
    int       node_count;
    FILE     *file;
} Compiler;

extern Compiler *init_compiler(AstNode **tree, int count);
extern void free_compiler(Compiler *compiler);
extern void compile(Compiler *compiler);

#endif