#include <stdlib.h>
#include <string.h>

#include "token.h"

#include <stdio.h>
#include "token.h"

char *token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "int";
        case TOKEN_RETURN: return "return";
        case TOKEN_CHAR: return "char";
        case TOKEN_FLOAT: return "float";
        case TOKEN_DOUBLE: return "double";
        case TOKEN_STRUCT: return "struct";
        case TOKEN_UNION: return "union";
        case TOKEN_LONG: return "long";
        case TOKEN_SHORT: return "short";
        case TOKEN_UNSIGNED: return "unsigned";
        case TOKEN_AUTO: return "auto";
        case TOKEN_CONST: return "const";
        case TOKEN_ASM: return "asm";
        case TOKEN_REGISTER: return "register";
        case TOKEN_TYPEDEF: return "typedef";
        case TOKEN_STATIC: return "static";
        case TOKEN_GOTO: return "goto";
        case TOKEN_SIZEOF: return "sizeof";
        case TOKEN_BREAK: return "break";
        case TOKEN_CONTINUE: return "continue";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_FOR: return "for";
        case TOKEN_DO: return "do";
        case TOKEN_WHILE: return "while";
        case TOKEN_SWITCH: return "switch";
        case TOKEN_CASE: return "case";
        case TOKEN_DEFAULT: return "default";
        case TOKEN_VOID: return "void";

        case TOKEN_INCLUDE: return "include";
        case TOKEN_DEFINE: return "define";
        case TOKEN_UNDEF: return "undef";
        case TOKEN_IFDEF: return "ifdef";
        case TOKEN_IFNDEF: return "ifndef";
        case TOKEN_ELIF: return "elif";
        case TOKEN_ENDIF: return "endif";

        case TOKEN_IDENTIFIER: return "identifier";
        case TOKEN_STRING_LITERAL: return "string literal";
        case TOKEN_INTEGER_LITERAL: return "integer literal";
        case TOKEN_HEX_LITERAL: return "hex literal";
        case TOKEN_BINARY_LITERAL: return "binary literal";
        case TOKEN_OCTAL_LITERAL: return "octal literal";
        case TOKEN_CHAR_LITERAL: return "char literal";
        case TOKEN_FLOAT_LITERAL: return "float literal";

        case TOKEN_LEFT_PAREN: return "left paren";
        case TOKEN_RIGHT_PAREN: return "right paren";
        case TOKEN_SEMICOLON: return "semicolon";
        case TOKEN_LEFT_BRACE: return "left brace";
        case TOKEN_RIGHT_BRACE: return "right brace";
        case TOKEN_SINGLE_EQUALS: return "single equals";
        case TOKEN_COMMA: return "comma";
        case TOKEN_HASHTAG: return "hashtag";
        case TOKEN_EXCLAMATION: return "exclamation";
        case TOKEN_GREATER_THAN: return "greater than";
        case TOKEN_LESS_THAN: return "less than";
        case TOKEN_GREATER_THAN_EQUALS: return "greater than equals";
        case TOKEN_LESS_THAN_EQUALS: return "less than equals";
        case TOKEN_EQUALS: return "equals";
        case TOKEN_NOT_EQUALS: return "not equals";
        case TOKEN_AND: return "and";
        case TOKEN_OR: return "or";
        case TOKEN_DOT: return "dot";

        case TOKEN_PLUS: return "plus";
        case TOKEN_MINUS: return "minus";
        case TOKEN_STAR: return "star";
        case TOKEN_SLASH: return "slash";
        case TOKEN_MODULO: return "modulo";
        case TOKEN_DECREMENT: return "decrement";
        case TOKEN_INCREMENT: return "increment";
        case TOKEN_PLUS_EQUALS: return "plus equals";
        case TOKEN_MINUS_EQUALS: return "minus equals";
        case TOKEN_STAR_EQUALS: return "star equals";
        case TOKEN_SLASH_EQUALS: return "slash equals";
        case TOKEN_MODULO_EQUALS: return "modulo equals";
        case TOKEN_BITWISE_AND_EQUALS: return "bitwise and equals";
        case TOKEN_BITWISE_OR_EQUALS: return "bitwise or equals";
        case TOKEN_BITWISE_XOR_EQUALS: return "bitwise xor equals";
        case TOKEN_BITWISE_LEFT_SHIFT_EQUALS: return "bitwise left shift equals";
        case TOKEN_BITWISE_RIGHT_SHIFT_EQUALS: return "bitwise right shift equals";
        case TOKEN_ARROW_OP: return "arrow op";
        case TOKEN_SQUARE_BRACKET_LEFT: return "square bracket left";
        case TOKEN_SQUARE_BRACKET_RIGHT: return "square bracket right";
        case TOKEN_QUESTION: return "question";
        case TOKEN_COLON: return "colon";
        case TOKEN_ELLIPSIS: return "ellipsis";

        case TOKEN_BITWISE_AND: return "bitwise and";
        case TOKEN_BITWISE_OR: return "bitwise or";
        case TOKEN_BITWISE_XOR: return "bitwise xor";
        case TOKEN_BITWISE_LEFT_SHIFT: return "bitwise left shift";
        case TOKEN_BITWISE_RIGHT_SHIFT: return "bitwise right shift";
        case TOKEN_BITWISE_NOT: return "bitwise not";

        case TOKEN_EOF: return "end of file";

        default: return "unknown token";
    }
}