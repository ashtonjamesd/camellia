#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    AST_VARIABLE_DECLARATION,
    AST_LITERAL_INT,
    AST_LITERAL_CHAR,
    AST_FUNCTION,
    AST_RETURN,
    AST_IDENTIFIER,
    AST_BINARY,
    AST_CALL_EXPR,
} AstType;

typedef enum {
    AST_TYPE_CHAR,
    AST_TYPE_INT,
    AST_TYPE_VOID,
    AST_TYPE_INVALID,
} AstDataType;

typedef struct AstNode AstNode;

typedef struct {
    int   value;
} AstLiteralInt;

typedef struct {
    char  value;
} AstLiteralChar;

typedef struct {
    AstDataType type;
    char       *identifier;
    AstNode    *value;
} AstVariableDeclaration;

typedef struct {
    AstNode *value;
} AstReturn;

typedef struct {
    char *name;
} AstIdentifier;

typedef struct {
    AstDataType  returnType;
    char        *identifier;
    AstNode    **body;
    int          count;
} AstFunctionDeclaration;

typedef struct {
    AstNode *left;
    Token    op;
    AstNode *right;
} AstBinaryExpr;

typedef struct {
    char *identifier;
} AstCallExpr;

struct AstNode {
    AstType type;

    union {
        AstLiteralInt          *lit_int;
        AstLiteralChar         *lit_char;
        AstVariableDeclaration *var_dec;
        AstFunctionDeclaration *func;
        AstReturn              *ret;
        AstIdentifier          *ident;
        AstBinaryExpr          *binary;
        AstCallExpr            *call;
    } as;
};

typedef enum {
    NO_PARSER_ERROR,
    PARSE_ERR_EXPECTED_EXPRESSION,
    PARSE_ERR_EXPECTED_IDENTIFIER,
    PARSE_ERR_EXPECTED_SEMICOLON,
    PARSE_ERR_VOID_NOT_ALLOWED,
} ParseErr;

typedef struct {
    AstNode **tree;
    int       node_count;
    int       node_capacity;
    int       debug;
    int       current;
    Token    *tokens;
    ParseErr  err;
} Parser;

extern Parser *init_parser(Token *tokens, int debug);
extern void parse_ast(Parser *parser);
extern void free_parser(Parser *parser);

#endif