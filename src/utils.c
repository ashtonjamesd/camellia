#include "ast.h"

char* ast_type_to_str(AstType type) {
    switch (type) {
        case AST_VARIABLE_DECLARATION: return "VARIABLE DECLARATION";
        case AST_LITERAL_INT: return "LITERAL INT";
        case AST_LITERAL_CHAR: return "LITERAL CHAR";
        case AST_FUNCTION: return "FUNCTION";
        case AST_RETURN: return "RETURN";
        case AST_IDENTIFIER: return "IDENTIFIER";
        default: return "UNKNOWN TYPE";
    }
}

char *data_type_to_str(AstDataType type) {
    switch (type) {
        case AST_TYPE_CHAR: return "CHAR";
        case AST_TYPE_INT: return "INT";
        case AST_TYPE_VOID: return "VOID";
        default: return "UNKNOWN TYPE";
    }
}