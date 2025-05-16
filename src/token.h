#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_LONG,
    TOKEN_SHORT,
    TOKEN_UNSIGNED,
    TOKEN_AUTO,
    TOKEN_CONST,
    TOKEN_ASM,
    TOKEN_REGISTER,
    TOKEN_TYPEDEF,
    TOKEN_STATIC,
    TOKEN_GOTO,
    TOKEN_SIZEOF,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_DO,
    TOKEN_WHILE,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_VOID,

    TOKEN_INCLUDE,
    TOKEN_DEFINE,
    TOKEN_UNDEF,
    TOKEN_IFDEF,
    TOKEN_IFNDEF,
    TOKEN_ELIF,
    TOKEN_ENDIF,

    TOKEN_IDENTIFIER,
    TOKEN_STRING_LITERAL,
    TOKEN_INTEGER_LITERAL,
    TOKEN_HEX_LITERAL,
    TOKEN_BINARY_LITERAL,
    TOKEN_OCTAL_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_FLOAT_LITERAL,

    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_SINGLE_EQUALS,
    TOKEN_COMMA,
    TOKEN_HASHTAG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_MODULO,
    TOKEN_TILDE,
    TOKEN_EXCLAMATION,
    TOKEN_GREATER_THAN,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN_EQUALS,
    TOKEN_LESS_THAN_EQUALS,
    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,

    TOKEN_EOF,
} TokenType;


typedef struct {
    char     *lexeme;
    TokenType type;
    int       line;
    int       has_whitespace_after;
} Token;

typedef struct {
    const char *symbol;
    TokenType   type;
} SymbolToken;

extern char *token_type_to_str(TokenType type);

#endif