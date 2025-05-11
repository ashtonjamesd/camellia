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
} AstType;

typedef enum {
    TYPE_CHAR,
    TYPE_INT,
    TYPE_VOID,
} AstDataType;

typedef struct AstNode AstNode;

typedef struct {
    char *type;
    int   value;
} AstLiteralInt;

typedef struct {
    char *type;
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
    Token op;
    AstNode *right;
} AstBinaryExpr;

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
    } as;
};

typedef enum {
    NO_PARSER_ERROR
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