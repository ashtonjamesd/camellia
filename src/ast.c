#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ast.h"
#include "utils.h"

static AstNode *parse_statement(Parser *parser);

static void print_depth(int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
}

static void print_node(AstNode *node, int depth) {
    print_depth(depth);

    if (node->type == AST_LITERAL_INT) {
        printf("LITERAL INT: %d", node->as.lit_int->value);
    }
    else if (node->type == AST_LITERAL_CHAR) {
        printf("LITERAL CHAR: %c", node->as.lit_char->value);
    }
    else if (node->type == AST_VARIABLE_DECLARATION) {
        printf("VARIABLE DECLARATION:\n");
    
        print_depth(depth);
        printf("IDENTIFIER: %s\n", node->as.var_dec->identifier);
        
        if (node->as.var_dec->value) {
            print_node(node->as.var_dec->value, depth);
        }
    }
    else if (node->type == AST_FUNCTION) {
        printf("FUNCTION:\n");

        print_depth(depth + 2);
        printf("IDENTIFIER: %s\n", node->as.func->identifier);
        
        print_depth(depth + 2);
        printf("RETURN TYPE: %s\n", data_type_to_str(node->as.func->returnType));
        print_depth(depth + 2);
        printf("Body (%d):\n", node->as.func->count);
        for (int i = 0; i < node->as.func->count; i++) {
            print_node(node->as.func->body[i], depth + 4);
        }
    }
    else if (node->type == AST_RETURN) {
        printf("RETURN: \n");
        print_node(node->as.ret->value, depth + 1);
    }
    else if (node->type == AST_BINARY) {
        printf("BINARY EXPR:\n");
        print_node(node->as.binary->left, depth + 2);

        print_depth(depth + 2);
        printf("OP: %s\n", node->as.binary->op.lexeme);

        print_node(node->as.binary->right, depth + 2);
    }
    else if (node->type == AST_IDENTIFIER) {
        printf("IDENTIFIER:\n");
        print_depth(depth + 2);
        printf("VALUE: %s\n", node->as.ident->name);
    }
    else if (node->type == AST_CALL_EXPR) {
        printf("CALL EXPR:\n");
        print_depth(depth + 2);
        printf("FUNC: %s\n", node->as.call->identifier);
    }
    else {
        printf("UNKNOWN NODE TYPE: %d", node->type);
    }
    printf("\n");
}

static char *parser_err_to_str(ParseErr err) {
    switch (err) {
        case NO_PARSER_ERROR: return "No parser error.";
        case PARSE_ERR_EXPECTED_EXPRESSION: return "Expected Expression.";
        case PARSE_ERR_EXPECTED_IDENTIFIER: return "Expected Identifier.";
        case PARSE_ERR_EXPECTED_SEMICOLON: return "Expected Semicolon.";
        case PARSE_ERR_VOID_NOT_ALLOWED: return "Void not allowed here.";
        default: return "UNKNOWN ERROR";
    }
}

