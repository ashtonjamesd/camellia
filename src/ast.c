#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"

static AstNode *parse_statement(Parser *parser);

static char* ast_type_to_str(AstType type) {
    switch (type) {
        case AST_VARIABLE_DECLARATION: return "VARIABLE DECLARATION";
        case AST_LITERAL_INT: return "LITERAL INT";
        case AST_LITERAL_CHAR: return "LITERAL CHAR";
        case AST_FUNCTION: return "FUNCTION";
        default: return "UNKNOWN TYPE";
    }
}

static char *data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_CHAR: return "CHAR";
        case TYPE_INT: return "INT";
        case TYPE_VOID: return "VOID";
        default: return "UNKNOWN TYPE";
    }
}

static void print_node(AstNode *node, int depth) {
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
    else if (node->type == AST_FUNCTION) {
        printf("FUNCTION:\n");
        printf("  IDENTIFIER: %s\n", node->as.func->identifier);
        printf("  RETURN TYPE: %s\n", data_type_to_str(node->as.func->returnType));
        printf("  Body (%d):\n", node->as.func->count);
        for (int i = 0; i < node->as.func->count; i++) {
            print_node(node->as.func->body[i], depth + 2);
        }
    }
    else if (node->type == AST_RETURN) {
        printf("RETURN: \n");
        print_node(node->as.ret->value, depth + 1);
    }
    else {
        printf("UNKNOWN NODE TYPE: %d", node->type);
    }
    printf("\n");
}

static char *parser_err_to_str(ParseErr err) {
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

Parser *init_parser(Token *tokens, int debug) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    if (!parser) {
        perror("Error allocating parser.");
        return NULL;
    }

    parser->node_count = 0;
    parser->node_capacity = 1;
    parser->debug = debug;
    parser->tree = malloc(sizeof(AstNode *) * parser->node_capacity);
    parser->tokens = tokens;
    parser->err = NO_PARSER_ERROR;
    parser->current = 0;

    return parser;
}

static void free_node(AstNode *node) {
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
    else if (node->type == AST_FUNCTION) {
        free(node->as.func->identifier);
        for (int i = 0; i < node->as.func->count; i++) {
            free_node(node->as.func->body[i]);
        }
        free(node->as.func->body);
        free(node->as.func);
        free(node);
    }
    else if (node->type == AST_RETURN) {
        free_node(node->as.ret->value);
        free(node->as.ret);
        free(node);
    }
    else {
        printf("Unknown AstType in 'free_node': '%s'\n", ast_type_to_str(node->type));
        printf("You probably forgot to add this type to the if-else block.\n");
    }
}

void free_parser(Parser *parser) {
    for (int i = 0; i < parser->node_count; i++) {
        free_node(parser->tree[i]);
    }

    free(parser->tree);
    free(parser);
}

static AstNode *init_node(void *value, AstType type){
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node) {
        perror("Error allocating node.");
        return NULL;
    }

    node->type = type;

    if (type == AST_LITERAL_INT) {
        node->as.lit_int = (AstLiteralInt *)value;
    }
    else if (type == AST_VARIABLE_DECLARATION) {
        node->as.var_dec = (AstVariableDeclaration *)value;
    }
    else if (type == AST_FUNCTION) {
        node->as.func = (AstFunctionDeclaration *)value;
    }
    else if (type == AST_RETURN) {
        node->as.ret = (AstReturn *)value;
    }
    else {
        printf("Unknown AstType in 'init_node': '%s'\n", ast_type_to_str(type));
        printf("You probably forgot to add this type to the if-else block.\n");
    }

    return node;
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

static AstFunctionDeclaration *init_function_node(AstNode **body, int count, char *identifier, DataType returnType) {
    AstFunctionDeclaration *func = (AstFunctionDeclaration *)malloc(sizeof(AstFunctionDeclaration));
    func->body = body;
    func->count = count;
    func->identifier = strdup(identifier);
    func->returnType = returnType;

    return func;
}

static AstNode *parse_function(Parser *parser) {
    advance(parser);

    Token function_name_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) return NULL;
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) return NULL;
    if (!expect(TOKEN_RIGHT_PAREN, parser)) return NULL;

    if (!expect(TOKEN_LEFT_BRACE, parser)) return NULL;

    int count = 0;
    int capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *) * capacity);
    
    while (!match(TOKEN_RIGHT_BRACE, parser)) {
        AstNode *node = parse_statement(parser);
        body[count++] = node;
    }

    AstFunctionDeclaration *func = init_function_node(body, count, function_name_token.lexeme, TYPE_VOID);
    AstNode *node = init_node(func, AST_FUNCTION);

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        // free some shit here
        return NULL;
    }

    return node;
}

static AstReturn *init_return(AstNode *value) {
    AstReturn *ret = (AstReturn *)malloc(sizeof(AstReturn));
    ret->value = value;

    return ret;
}

static AstNode *parse_return(Parser *parser) {
    advance(parser);

    AstNode *return_item = parse_literal(parser);

    AstReturn *ret = init_return(return_item);
    AstNode *node = init_node(ret, AST_RETURN);

    if (!expect(TOKEN_SEMICOLON, parser)) return NULL;

    return node;
}

static AstNode *parse_statement(Parser *parser) {
    if (match(TOKEN_INT, parser)) {
        return parse_var_dec(parser);
    }
    else if (match(TOKEN_INTEGER_LITERAL, parser)) {
        return parse_literal(parser);
    }
    else if (match(TOKEN_VOID, parser)) {
        return parse_function(parser);
    }
    else if (match(TOKEN_RETURN, parser)) {
        return parse_return(parser);
    }
    else {
        return NULL;
    }
}

void parse_ast(Parser *parser) {
    while (!is_end(parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) break;
        
        if (parser->node_count >= parser->node_capacity) {
            parser->node_capacity *= 2;
            parser->tree = realloc(parser->tree, sizeof(AstNode *) * parser->node_capacity);
        }
    
        parser->tree[parser->node_count++] = node;

        if (is_end(parser)) break;
        advance(parser);
    }

    parser_print(parser);
}