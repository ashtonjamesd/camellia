#include "ast.h"

char* ast_type_to_str(AstType type) {
    switch (type) {
        case AST_VARIABLE_DECLARATION: return "variable declaration";
        case AST_LITERAL_INT: return "literal int";
        case AST_LITERAL_CHAR: return "literal char";
        case AST_FUNCTION: return "function";
        case AST_RETURN: return "return";
        case AST_IDENTIFIER: return "identifier";
        case AST_BINARY: return "binary";
        case AST_CALL_EXPR: return "call expr";
        case AST_FUNCTION_PARAMETER: return "function param";
        case AST_INLINE_ASM_BLOCK: return "inline asm";
        case AST_ASSIGNMENT: return "assignment";
        case AST_IF: return "if";
        case AST_WHILE: return "while";
        case AST_BREAK: return "break";
        case AST_CONTINUE: return "continue";
        case AST_UNARY: return "unary";
        case AST_FOR: return "for";
        case AST_TERNARY: return "ternary";
        default: return "unknown type";
    }
}

char *data_type_to_str(AstDataType type) {
    switch (type) {
        case AST_TYPE_CHAR: return "char";
        case AST_TYPE_INT: return "int";
        case AST_TYPE_VOID: return "void";
        default: return "unknown type";
    }
}