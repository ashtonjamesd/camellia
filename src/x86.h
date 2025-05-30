#ifndef COMPILE_H
#define COMPILE_H

#include "ast.h"

typedef struct {
    char *name;
    int offset;
} Symbol;

typedef struct {
    Symbol *symbols;
    int count;
    int capacity;
} SymbolTable;

typedef struct {
    AstNode **tree;
    int       node_count;
    FILE     *file;

    // if 1, will not remove the compiler asm output
    int       emitAsm;

    // if 1, will not remove the compiler object file output
    int       emitObj;

    // represents the name of the .exe produced
    char     *exe;

    SymbolTable *symbol_table;
} Compiler;

extern Compiler *init_compiler(AstNode **tree, int count, char *exe, int emitAsm, int emitObj);
extern void free_compiler(Compiler *compiler);
extern void compile(Compiler *compiler);

#endif