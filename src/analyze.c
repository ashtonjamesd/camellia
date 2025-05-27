#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "analyze.h"
#include "symtab.h"
#include "utils.h"

static void analyze_node(AstNode *node, Analyzer *analyzer);

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
    analyzer->err = NO_ANALYZE_ERR;

    analyzer->node_count = count;
    analyzer->tree = tree;

    return analyzer;
}

void free_analyzer(Analyzer *analyzer) {
    if (!analyzer) return;
    free(analyzer);
}

static void err_undefined_identifier(char *name, Analyzer *analyzer) {
    analyzer->err = ANALYZE_ERR_REDEFINED_VARIABLE;
    printf("undefined identifier '%s'", name);
}

static void analyze_function(AstFunctionDeclaration *func, Analyzer *analyzer) {
    for (int i = 0; i < func->body_count; i++) {
        analyze_node(func->body[i], analyzer);
    }
}

static void add_variable_symbol(AstDeclarator *var_dec, Analyzer *analyzer) {
    if (analyzer->variable_symbols->count >= analyzer->variable_symbols->capacity) {
        analyzer->variable_symbols->capacity *= 2;
        analyzer->variable_symbols->symbols = realloc(analyzer->variable_symbols->symbols, sizeof(VariableSymbol *) * analyzer->variable_symbols->capacity);
    }
    
    VariableSymbol *symbol = malloc(sizeof(VariableSymbol));
    symbol->identifier = strdup(var_dec->identifier);

    analyzer->variable_symbols->symbols[analyzer->variable_symbols->count++] = symbol;
}

static VariableSymbol *get_variable_symbol(char *symbol, Analyzer *analyzer) {
    for (int i = 0; i < analyzer->variable_symbols->count; i++) {
        VariableSymbol *var = analyzer->variable_symbols->symbols[i];

        if (strcmp(var->identifier, symbol) == 0) {
            return var;
        }
    }

    return NULL;
}

static void err(AnalyzerErr err, Analyzer *analyzer) {
    analyzer->err = err;
}

static void analyze_assignment(AstAssignment *assign, Analyzer *analyzer) {
    int exists = 0;
    for (int i = 0; i < analyzer->variable_symbols->count; i++) {
        if (get_variable_symbol(assign->identifier, analyzer)) {
            exists = 1;
            break;
        }
    }

    if (!exists) {
        printf("Undefined identifier '%s'\n", assign->identifier);
        err(ANALYZE_ERR_UNDEFINED_IDENTIFIER, analyzer);
    }

    analyze_node(assign->value, analyzer);
}

static void analyze_variable_declaration(AstVariableDeclaration *var_dec, Analyzer *analyzer) {
    for (int i = 0; i < var_dec->declarator_count; i++) {
        if (get_variable_symbol(var_dec->declarators[i]->identifier, analyzer)) {
            printf("Redefinition of variable '%s'\n", var_dec->declarators[i]->identifier);
            err(ANALYZE_ERR_REDEFINED_VARIABLE, analyzer);
            return;
        }
    }

    for (int i = 0; i < var_dec->declarator_count; i++) {
        add_variable_symbol(var_dec->declarators[i], analyzer);
    }
}

static void analyze_identifier(AstIdentifier *ident, Analyzer *analyzer) {
    if (!get_variable_symbol(ident->name, analyzer)) {
        printf("Undefined identifier '%s'\n", ident->name);
        err(ANALYZE_ERR_UNDEFINED_IDENTIFIER, analyzer);
        return;
    }
}


static void analyze_node(AstNode *node, Analyzer *analyzer) {
    if (node->type == AST_VARIABLE_DECLARATION) {
        analyze_variable_declaration(node->as.var_dec, analyzer);
    }
    else if (node->type == AST_FUNCTION) {
        analyze_function(node->as.func, analyzer);
    }
    else if (node->type == AST_ASSIGNMENT) {
        analyze_assignment(node->as.assign, analyzer);
    }
    else if (node->type == AST_IDENTIFIER) {
        analyze_identifier(node->as.ident, analyzer);
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