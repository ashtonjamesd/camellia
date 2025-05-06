#include <stdlib.h>
#include <string.h>

#include "token.h"

Token *init_token(const char *lexeme, TokenType type) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->lexeme = strdup(lexeme);
    token->type =type;

    return token;
}

char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "INT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_LEFT_PAREN: return "LEFT PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT PAREN";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_LEFT_BRACE: return "LEFT BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT BRACE";
        case TOKEN_INTEGER_LITERAL: return "TOKEN INTEGER LITERAL";
        case TOKEN_SINGLE_EQUALS: return "SINGLE EQUALS";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_STRING: return "STRING";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_CHAR_LITERAL: return "CHAR LITERAL";
        default: return "UNKNOWN";
    };
}