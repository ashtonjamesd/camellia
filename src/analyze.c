#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "analyze.h"
#include "symtab.h"
#include "utils.h"

void analyze_node(AstNode *node, Analyzer *analyzer);

Analyzer *init_analyzer(AstNode **tree, int count) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    if (!analyzer) {
        perror("Error allocating analyzer.");
        return NULL;
    }

    analyzer->variable_symbols = malloc(sizeof(VariableSymbols));
    analyzer->variable_symbols->capacity = 1;
    analyzer->variable_symbols->count = 0;
    analyzer->variable_symbols->symbols = malloc(sizeof(VariableSymbol *));

    analyzer->node_count = count;
    analyzer->tree = tree;

    return analyzer;
}

void free_analyzer(Analyzer *analyzer) {
    if (!analyzer) return;
    free(analyzer);
}

void analyze_function(AstFunctionDeclaration *func, Analyzer *analyzer) {
    for (int i = 0; i < func->count; i++) {
        analyze_node(func->body[i], analyzer);
    }
}

void add_variable_symbol(AstVariableDeclaration *var_dec, Analyzer *analyzer) {
    if (analyzer->variable_symbols->count >= analyzer->variable_symbols->capacity) {
        analyzer->variable_symbols->capacity *= 2;
        analyzer->variable_symbols->symbols = realloc(analyzer->variable_symbols->symbols, sizeof(VariableSymbol *) * analyzer->variable_symbols->capacity);
    }
    
    VariableSymbol *symbol = malloc(sizeof(VariableSymbol));
    symbol->identifier = strdup(var_dec->identifier);

    analyzer->variable_symbols->symbols[analyzer->variable_symbols->count++] = symbol;
}

VariableSymbol *get_variable_symbol(char *symbol, Analyzer *analyzer) {
    for (int i = 0; i < analyzer->variable_symbols->count; i++) {
        VariableSymbol *var = analyzer->variable_symbols->symbols[i];

        if (strcmp(var->identifier, symbol) == 0) {
            return var;
        }
    }

    return NULL;
}

void analyze_variable_declaration(AstVariableDeclaration *var_dec, Analyzer *analyzer) {
    if (get_variable_symbol(var_dec->identifier, analyzer)) {
        printf("Redefinition of variable '%s'\n", var_dec->identifier);
        return;
    }

    add_variable_symbol(var_dec, analyzer);
}

void analyze_node(AstNode *node, Analyzer *analyzer) {
    if (node->type == AST_VARIABLE_DECLARATION) {
        analyze_variable_declaration(node->as.var_dec, analyzer);
    }
    else if (node->type == AST_FUNCTION) {
        analyze_function(node->as.func, analyzer);
    }
    else {
        printf("Unknown node type '%s' in 'analyze_node'\n", ast_type_to_str(node->type));
    }
}

void analyze_ast(Analyzer *analyzer) {
    for (int i = 0; i < analyzer->node_count; i++) {
        analyze_node(analyzer->tree[i], analyzer);
    }

}