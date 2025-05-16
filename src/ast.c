#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "ast.h"
#include "utils.h"

static AstNode *parse_statement(Parser *parser);

Parser *init_parser(Token *tokens, int debug, char *file) {
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
    parser->file = file;

    return parser;
}

static void print_depth(int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
}

static void print_node(AstNode *node, int depth) {
    print_depth(depth);

    switch (node->type) {
        case AST_LITERAL_INT:
            printf("LITERAL INT: %d\n", node->as.lit_int->value);
            break;

        case AST_LITERAL_CHAR:
            printf("LITERAL CHAR: '%c'\n", node->as.lit_char->value);
            break;

        case AST_VARIABLE_DECLARATION:
            printf("VARIABLE DECLARATION:\n");
            print_depth(depth + 1);
            printf("IDENTIFIER: %s\n", node->as.var_dec->identifier);
            if (node->as.var_dec->value) {
                print_depth(depth + 1);
                printf("VALUE:\n");
                print_node(node->as.var_dec->value, depth + 2);
            }
            break;

        case AST_INLINE_ASM_BLOCK:
            printf("INLINE ASM (%d lines):\n", node->as.asm_inl->line_count);
            for (int i = 0; i < node->as.asm_inl->line_count; i++) {
                print_depth(depth + 1);
                printf("%s\n", node->as.asm_inl->lines[i]);
            }
            break;

        case AST_FUNCTION:
            printf("FUNCTION:\n");
            print_depth(depth + 1);
            printf("IDENTIFIER: %s\n", node->as.func->identifier);
            print_depth(depth + 1);
            printf("RETURN TYPE: %s\n", data_type_to_str(node->as.func->returnType));

            print_depth(depth + 1);
            printf("PARAMETERS (%d):\n", node->as.func->params_count);
            for (int i = 0; i < node->as.func->params_count; i++) {
                print_depth(depth + 2);
                printf("ID: %s\n", node->as.func->params[i]->name);
                print_depth(depth + 2);
                printf("TYPE: %s\n", data_type_to_str(node->as.func->params[i]->type));
                print_depth(depth + 2);
                printf("CONST: %d\n", node->as.func->params[i]->constant);
            }

            print_depth(depth + 1);
            printf("BODY (%d):\n", node->as.func->body_count);
            for (int i = 0; i < node->as.func->body_count; i++) {
                print_node(node->as.func->body[i], depth + 2);
            }
            break;

        case AST_RETURN:
            printf("RETURN:\n");
            print_node(node->as.ret->value, depth + 1);
            break;

        case AST_BINARY:
            printf("BINARY EXPRESSION:\n");
            print_depth(depth + 1);
            printf("LEFT:\n");
            print_node(node->as.binary->left, depth + 2);
            print_depth(depth + 1);
            printf("OPERATOR: %s\n", node->as.binary->op.lexeme);
            print_depth(depth + 1);
            printf("RIGHT:\n");
            print_node(node->as.binary->right, depth + 2);
            break;

        case AST_UNARY:
            printf("UNARY EXPRESSION:\n");
            print_depth(depth + 1);
            printf("LEFT:\n");
            print_node(node->as.unary->left, depth + 2);
            print_depth(depth + 1);
            printf("OPERATOR: %s\n", node->as.unary->op.lexeme);
            break;

        case AST_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->as.ident->name);
            break;

        case AST_CALL_EXPR:
            printf("CALL EXPRESSION:\n");
            print_depth(depth + 1);
            printf("FUNCTION: %s\n", node->as.call->identifier);
            break;

        case AST_ASSIGNMENT:
            printf("ASSIGNMENT:\n");
            print_depth(depth + 1);
            printf("IDENTIFIER: %s\n", node->as.assign->identifier);
            print_depth(depth + 1);
            printf("VALUE:\n");
            print_node(node->as.assign->value, depth + 2);
            break;

        case AST_IF:
            printf("IF STATEMENT:\n");
            print_depth(depth + 1);
            printf("CONDITION:\n");
            print_node(node->as.iff->condition, depth + 2);
            print_depth(depth + 2);
            printf("BODY (%d):\n", node->as.iff->body_count);
            for (int i = 0; i < node->as.iff->body_count; i++) {
                print_node(node->as.iff->body[i], depth + 3);
            }
            if (node->as.iff->else_body) {
                print_depth(depth);
                printf("ELSE:\n");
                print_depth(depth + 1);
                printf("BODY (%d):\n", node->as.iff->else_body_count);
                for (int i = 0; i < node->as.iff->else_body_count; i++) {
                    print_node(node->as.iff->else_body[i], depth + 2);
                }
            }
            break;

        case AST_WHILE:
            printf("WHILE STATEMENT:\n");
            print_depth(depth + 1);
            printf("CONDITION:\n");
            print_node(node->as.whilee->condition, depth + 2);
            print_depth(depth + 2);
            printf("BODY (%d):\n", node->as.whilee->body_count);
            for (int i = 0; i < node->as.whilee->body_count; i++) {
                print_node(node->as.whilee->body[i], depth + 3);
            }

        default:
            printf("UNKNOWN NODE TYPE: '%s'\n", ast_type_to_str(node->type));
            break;
    }
}

