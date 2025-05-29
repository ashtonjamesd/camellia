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
    AST_BLOCK,
    AST_DO_WHILE,
    AST_STRUCT,
    AST_ENUM,
    AST_ENUM_VALUE,
    AST_UNION,
    AST_CAST,
    AST_ARR_SUBSCRIPT,
    AST_DECLARATOR,
    AST_TYPEDEF,
    AST_ARRAY_DECLARATION,
} AstType;

typedef enum {
    AST_TYPE_CHAR,
    AST_TYPE_INT,
    AST_TYPE_VOID,
    AST_TYPE_SHORT,
    AST_TYPE_DOUBLE,
    AST_TYPE_LONG,
    AST_TYPE_FLOAT,
    AST_TYPE_INVALID,
} AstDataType;

typedef struct AstNode AstNode;

typedef struct {
    int is_const;
    int is_volatile;
    int is_static;
    int is_unsigned;
    int is_signed;
    int long_count;
    int is_short;
    int pointer_level;
    TokenType type;
} TypeSpecifier;

typedef struct {
    int   value;
} AstLiteralInt;

typedef struct {
    char  value;
} AstLiteralChar;

typedef struct {
    char       *identifier;
    AstNode    *value;
    int         pointer_level;
} AstDeclarator;

typedef struct {
    AstDeclarator **declarators;
    int             declarator_count;
    TypeSpecifier   type_specifier;
} AstVariableDeclaration;

typedef struct {
    AstNode *value;
} AstReturn;

typedef struct {
    char *name;
} AstIdentifier;

typedef struct {
    AstNode *right;
    Token    type;
    int      pointer_level;
} AstCast;

typedef struct {
    char         *name;
    // AstDataType   type;
    // int           constant;
    TypeSpecifier type_specifier;
} AstFunctionParameter;

typedef struct {
    char     *name;
    AstNode **fields;
    int       field_count;
} AstStruct;

typedef struct {
    char     *name;
    AstNode **fields;
    int       field_count;
} AstUnion;

typedef struct {
    char *name;
    int   value;
    int   explicit_value;
} AstEnumValue;

typedef struct {
    char          *name;
    AstEnumValue **values;
    int            value_count;
} AstEnum;

typedef struct {
    AstNode **body;
    int       body_count;
} AstBlock;

typedef struct {
    char                  *identifier;
    AstDeclarator         *declarator;
    AstNode              **body;
    int                    body_count;
    int                    params_count;
    AstFunctionParameter **params;
    int                    is_void_params;
    TypeSpecifier          type_specifier;
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
    int    line_count;
} AstInlineAsmBlock;

typedef struct {
    char    *identifier;
    AstNode *value;
} AstAssignment;

typedef struct {
    TypeSpecifier type_specs;
    char *identifier;
} AstTypedef;

typedef struct {
    AstNode *condition;
    AstNode **body;
    int       body_count;
    AstNode **else_body;
    int       else_body_count;
} AstIfStatement;

typedef struct {
    AstNode *condition;
    AstNode **body;
    int       body_count;
} AstWhile;

typedef struct {
    AstNode *condition;
    AstBlock *block;
} AstDoWhile;

typedef struct {
    int dummy;
} AstBreak;

typedef struct {
    int dummy;
} AstContinue;

typedef struct {
    AstNode *left;
    Token    op;
    int      is_postfix;
} AstUnary;

typedef struct {
    AstNode *base;
    AstNode *index;
} AstArraySubscript;

typedef struct {
    AstNode *initializer;
    AstNode *condition;
    AstNode *alteration;
    AstNode *block;
} AstFor;

typedef struct {
    AstNode *condition;
    AstNode *true_expr;
    AstNode *false_expr;
} AstTernary;

typedef struct {
    char *identifier;
    TypeSpecifier type_specs;
    AstNode **dimensions;
    int dimension_count;
} AstArrayDeclaration;

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
        AstTernary             *ternary;
        AstBlock               *block;
        AstDoWhile             *do_while;
        AstStruct              *a_struct;
        AstEnum                *an_enum;
        AstEnumValue           *enum_val;
        AstUnion               *a_union;
        AstCast                *cast;
        AstArraySubscript      *arr_sub;
        AstDeclarator          *declarator;
        AstTypedef             *type_def;
        AstArrayDeclaration    *array_decl;
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
    int       is_var_dec;

    // the token that caused the err, null if none occurred
    Token     errToken;
} Parser;

extern Parser *init_parser(Token *tokens, int debug, char *file);
extern void parse_ast(Parser *parser);
extern void free_parser(Parser *parser);

#endif