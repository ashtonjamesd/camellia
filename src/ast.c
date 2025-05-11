#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"


static char* ast_type_to_str(AstType type) {
    switch (type) {
        case AST_VARIABLE_DECLARATION: return "VARIABLE DECLARATION";
        case AST_LITERAL_INT: return "LITERAL INT";
        case AST_LITERAL_CHAR: return "LITERAL CHAR";
        default: return "UNKNOWN TYPE";
    }
}


void print_node(AstNode *node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    if (node->type == AST_LITERAL_INT) {
        printf("LITERAL: %d", node->as.lit_int->value);
    }
    else if (node->type == AST_VARIABLE_DECLARATION) {
        printf("VARIABLE DECLARATION:\n");
        printf("  IDENTIFIER: %s\n", node->as.var_dec->identifier);
        if (node->as.var_dec->value) {
            print_node(node->as.var_dec->value, depth + 1);
        }
    }
    else {
        printf("UNKNOWN NODE TYPE: %d", node->type);
    }
    printf("\n");
}

static AstNode *init_node(void *value, AstType type){
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->type = type;

    if (type == AST_LITERAL_INT) {
        node->as.lit_int = (AstLiteralInt *)value;
    }
    else if (type == AST_VARIABLE_DECLARATION) {
        node->as.var_dec = (AstVariableDeclaration *)value;
    }
    else {
        printf("Unknown expression in 'init_node': %s\n", ast_type_to_str(type));
        printf("You probably forgot to add this type to the if-else block.");
    }

    return node;
}

Parser *init_parser(Token *tokens, int debug) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->node_count = 0;
    parser->node_capacity = 1;
    parser->debug = debug;
    parser->tree = malloc(sizeof(AstNode *) * parser->node_capacity);
    parser->tokens = tokens;
    parser->err = NO_PARSER_ERROR;
    parser->current = 0;

    return parser;
}

void free_node(AstNode *node) {
    if (!node) return;

    if (node->type == AST_LITERAL_INT) {
        free(node->as.lit_int);
        free(node);
    }
    else if (node->type == AST_VARIABLE_DECLARATION) {
        free(node->as.var_dec->identifier);
        free_node(node->as.var_dec->value);
        free(node->as.var_dec);
        free(node);
    }

}

void free_parser(Parser *parser) {
    for (int i = 0; i < parser->node_count; i++) {
        free_node(parser->tree[i]);
    }

    free(parser->tree);
    free(parser);
}

char *parser_err_to_str(ParseErr err) {
    switch (err) {
        case NO_PARSER_ERROR: return "No parser error.";
        default: return "UNKNOWN ERROR";
    }
}

void parser_print(Parser *parser) {
    printf("\n\nPARSER OUTPUT (%d)\n", parser->node_count);
    printf("Error: %s\n\n", parser_err_to_str(parser->err));

    printf("AST Nodes:\n");
    for (int i = 0; i < parser->node_count; i++) {
        AstNode *node = parser->tree[i];
        print_node(node, 1);
    }
    printf("\n");
}

static inline Token current_token(Parser *parser) {
    return parser->tokens[parser->current];
}

static inline void advance(Parser *parser) {
    parser->current++;
}

static inline int is_end(Parser *parser) {
    return current_token(parser).type == TOKEN_EOF;
}

static inline int match(TokenType type, Parser *parser) {
    return current_token(parser).type == type;
}

static int is_literal(Parser *parser) {
    return match(TOKEN_INTEGER_LITERAL, parser)
        || match(TOKEN_CHAR_LITERAL, parser)
        || match(TOKEN_FLOAT_LITERAL, parser)
        || match(TOKEN_OCTAL_LITERAL, parser)
        || match(TOKEN_BINARY_LITERAL, parser)
        || match(TOKEN_STRING_LITERAL, parser)
        || match(TOKEN_HEX_LITERAL, parser);
}

static inline int expect(TokenType type, Parser *parser) {
    if (parser->tokens[parser->current].type == type) {
        advance(parser);
        return 1;
    }

    printf("expected %s, got %s", token_type_to_str(type), token_type_to_str(parser->tokens[parser->current].type));

    return 0;
}

static AstNode *parse_literal(Parser *parser) {
    Token token = current_token(parser);

    AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
    lit->value = atoi(token.lexeme);

    AstNode *node = init_node(lit, AST_LITERAL_INT);

    advance(parser);

    return node;
}

static AstVariableDeclaration *init_var_dec(char* identifier, AstNode *literal) {
    AstVariableDeclaration *var_dec = (AstVariableDeclaration *)malloc(sizeof(AstVariableDeclaration));
    var_dec->identifier = strdup(identifier);
    var_dec->type = TYPE_INT;
    var_dec->value = literal;

    return var_dec;
}

static AstNode *parse_var_dec(Parser *parser) {
    Token type_token = current_token(parser);
    advance(parser);

    Token id_token = current_token(parser);
    if (!expect(TOKEN_IDENTIFIER, parser)) return NULL;

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);

        AstVariableDeclaration *var_dec = init_var_dec(id_token.lexeme, NULL);
        AstNode *node = init_node(var_dec, AST_VARIABLE_DECLARATION);

        return node;
    }

    if (!expect(TOKEN_SINGLE_EQUALS, parser)) return NULL;
    if (!is_literal(parser)) return NULL;

    AstNode *literal = parse_literal(parser);
    if (!literal) return NULL;

    if (!expect(TOKEN_SEMICOLON, parser)) {
        free_node(literal);
        return NULL;
    }
    
    AstVariableDeclaration *var_dec = init_var_dec(id_token.lexeme, literal);
    AstNode *node = init_node(var_dec, AST_VARIABLE_DECLARATION);

    return node;
}

static AstNode* parse_statement(Parser *parser) {
    if (current_token(parser).type == TOKEN_INT) {
        return parse_var_dec(parser);
    }
    else if (current_token(parser).type == TOKEN_INTEGER_LITERAL) {
        return parse_literal(parser);
    }

    return NULL;
}

static void add_node(AstNode *node, Parser *parser) {
    if (parser->node_count >= parser->node_capacity) {
        parser->node_capacity *= 2;
        parser->tree = realloc(parser->tree, sizeof(AstNode *) * parser->node_capacity);
    }

    parser->tree[parser->node_count++] = node;
}

void parse_ast(Parser *parser) {
    while (!is_end(parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) break;
        
        add_node(node, parser);

        if (is_end(parser)) break;
        advance(parser);
    }

    parser_print(parser);
}