static char *parser_err_to_str(ParseErr err) {
    switch (err) {
        case NO_PARSER_ERROR: return "no parser error";
        case PARSE_ERR_EXPECTED_EXPRESSION: return "expected expression";
        case PARSE_ERR_EXPECTED_IDENTIFIER: return "expected identifier";
        case PARSE_ERR_EXPECTED_SEMICOLON: return "expected semicolon";
        case PARSE_ERR_VOID_NOT_ALLOWED: return "void not allowed here";
        case PARSE_ERR_INVALID_SYNTAX: return "invalid syntax";
        case PARSE_ERR_OUT_OF_MEMORY: return "out of memory";
        default: return "unknown error, ummm oops";
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
        for (int i = 0; i < node->as.func->body_count; i++) {
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
    else if (node->type == AST_UNARY) {
        free(node->as.unary->left);
        free(node->as.unary);
        free(node);
    }
    else if (node->type == AST_CALL_EXPR) {
        free(node->as.call->identifier);
        free(node->as.call);
        free(node);
    }
    else if (node->type == AST_INLINE_ASM_BLOCK) {
        for (size_t i = 0; i < node->as.asm_inl->line_count; i++) {
            free(node->as.asm_inl->lines[i]);
        }

        free(node->as.asm_inl);
        free(node);
    }
    else if (node->type == AST_ASSIGNMENT) {
        free_node(node->as.assign->value);
        free(node->as.assign->identifier);
        free(node->as.assign);
        free(node);
    }
    else if (node->type == AST_IF) {
        for (int i = 0; i < node->as.iff->body_count; i++) {
            free_node(node->as.iff->body[i]);
        }

        free(node->as.iff);
        free(node);
    }
    else if (node->type == AST_WHILE) {
        for (int i = 0; i < node->as.whilee->body_count; i++) {
            free_node(node->as.whilee->body[i]);
        }

        free_node(node->as.whilee->condition);

        free(node->as.whilee);
        free(node);
    }
    else if (node->type == AST_CONTINUE) {
        free(node->as.cont);
        free(node);
    }
    else if (node->type == AST_BREAK) {
        free(node->as.brk);
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
    else if (type == AST_INLINE_ASM_BLOCK) {
        node->as.asm_inl = (AstInlineAsmBlock *)value;
    }
    else if (type == AST_ASSIGNMENT) {
        node->as.assign = (AstAssignment *)value;
    }
    else if (type == AST_IF) {
        node->as.iff = (AstIfStatement *)value;
    }
    else if (type == AST_WHILE) {
        node->as.whilee = (AstWhile *)value;
    }
    else if (type == AST_BREAK) {
        node->as.brk = (AstBreak *)value;
    }
    else if (type == AST_CONTINUE) {
        node->as.cont = (AstContinue *)value;
    }
    else if (type == AST_UNARY) {
        node->as.unary = (AstUnary *)value;
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

int lenHelper(unsigned x) {
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

static void pretty_error(Parser *parser) {
    Token errTok;

    recede(parser);
    errTok = current_token(parser);

    if (errTok.lexeme == NULL) {
        advance(parser);
        for (int i = parser->current; i >= 0; i--) {
            if (parser->tokens[i].type != TOKEN_EOF) {
                errTok = parser->tokens[i];
                break;
            }
        }
    }

    printf("\n%s:%d\n", parser->file, errTok.line);
    printf("error: %s\n", parser_err_to_str(parser->err));

    const int line_num_len = 6; // '   XX | ' is 6 characters
    printf("   %d | ", errTok.line);

    int char_pos = 0;
    int caret_pos = -1;

    for (int i = 0; parser->tokens[i].type != TOKEN_EOF; i++) {
        Token tok = parser->tokens[i];
        if (tok.line != errTok.line) continue;

        if (caret_pos == -1 &&
            tok.line == errTok.line &&
            strcmp(tok.lexeme, errTok.lexeme) == 0) {
            caret_pos = char_pos;
        }

        printf("%s", tok.lexeme);
        char_pos += strlen(tok.lexeme);

        if (tok.has_whitespace_after) {
            printf(" ");
            char_pos++;
        }
    }

    if (caret_pos == -1) caret_pos = char_pos;

    printf("\n     | ");
    for (int i = 0; i < caret_pos + 1; i++) {
        printf(" ");
    }
    printf("^\n");
}


static int is_literal_or_ident(Parser *parser) {
    return match(TOKEN_INTEGER_LITERAL, parser)
        || match(TOKEN_CHAR_LITERAL, parser)
        || match(TOKEN_FLOAT_LITERAL, parser)
        || match(TOKEN_OCTAL_LITERAL, parser)
        || match(TOKEN_BINARY_LITERAL, parser)
        || match(TOKEN_STRING_LITERAL, parser)
        || match(TOKEN_HEX_LITERAL, parser)
        || match(TOKEN_IDENTIFIER, parser);
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

static AstUnary *init_unary_node(AstNode *left, Token op) {
    AstUnary *unary = malloc(sizeof(AstUnary));
    unary->left = left;
    unary->op = op;

    return unary;
}

static AstNode *parse_unary(Parser *parser) {
    if (match(TOKEN_PLUS, parser) || match(TOKEN_MINUS, parser) || match(TOKEN_EXCLAMATION, parser) || match(TOKEN_TILDE, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_unary(parser);
        AstUnary *unary = init_unary_node(right, op);
        return init_node(unary, AST_UNARY);
    }

    return parse_primary(parser);
}

static AstNode *parse_factor(Parser *parser) {
    AstNode *left = parse_unary(parser);
    
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

static AstNode *parse_comparison(Parser *parser) {
    AstNode *left = parse_term(parser);

    while (
        match(TOKEN_EQUALS, parser) || match(TOKEN_GREATER_THAN, parser) || match(TOKEN_LESS_THAN_EQUALS, parser) ||
        match(TOKEN_GREATER_THAN_EQUALS, parser) || match(TOKEN_LESS_THAN, parser) || match(TOKEN_NOT_EQUALS, parser)
    ) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_term(parser);
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_expression(Parser *parser) {
    return parse_comparison(parser);
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
    if (!is_literal_or_ident(parser)) {
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

static AstFunctionDeclaration *init_function_node(AstNode **body, int body_count, char *identifier, AstDataType returnType, AstFunctionParameter **params, int params_count) {
    AstFunctionDeclaration *func = (AstFunctionDeclaration *)malloc(sizeof(AstFunctionDeclaration));
    func->body = body;
    func->body_count = body_count;
    func->identifier = strdup(identifier);
    func->returnType = returnType;
    func->params = params;
    func->params_count = params_count;

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

static int is_valid_type(Token token) {
    return token_to_ast_data_type(token) != AST_TYPE_INVALID;
}

static AstFunctionParameter *init_func_parameter(char *id, AstDataType type, int constant) {
    AstFunctionParameter *param = malloc(sizeof(AstFunctionParameter));
    param->name = strdup(id);
    param->type = type;
    param->constant = constant;

    return param;
}

static AstNode *parse_function(Parser *parser) {
    Token return_type_token = current_token(parser);
    AstDataType type = token_to_ast_data_type(return_type_token);
    if (type == AST_TYPE_INVALID) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    advance(parser);

    Token identifier_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstFunctionParameter **params = malloc(sizeof(AstFunctionParameter *));
    if (!params) {
        return NULL;
    }

    int capacity = 1;
    int params_count = 0;

    recede(parser);
    do {
        advance(parser);

        if (match(TOKEN_RIGHT_PAREN, parser)) break;

        int constant = 0;
        if (match(TOKEN_CONST, parser)) {
            advance(parser);
            constant = 1;
        }

        Token type = current_token(parser);
        if (!is_valid_type(type)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }
        advance(parser);

        Token id = current_token(parser);
        if (!match(TOKEN_IDENTIFIER, parser)) {
            return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
        }
        advance(parser);

        AstFunctionParameter *param = init_func_parameter(id.lexeme, token_to_ast_data_type(type), constant);
        if (params_count >= capacity) {
            capacity *= 2;
            params = realloc(params, sizeof(AstFunctionParameter *) * capacity);
        }
        params[params_count++] = param;

    } while (match(TOKEN_COMMA, parser));

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);

        AstFunctionDeclaration *func = init_function_node(NULL, 0, identifier_token.lexeme, type, params, params_count);
        AstNode *node = init_node(func, AST_FUNCTION);

        return node;
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

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

    AstFunctionDeclaration *func = init_function_node(body, body_statement_count, identifier_token.lexeme, type, params, params_count);
    AstNode *node = init_node(func, AST_FUNCTION);

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        // free some shit here
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
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

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstReturn *ret = init_return(return_expr);
    AstNode *node = init_node(ret, AST_RETURN);

    return node;
}

static AstInlineAsmBlock *init_inline_asm_node(char **asm_lines, int line_count) {
    AstInlineAsmBlock *asm_inl = malloc(sizeof(AstInlineAsmBlock));
    asm_inl->lines = asm_lines;
    asm_inl->line_count = line_count;

    return asm_inl;
}

static AstNode *parse_inline_asm(Parser *parser) {
    advance(parser);
    
    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int capacity = 1;
    int count = 0;
    char **asm_lines = malloc(sizeof(char *) * capacity);

    recede(parser);
    do {
        advance(parser);
        
        if (!match(TOKEN_STRING_LITERAL, parser)) {
            free(asm_lines);
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        Token str_token = current_token(parser);
        
        if (count >= capacity) {
            capacity *= 2;
            char **new_lines = realloc(asm_lines, sizeof(char *) * capacity);
            if (!new_lines) {
                free(asm_lines);
                return parser_err(PARSE_ERR_OUT_OF_MEMORY, parser);
            }
            asm_lines = new_lines;
        }

        asm_lines[count++] = strdup(str_token.lexeme);
        advance(parser);

    } while (match(TOKEN_COMMA, parser));

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        for (size_t i = 0; i < count; i++) free(asm_lines[i]);
        free(asm_lines);
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        for (size_t i = 0; i < count; i++) free(asm_lines[i]);
        free(asm_lines);
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstInlineAsmBlock *asm_inl = init_inline_asm_node(asm_lines, count);
    AstNode *node = init_node(asm_inl, AST_INLINE_ASM_BLOCK);

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

static AstNode *parse_empty_expression(Parser *parser) {
    AstNode *expr = parse_expression(parser);
    if (!expr) return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        free_node(expr);
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    return expr;
}

static AstAssignment *init_assignment(char *identifier, AstNode *value) {
    AstAssignment *assign = malloc(sizeof(AstAssignment));
    assign->identifier = strdup(identifier);
    assign->value = value;

    return assign;
}

static AstNode *parse_assignment(Parser *parser) {
    Token id = current_token(parser);
    advance(parser);

    if (!expect(TOKEN_SINGLE_EQUALS, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *expr = parse_expression(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstAssignment *assign = init_assignment(id.lexeme, expr);
    AstNode *node = init_node(assign, AST_ASSIGNMENT);

    return node;
}

static AstIfStatement *init_if_statement(AstNode **body, AstNode **else_body, AstNode *condition, int body_count, int else_body_count) {
    AstIfStatement *iff = malloc(sizeof(AstIfStatement));
    iff->condition = condition;
    iff->body = body;
    iff->body_count = body_count;
    iff->else_body = else_body;
    iff->else_body_count = else_body_count;

    return iff;
}

static AstNode *parse_if(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *if_condition = parse_expression(parser);
    if (!if_condition) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *));

    do {
        if (match(TOKEN_RIGHT_BRACE, parser)) break;
        AstNode *statement = parse_statement(parser);

        if (body_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, body_capacity * sizeof(AstNode *));
        }
        body[body_count++] = statement;

    } while (!match(TOKEN_RIGHT_BRACE, parser));

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int else_body_count = 0;
    int else_body_capacity = 1;
    AstNode **else_body = malloc(sizeof(AstNode *));

    int has_else = 0;

    if (match(TOKEN_ELSE, parser)) {
        has_else = 1;
        advance(parser);
        
        if (!expect(TOKEN_LEFT_BRACE, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }
            
        do {
            if (match(TOKEN_RIGHT_BRACE, parser)) break;
            
            AstNode *statement = parse_statement(parser);

            if (else_body_count >= else_body_capacity) {
                else_body_capacity *= 2;
                else_body = realloc(else_body, else_body_capacity * sizeof(AstNode *));
            }
            else_body[else_body_count++] = statement;

        } while (!match(TOKEN_RIGHT_BRACE, parser));
    }

    if (has_else) {
        advance(parser);
    }

    AstIfStatement *iff = init_if_statement(body, else_body, if_condition, body_count, else_body_count);
    AstNode *node = init_node(iff, AST_IF);

    return node;
}

static AstWhile *init_while(AstNode *condition, AstNode **body, int body_count) {
    AstWhile *whilee = malloc(sizeof(AstWhile));
    whilee->condition = condition;
    whilee->body = body;
    whilee->body_count = body_count;

    return whilee;
}

static AstNode *parse_while(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *condition = parse_expression(parser);
    if (!condition) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *));

    do {
        if (match(TOKEN_RIGHT_BRACE, parser)) break;
        AstNode *statement = parse_statement(parser);

        if (body_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, body_capacity * sizeof(AstNode *));
        }
        body[body_count++] = statement;

    } while (!match(TOKEN_RIGHT_BRACE, parser));

    advance(parser);

    AstWhile *whilee = init_while(condition, body, body_count);
    AstNode *node = init_node(whilee, AST_WHILE);

    return node;
}

static AstNode *parse_break(Parser *parser) {

}

static AstNode *parse_continue(Parser *parser) {

}

static AstNode *parse_statement(Parser *parser) {
    if (is_valid_type(current_token(parser))) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_RETURN, parser)) {
        return parse_return(parser);
    }
    else if (match(TOKEN_ASM, parser)) {
        return parse_inline_asm(parser);
    }
    else if (match(TOKEN_IF, parser)) {
        return parse_if(parser);
    }
    else if (match(TOKEN_WHILE, parser)) {
        return parse_while(parser);
    }
    else if (match(TOKEN_BREAK, parser)) {
        return parse_break(parser);
    }
    else if (match(TOKEN_CONTINUE, parser)) {
        return parse_continue(parser);
    }
    else if (match(TOKEN_IDENTIFIER, parser)) {
        advance(parser);

        if (match(TOKEN_SINGLE_EQUALS, parser)) {
            recede(parser);
            return parse_assignment(parser);
        }
        else {
            return parse_empty_expression(parser);
        }
    }
    else {
        return parse_empty_expression(parser);
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
        pretty_error(parser);
        return;
    }

    if (parser->debug) parser_print(parser);
}