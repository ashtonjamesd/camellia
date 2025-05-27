#ifndef ANALYZE_H
#define ANALYZE_H

#include "ast.h"
#include "symtab.h"

typedef enum {
    ANALYZE_ERR_REDEFINED_VARIABLE,
    ANALYZE_ERR_UNDEFINED_IDENTIFIER,
    NO_ANALYZE_ERR,
} AnalyzerErr;

typedef struct {
    AstNode        **tree;
    int              node_count;

    VariableSymbols *variable_symbols;
    FunctionSymbols *function_symbols;
    TypedefSymbols  *typedef_symbols;
    LabelSymbols    *label_symbols;

    AnalyzerErr      err;
} Analyzer;

extern Analyzer *init_analyzer(AstNode **tree, int count);
extern void free_analyzer(Analyzer *analyzer);
extern void analyze_ast(Analyzer *analyzer);

#endif