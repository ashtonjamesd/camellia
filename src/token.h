#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_INTEGER_LITERAL,
    TOKEN_SINGLE_EQUALS,
    TOKEN_COMMA,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_CHAR_LITERAL,
} TokenType;

typedef struct {
    char *lexeme;
    TokenType type;
} Token;

typedef struct {
    const char *symbol;
    TokenType type;
} SymbolToken;

extern Token *init_token(const char *lexeme, TokenType type);
extern char *token_type_to_str(TokenType type);

#endif