void parser_print(Parser *parser) {
  printf("\n\nPARSER SUCCESS\n");
  printf("AST Nodes (%d):\n", parser->node_count);
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
    else if (node->type == AST_LITERAL_CHAR) {
        free(node->as.lit_char);
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
    else if (node->type == AST_IDENTIFIER) {
        free(node->as.ident->name);
        free(node->as.ident);
        free(node);
    }
    else if (node->type == AST_BINARY) {
        free_node(node->as.binary->left);
        free_node(node->as.binary->right);
        free(node->as.binary);
        free(node);
    }
    else if (node->type == AST_CALL_EXPR) {
        free(node->as.call->identifier);
        free(node->as.call);
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
    else if (type == AST_LITERAL_CHAR) {
        node->as.lit_char = (AstLiteralChar *)value;
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
    else if (type == AST_IDENTIFIER) {
        node->as.ident = (AstIdentifier *)value;
    }
    else if (type == AST_BINARY) {
        node->as.binary = (AstBinaryExpr *)value;
    }
    else if (type == AST_CALL_EXPR) {
        node->as.call = (AstCallExpr *)value;
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

static inline void recede(Parser *parser) {
    parser->current--;
}

static inline int is_end(Parser *parser) {
    return current_token(parser).type == TOKEN_EOF;
}

static inline int match(TokenType type, Parser *parser) {
    return current_token(parser).type == type;
}

static inline void *parser_err(ParseErr err, Parser *parser) {
    parser->err = err;
    return NULL;
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

    return 0;
}

static AstCallExpr *init_call_expr(char *identifier) {
    AstCallExpr *expr = malloc(sizeof(AstCallExpr));
    expr->identifier = strdup(identifier);

    return expr;
}

static AstNode *parse_primary(Parser *parser) {
    Token token = current_token(parser);
    advance(parser);

    if (token.type == TOKEN_INTEGER_LITERAL) {
        AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
        lit->value = atoi(token.lexeme);
        AstNode *node = init_node(lit, AST_LITERAL_INT);

        return node;
    }
    else if (token.type == TOKEN_CHAR_LITERAL) {
        AstLiteralChar *lit = malloc(sizeof(AstLiteralChar));
        lit->value = token.lexeme[0];
        AstNode *node = init_node(lit, AST_LITERAL_CHAR);

        return node;

    }
    else if (token.type == TOKEN_IDENTIFIER) {
        char *name = strdup(token.lexeme);

        if (match(TOKEN_LEFT_PAREN, parser)) {
            advance(parser);
            
            if (!expect(TOKEN_RIGHT_PAREN, parser)) {
                return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
            }
        
            AstCallExpr *expr = init_call_expr(name);
            AstNode *node = init_node(expr, AST_CALL_EXPR);

            return node;
        }

        AstIdentifier *ident = malloc(sizeof(AstIdentifier));
        ident->name = name;
        AstNode *node = init_node(ident, AST_IDENTIFIER);

        return node;
    }

    return NULL;
}

static AstBinaryExpr *init_binary_node(AstNode *left, Token op, AstNode *right) {
    AstBinaryExpr *binary = malloc(sizeof(AstBinaryExpr));
    binary->left = left;
    binary->op = op;
    binary->right = right;

    return binary;
}

static AstNode *parse_factor(Parser *parser) {
    AstNode *left = parse_primary(parser);
    
    while (match(TOKEN_STAR, parser) || match(TOKEN_SLASH, parser) || match(TOKEN_MODULO, parser)) {
        Token op = current_token(parser);
        advance(parser);
        
        AstNode *right = parse_factor(parser);
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_term(Parser *parser) {
    AstNode *left = parse_factor(parser);

    while (match(TOKEN_PLUS, parser) || match(TOKEN_MINUS, parser)) {
        Token op = current_token(parser);
        advance(parser);
        
        AstNode *right = parse_factor(parser);
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_expression(Parser *parser) {
    return parse_term(parser);
}

static AstVariableDeclaration *init_var_dec(char* identifier, AstNode *literal) {
    AstVariableDeclaration *var_dec = (AstVariableDeclaration *)malloc(sizeof(AstVariableDeclaration));
    var_dec->identifier = strdup(identifier);
    var_dec->type = AST_TYPE_INT;
    var_dec->value = literal;

    return var_dec;
}

static AstNode *parse_var_dec(Parser *parser) {
    Token type_token = current_token(parser);
    advance(parser);

    Token id_token = current_token(parser);
    if (!expect(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);

        AstVariableDeclaration *var_dec = init_var_dec(id_token.lexeme, NULL);
        AstNode *node = init_node(var_dec, AST_VARIABLE_DECLARATION);

        return node;
    }

    if (!expect(TOKEN_SINGLE_EQUALS, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }
    if (!is_literal(parser)) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    AstNode *literal = parse_expression(parser);
    if (!literal) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        free_node(literal);
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }
    
    AstVariableDeclaration *var_dec = init_var_dec(id_token.lexeme, literal);
    AstNode *node = init_node(var_dec, AST_VARIABLE_DECLARATION);

    return node;
}

static AstFunctionDeclaration *init_function_node(AstNode **body, int count, char *identifier, AstDataType returnType) {
    AstFunctionDeclaration *func = (AstFunctionDeclaration *)malloc(sizeof(AstFunctionDeclaration));
    func->body = body;
    func->count = count;
    func->identifier = strdup(identifier);
    func->returnType = returnType;

    return func;
}

static AstDataType token_to_ast_data_type(Token token) {
    switch (token.type) {
        case TOKEN_INT: return AST_TYPE_INT;
        case TOKEN_CHAR: return AST_TYPE_CHAR;
        case TOKEN_VOID: return AST_TYPE_VOID;
    }

    return AST_TYPE_INVALID;
}

static AstNode *parse_function(Parser *parser) {
    Token return_type_token = current_token(parser);
    AstDataType type = token_to_ast_data_type(return_type_token);
    if (type == AST_TYPE_INVALID) return NULL;

    advance(parser);

    Token identifier_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) return NULL;
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) return NULL;
    if (!expect(TOKEN_RIGHT_PAREN, parser)) return NULL;

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);

        AstFunctionDeclaration *func = init_function_node(NULL, 0, identifier_token.lexeme, type);
        AstNode *node = init_node(func, AST_FUNCTION);

        return node;
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) return NULL;

    int body_statement_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *) * body_capacity);
    
    while (!match(TOKEN_RIGHT_BRACE, parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) return NULL;

        if (body_statement_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, sizeof(AstNode *) * body_capacity);
        }
        body[body_statement_count++] = node;
    }

    AstFunctionDeclaration *func = init_function_node(body, body_statement_count, identifier_token.lexeme, type);
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

    AstNode *return_expr = parse_expression(parser);
    if (!return_expr) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    printf("%s", current_token(parser).lexeme);
    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstReturn *ret = init_return(return_expr);
    AstNode *node = init_node(ret, AST_RETURN);

    return node;
}

static AstNode *parse_type_statement(Parser *parser) {
    advance(parser);

    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    if (match(TOKEN_LEFT_PAREN, parser)) {
        recede(parser);
        recede(parser);

        return parse_function(parser);
    }
    else if (match(TOKEN_SINGLE_EQUALS, parser)) {
        recede(parser);
        recede(parser);

        if (match(TOKEN_VOID, parser)) {
            return parser_err(PARSE_ERR_VOID_NOT_ALLOWED, parser);
        }

        return parse_var_dec(parser);
    }
    else if (match(TOKEN_SEMICOLON, parser)) {
        recede(parser);
        recede(parser);

        if (match(TOKEN_VOID, parser)) {
            return parser_err(PARSE_ERR_VOID_NOT_ALLOWED, parser);
        }

        return parse_var_dec(parser);
    }
    else {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }
}

static AstNode *parse_statement(Parser *parser) {
    if (token_to_ast_data_type(current_token(parser)) != AST_TYPE_INVALID) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_RETURN, parser)) {
        return parse_return(parser);
    }
    else {
        AstNode *expr = parse_expression(parser);
        if (!expr) return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);

        if (!expect(TOKEN_SEMICOLON, parser)) {
            free_node(expr);
            return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
        }

        return expr;
    }
}

void parse_ast(Parser *parser) {
    while (!is_end(parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) {
            break;
        }
        
        if (parser->node_count >= parser->node_capacity) {
            parser->node_capacity *= 2;
            parser->tree = realloc(parser->tree, sizeof(AstNode *) * parser->node_capacity);
        }
    
        parser->tree[parser->node_count++] = node;

        if (is_end(parser)) break;
    }

    if (parser->err != NO_PARSER_ERROR) {
        printf("%s\n", parser_err_to_str(parser->err));
    }

    if (parser->debug) parser_print(parser);
}