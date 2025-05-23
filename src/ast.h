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
    AST_FUNCTION_PARAMETER,
    AST_INLINE_ASM_BLOCK,
    AST_ASSIGNMENT,
    AST_IF,
    AST_WHILE,
    AST_BREAK,
    AST_CONTINUE,
    AST_UNARY,
    AST_FOR,
    AST_TERNARY,
} AstType;

typedef enum {
    AST_TYPE_CHAR,
    AST_TYPE_INT,
    AST_TYPE_VOID,
    AST_TYPE_INVALID,
} AstDataType;

typedef enum {
    DECL_STATIC   = 1 << 0,
    DECL_CONST    = 1 << 1,
    DECL_VOLATILE = 1 << 2,
} AstDeclQualifierFlags;

typedef struct AstNode AstNode;

typedef struct {
    int   value;
} AstLiteralInt;

typedef struct {
    char  value;
} AstLiteralChar;

typedef struct {
    char       *identifier;
    AstNode    *value;
    int pointer_level;
} AstDeclarator;

typedef struct {
    AstDataType type;
    AstDeclarator **declarators;
    int declarator_count;
    int qualifiers; // bitmask for decl_flags
} AstVariableDeclaration;

typedef struct {
    AstNode *value;
} AstReturn;

typedef struct {
    char *name;
} AstIdentifier;

typedef struct {
    char *name;
    AstDataType type;
    int constant;
} AstFunctionParameter;

typedef struct {
    AstDataType            returnType;
    char                  *identifier;
    AstNode              **body;
    int                    body_count;
    int                    params_count;
    AstFunctionParameter **params;
} AstFunctionDeclaration;

typedef struct {
    AstNode *left;
    Token    op;
    AstNode *right;
} AstBinaryExpr;

typedef struct {
    char *identifier;
} AstCallExpr;

typedef struct {
    char **lines;
    int line_count;
} AstInlineAsmBlock;

typedef struct {
    char *identifier;
    AstNode *value;
} AstAssignment;

typedef struct {
    AstNode *condition;
    AstNode **body;
    int body_count;
    AstNode **else_body;
    int else_body_count;
} AstIfStatement;

typedef struct {
    AstNode *condition;
    AstNode **body;
    int body_count;
} AstWhile;

typedef struct {
    int dummy;
} AstBreak;

typedef struct {
    int dummy;
} AstContinue;

typedef struct {
    AstNode *left;
    Token    op;
} AstUnary;

typedef struct {
    AstNode *initializer;
    AstNode *condition;
    AstNode *alteration;
    AstNode **body;
    int body_count;
} AstFor;

typedef struct {
    AstNode *condition;
    AstNode *true_expr;
    AstNode *false_expr;
} AstTernary;

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
        AstFunctionParameter   *param;
        AstInlineAsmBlock      *asm_inl;
        AstAssignment          *assign;
        AstIfStatement         *if_stmt;
        AstWhile               *while_stmt;
        AstBreak               *brk;
        AstContinue            *cont;
        AstUnary               *unary;
        AstFor                 *for_stmt;
        AstTernary         *ternary;
    } as;
};

typedef enum {
    NO_PARSER_ERROR,
    PARSE_ERR_OUT_OF_MEMORY,
    PARSE_ERR_EXPECTED_EXPRESSION,
    PARSE_ERR_EXPECTED_IDENTIFIER,
    PARSE_ERR_EXPECTED_SEMICOLON,
    PARSE_ERR_INVALID_SYNTAX,
    PARSE_ERR_VOID_NOT_ALLOWED,
} ParseErr;

typedef struct {
    AstNode **tree;
    int       node_count;
    int       node_capacity;
    int       debug;
    int       current;
    Token    *tokens;

    // the name of the file being parsed
    char     *file;
    ParseErr  err;

    // 1 when the parser is parsing a variable declaration
    // this is used to stop the parser from assuming a comma operator in the following:
    //    int x = 1, y;
    //
    // this flag prevents the above being parsed as a binary expression rather than a list of declarators
    int is_var_dec;

    // the token that caused the err, null if none occurred
    Token     errToken;
} Parser;

extern Parser *init_parser(Token *tokens, int debug, char *file);
extern void parse_ast(Parser *parser);
extern void free_parser(Parser *parser);

#endif