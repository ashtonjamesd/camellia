#include <stdlib.h>
#include <string.h>

#include "token.h"

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
        case TOKEN_STRING_LITERAL: return "STRING";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_CHAR_LITERAL: return "CHAR LITERAL";
        case TOKEN_STAR: return "STAR";
        case TOKEN_FLOAT_LITERAL: return "FLOAT LITERAL";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_DOUBLE: return "DOUBLE";
        case TOKEN_STRUCT: return "STRUCT";
        case TOKEN_UNION: return "UNION";
        case TOKEN_LONG: return "LONG";
        case TOKEN_SHORT: return "SHORT";
        case TOKEN_UNSIGNED: return "UNSIGNED";
        case TOKEN_AUTO: return "AUTO";
        case TOKEN_REGISTER: return "REGISTER";
        case TOKEN_TYPEDEF: return "TYPEDEF";
        case TOKEN_STATIC: return "STATIC";
        case TOKEN_GOTO: return "GOTO";
        case TOKEN_SIZEOF: return "SIZEOF";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_DO: return "DO";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_SWITCH: return "SWITCH";
        case TOKEN_CASE: return "CASE";
        case TOKEN_DEFAULT: return "DEFAULT";
        case TOKEN_VOID: return "VOID";
        case TOKEN_EOF: return "EOF";
        case TOKEN_HEX_LITERAL: return "HEX LITERAL";
        case TOKEN_BINARY_LITERAL: return "BINARY LITERAL";
        case TOKEN_OCTAL_LITERAL: return "OCTAL LITERAL";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_INCLUDE: return "INCLUDE";
        case TOKEN_DEFINE: return "DEFINE";
        case TOKEN_UNDEF: return "UNDEF";
        case TOKEN_IFDEF: return "IFDEF";
        case TOKEN_IFNDEF: return "IFNDEF";
        case TOKEN_ELIF: return "ELIF";
        case TOKEN_ENDIF: return "ENDIF";
        default: return "UNKNOWN";
    };
}