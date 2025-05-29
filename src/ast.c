#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "ast.h"
#include "utils.h"

static AstNode *parse_statement(Parser *parser);
static AstNode *parse_expression(Parser *parser);
static AstAssignment *init_assignment(char *identifier, AstNode *value);

Parser *init_parser(Token *tokens, int debug, char *file) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    if (!parser) {
        perror("Error allocating parser.");
        return NULL;
    }

    parser->node_count = 0;
    parser->node_capacity = 1;
    parser->debug = debug;
    parser->tree = malloc(sizeof(AstNode *) * parser->node_capacity);
    parser->tokens = tokens;
    parser->err = NO_PARSER_ERROR;
    parser->current = 0;
    parser->file = file;

    return parser;
}

static void print_depth(int depth) {
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }
}

static void print_type_specifiers(TypeSpecifier type_specifier, int depth) {
    if (type_specifier.is_static) printf(" static ");
    if (type_specifier.is_volatile) printf(" volatile ");
    if (type_specifier.is_const) printf(" const ");
    if (type_specifier.is_short) printf(" short ");
    if (type_specifier.is_signed) printf(" signed ");
    if (type_specifier.is_unsigned) printf(" unsigned ");
    for (int i = 0; i < type_specifier.long_count; i++) printf(" long ");
    for (int i = 0; i < type_specifier.pointer_level; i++) printf("*");
    printf("\n");
    print_depth(depth);
    printf("TYPE: ");
    printf("%s\n", token_type_to_str(type_specifier.type));
}

static void print_node(AstNode *node, int depth) {
    print_depth(depth);

    int factor = 2;

    switch (node->type) {
        case AST_LITERAL_INT:
            printf("LITERAL INT: %d\n", node->as.lit_int->value);
            break;

        case AST_LITERAL_CHAR:
            printf("LITERAL CHAR: '%c'\n", node->as.lit_char->value);
            break;

        case AST_VARIABLE_DECLARATION:
            printf("VARIABLE DECLARATION:\n");
            print_depth(depth + factor);
            printf("DECLARATORS (%d):\n", node->as.var_dec->declarator_count);
            print_depth(depth + factor);
            printf("TYPE SPECIFIERS:");
            print_type_specifiers(node->as.var_dec->type_specifier, depth + factor);

            for (int i = 0; i < node->as.var_dec->declarator_count; i++) {
                print_depth(depth + factor * 2);
                printf("DECLARATOR:\n");
                print_depth(depth + factor * 3);
                printf("IDENTIFIER: %s\n", node->as.var_dec->declarators[i]->identifier);
                print_depth(depth + factor * 3);
                printf("POINTER LEVEL: %d\n", node->as.var_dec->declarators[i]->pointer_level);
                if (node->as.var_dec->declarators[i]->value) {
                    print_depth(depth + factor * 3);
                    printf("VALUE:\n");
                    print_node(node->as.var_dec->declarators[i]->value, depth + factor * 4);
                }
            }
            break;

        case AST_INLINE_ASM_BLOCK:
            printf("INLINE ASM (%d lines):\n", node->as.asm_inl->line_count);
            for (int i = 0; i < node->as.asm_inl->line_count; i++) {
                print_depth(depth + 1);
                printf("%s\n", node->as.asm_inl->lines[i]);
            }
            break;

        case AST_FUNCTION:
            printf("FUNCTION:\n");
            print_depth(depth + factor);
            printf("IDENTIFIER: %s\n", node->as.func->identifier);
            print_depth(depth + factor);
            printf("TYPE SPECIFIERS:");
            print_type_specifiers(node->as.func->type_specifier, depth + factor);
            print_depth(depth + factor);
            printf("IS VOID PARAMS: %d\n", node->as.func->is_void_params);
            print_depth(depth + factor);
            printf("PARAMETERS (%d):\n", node->as.func->params_count);
            for (int i = 0; i < node->as.func->params_count; i++) {
                print_depth(depth + factor * 2);
                printf("ID: %s\n", node->as.func->params[i]->name);
                print_depth(depth + factor * 2);
                printf("TYPE SPECIFIER: ");
                print_type_specifiers(node->as.func->params[i]->type_specifier, depth + factor);
            }

            print_depth(depth + factor);
            printf("BODY (%d):\n", node->as.func->body_count);
            for (int i = 0; i < node->as.func->body_count; i++) {
                print_node(node->as.func->body[i], depth + factor * 2);
            }
            break;

        case AST_RETURN:
            printf("RETURN:\n");
            print_node(node->as.ret->value, depth + factor);
            break;

        case AST_BINARY:
            printf("BINARY EXPRESSION:\n");
            print_depth(depth + factor);
            printf("LEFT:\n");
            print_node(node->as.binary->left, depth + factor * 2);
            print_depth(depth + factor);
            printf("OPERATOR: %s\n", node->as.binary->op.lexeme);
            print_depth(depth + factor);
            printf("RIGHT:\n");
            print_node(node->as.binary->right, depth + factor * 2);
            break;

        case AST_UNARY:
            printf("UNARY EXPRESSION:\n");
            print_depth(depth + factor);
            printf("LEFT:\n");
            print_node(node->as.unary->left, depth +  factor * 2);
            print_depth(depth + factor);
            printf("OPERATOR: %s\n", node->as.unary->op.lexeme);
            print_depth(depth + factor);
            printf("POSTFIX: %d\n", node->as.unary->is_postfix);
            break;

        case AST_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->as.ident->name);
            break;

        case AST_CALL_EXPR:
            printf("CALL EXPRESSION:\n");
            print_depth(depth + factor);
            printf("FUNCTION: %s\n", node->as.call->identifier);
            break;

        case AST_ASSIGNMENT:
            printf("ASSIGNMENT:\n");
            print_depth(depth + factor);
            printf("IDENTIFIER: %s\n", node->as.assign->identifier);
            print_depth(depth + factor);
            printf("VALUE:\n");
            print_node(node->as.assign->value, depth + factor * 2);
            break;

        case AST_IF:
            printf("IF STATEMENT:\n");
            print_depth(depth + factor);
            printf("CONDITION:\n");
            print_node(node->as.if_stmt->condition, depth + factor * 2);
            print_depth(depth + factor * 2);
            printf("BODY (%d):\n", node->as.if_stmt->body_count);
            for (int i = 0; i < node->as.if_stmt->body_count; i++) {
                print_node(node->as.if_stmt->body[i], depth + factor * 3);
            }
            if (node->as.if_stmt->else_body) {
                print_depth(depth);
                printf("ELSE:\n");
                print_depth(depth + factor);
                printf("BODY (%d):\n", node->as.if_stmt->else_body_count);
                for (int i = 0; i < node->as.if_stmt->else_body_count; i++) {
                    print_node(node->as.if_stmt->else_body[i], depth + factor * 2);
                }
            }
            break;

        case AST_WHILE:
            printf("WHILE STATEMENT:\n");
            print_depth(depth + factor);
            printf("CONDITION:\n");
            print_node(node->as.while_stmt->condition, depth + factor * 2);
            print_depth(depth + factor * 2);
            printf("BODY (%d):\n", node->as.while_stmt->body_count);
            for (int i = 0; i < node->as.while_stmt->body_count; i++) {
                print_node(node->as.while_stmt->body[i], depth + factor * 3);
            }
            break;

        case AST_FOR:
            printf("FOR STATEMENT:\n");
            print_depth(depth + factor);
            
            printf("INITIALIZER:\n");
            if (node->as.for_stmt->initializer) {
                print_node(node->as.for_stmt->initializer, depth + factor * 2);
            } else {
                print_depth(depth + factor * 2);
                printf("NONE\n");
            }
            print_depth(depth + factor);
            
            printf("CONDITION:\n");
            if (node->as.for_stmt->condition) {
                print_node(node->as.for_stmt->condition, depth + factor * 2);
            } else {
                print_depth(depth + factor * 2);
                printf("NONE\n");
            }
            print_depth(depth + factor);
            
            printf("ALTERATION:\n");
            if (node->as.for_stmt->alteration) {
                print_node(node->as.for_stmt->alteration, depth + factor * 2);
            } else {
                print_depth(depth + factor * 2);
                printf("NONE\n");
            }
            print_node(node->as.for_stmt->block, depth + factor);
            break;
        
        case AST_BLOCK:
            printf("BODY (%d):\n", node->as.block->body_count);
            for (int i = 0; i < node->as.block->body_count; i++) {
                print_node(node->as.block->body[i], depth + factor);
            }
            break;

        case AST_DO_WHILE:
            printf("DO WHILE STATEMENT:\n");
            print_depth(depth + factor);
            printf("CONDITION:\n");
            print_node(node->as.while_stmt->condition, depth + factor * 2);
            print_depth(depth + factor);
            printf("BODY (%d):\n", node->as.do_while->block->body_count);
            for (int i = 0; i < node->as.do_while->block->body_count; i++) {
                print_node(node->as.do_while->block->body[i], depth + factor * 2);
            }
            break;

        case AST_STRUCT:
            printf("STRUCT:\n");
            print_depth(depth + factor);
            printf("NAME: %s\n", node->as.a_struct->name);
            print_depth(depth + factor);
            printf("FIELDS: (%d)\n", node->as.a_struct->field_count);
            for (int i = 0; i < node->as.a_struct->field_count; i++) {
                print_node(node->as.a_struct->fields[i], depth + factor * 2);
            }
            break;

        case AST_UNION:
            printf("UNION:\n");
            print_depth(depth + factor);
            printf("NAME: %s\n", node->as.a_union->name);
            print_depth(depth + factor);
            printf("FIELDS: (%d)\n", node->as.a_union->field_count);
            for (int i = 0; i < node->as.a_union->field_count; i++) {
                print_node(node->as.a_union->fields[i], depth + factor * 2);
            }
            break;

        case AST_ENUM:
            printf("ENUM:\n");
            print_depth(depth + factor);
            printf("NAME: %s\n", node->as.an_enum->name);
            print_depth(depth + factor);
            printf("VALUES (%d):\n", node->as.an_enum->value_count);
            for (int i = 0; i < node->as.an_enum->value_count; i++) {
                print_depth(depth + factor * 2);
                printf("ENUM VALUE %d:\n", i);
                print_depth(depth + factor * 3);
                printf("NAME: %s\n", node->as.an_enum->values[i]->name);
                print_depth(depth + factor * 3);
                printf("VALUE: %d\n", node->as.an_enum->values[i]->value);
                print_depth(depth + factor * 3);
                printf("EXPLICIT: %d\n", node->as.an_enum->values[i]->explicit_value);
            }
            break;

        case AST_TERNARY:
            printf("TERNARY:\n");
            print_depth(depth + factor);
            printf("CONDITION:\n");
            print_node(node->as.ternary->condition, depth + factor * 2);
            print_depth(depth + factor);
            printf("TRUE EXPR:\n");
            print_node(node->as.ternary->true_expr, depth + factor * 2);
            print_depth(depth + factor);
            printf("FALSE EXPR:\n");
            print_node(node->as.ternary->false_expr, depth + factor * 2);
            break;
            
        case AST_CAST:
            printf("CAST:\n");
            print_depth(depth + factor);
            printf("RIGHT:\n");
            print_node(node->as.cast->right, depth + factor * 2);
            print_depth(depth + factor);
            printf("TO: %s\n", node->as.cast->type.lexeme);
            print_depth(depth + factor);
            printf("POINTER LEVEL: %d\n", node->as.cast->pointer_level);
            break;

        case AST_ARR_SUBSCRIPT:
            printf("ARR SUB:\n");
            print_depth(depth + factor);
            printf("BASE:\n");
            print_node(node->as.arr_sub->base, depth + factor * 2);
            print_depth(depth + factor);
            printf("INDEX:\n");
            print_node(node->as.arr_sub->index, depth + factor * 2);
            break;

        case AST_TYPEDEF:
            printf("IDENTIFIER: ");
            printf("%s\n", node->as.type_def->identifier);
            print_depth(depth + factor);
            printf("TYPE SPECIFIERS:");
            print_type_specifiers(node->as.type_def->type_specs, depth + factor);
            break;

        case AST_ARRAY_DECLARATION:
            printf("ARRAY DECLARATION: ");
            printf("%s\n", node->as.array_decl->identifier);
            print_depth(depth + factor);
            printf("TYPE SPECIFIERS:");
            print_type_specifiers(node->as.array_decl->type_specs, depth + factor);
            print_depth(depth + factor);
            printf("DIMENSIONS (%d):\n", node->as.array_decl->dimension_count);
            for (int i = 0; i < node->as.array_decl->dimension_count; i++) {
                print_depth(depth + factor * 2);
                printf("DIMENSION (%d):\n", i);
                print_node(node->as.array_decl->dimensions[i], depth + factor * 3);
            }
            break;

        case AST_FUNCTION_POINTER_DECLARATION:
            printf("FUNCTION POINTER DECLARATION: ");
            printf("%s\n", node->as.fptr->identifier);
            print_depth(depth + factor);
            printf("RETURN TYPE SPECIFIERS: ");
            print_type_specifiers(node->as.fptr->return_type_specs, depth + factor);
            print_depth(depth + factor);
            printf("PARAM TYPE SPECIFIERS (%d):", node->as.fptr->param_count);
            for (int i = 0; i < node->as.fptr->param_count; i++) {
                print_type_specifiers(node->as.fptr->param_type_specs[i], depth + factor * 2);
            }
            break;

        default:
            printf("unknown node type: '%s'\n", ast_type_to_str(node->type));
            break;
    }
}

static char *parser_err_to_str(ParseErr err) {
    switch (err) {
        case NO_PARSER_ERROR: return "no parser error";
        case PARSE_ERR_EXPECTED_EXPRESSION: return "expected expression";
        case PARSE_ERR_EXPECTED_IDENTIFIER: return "expected identifier";
        case PARSE_ERR_EXPECTED_SEMICOLON: return "expected semicolon";
        case PARSE_ERR_VOID_NOT_ALLOWED: return "void not allowed here";
        case PARSE_ERR_INVALID_SYNTAX: return "invalid syntax";
        case PARSE_ERR_OUT_OF_MEMORY: return "out of memory";
        default: return "unknown error, ummm oops";
    }
}

void parser_print(Parser *parser) {
  printf("\n\nPARSER SUCCESS\n");
  printf("AST Nodes (%d):\n", parser->node_count);
    for (int i = 0; i < parser->node_count; i++) {
        AstNode *node = parser->tree[i];
        print_node(node, 1);
    }
    printf("\n");
}

static void free_node(AstNode *node) {
    if (!node) return;

    if (node->type == AST_LITERAL_INT) {
        free(node->as.lit_int);
        free(node);
    }
    else if (node->type == AST_LITERAL_CHAR) {
        free(node->as.lit_char);
        free(node);
    }
    else if (node->type == AST_VARIABLE_DECLARATION) {
        for (int i = 0; i < node->as.var_dec->declarator_count; i++) {
            free(node->as.var_dec->declarators[i]->identifier);
            free_node(node->as.var_dec->declarators[i]->value);
        }
        free(node->as.var_dec->declarators);
        free(node->as.var_dec);
        free(node);
    }
    else if (node->type == AST_FUNCTION) {
        for (int i = 0; i < node->as.func->body_count; i++) {
            free_node(node->as.func->body[i]);
        }
        for (int i = 0; i < node->as.func->params_count; i++) {
            free(node->as.func->params[i]->name);
            free(node->as.func->params[i]);
        }
        free(node->as.func->identifier);
        free(node->as.func->body);
        free(node->as.func);
        free(node);
    }
    else if (node->type == AST_FUNCTION_PARAMETER) {
        free(node->as.param->name);
        free(node->as.param);
        free(node);
    }
    else if (node->type == AST_RETURN) {
        free_node(node->as.ret->value);
        free(node->as.ret);
        free(node);
    }
    else if (node->type == AST_IDENTIFIER) {
        free(node->as.ident->name);
        free(node->as.ident);
        free(node);
    }
    else if (node->type == AST_BINARY) {
        free_node(node->as.binary->left);
        free_node(node->as.binary->right);
        free(node->as.binary);
        free(node);
    }
    else if (node->type == AST_UNARY) {
        free(node->as.unary->left);
        free(node->as.unary);
        free(node);
    }
    else if (node->type == AST_CALL_EXPR) {
        free(node->as.call->identifier);
        free(node->as.call);
        free(node);
    }
    else if (node->type == AST_INLINE_ASM_BLOCK) {
        for (int i = 0; i < node->as.asm_inl->line_count; i++) {
            free(node->as.asm_inl->lines[i]);
        }

        free(node->as.asm_inl);
        free(node);
    }
    else if (node->type == AST_ASSIGNMENT) {
        free_node(node->as.assign->value);
        free(node->as.assign->identifier);
        free(node->as.assign);
        free(node);
    }
    else if (node->type == AST_IF) {
        for (int i = 0; i < node->as.if_stmt->body_count; i++) {
            free_node(node->as.if_stmt->body[i]);
        }

        free(node->as.if_stmt);
        free(node);
    }
    else if (node->type == AST_WHILE) {
        for (int i = 0; i < node->as.while_stmt->body_count; i++) {
            free_node(node->as.while_stmt->body[i]);
        }

        free_node(node->as.while_stmt->condition);

        free(node->as.while_stmt);
        free(node);
    }
    else if (node->type == AST_CONTINUE) {
        free(node->as.cont);
        free(node);
    }
    else if (node->type == AST_BREAK) {
        free(node->as.brk);
        free(node);
    }
    else if (node->type == AST_FOR) {
        free_node(node->as.for_stmt->block);

        free_node(node->as.for_stmt->initializer);
        free_node(node->as.for_stmt->condition);
        free_node(node->as.for_stmt->alteration);

        free(node->as.for_stmt);
        free(node);
    }
    else if (node->type == AST_BLOCK) {
        for (int i = 0; i < node->as.block->body_count; i++) {
            free_node(node->as.block->body[i]);
        }
    }
    else if (node->type == AST_TERNARY) {
        free(node->as.ternary->condition);
        free(node->as.ternary->true_expr);
        free(node->as.ternary->false_expr);
        free(node->as.ternary);
        free(node);
    }
    else if (node->type == AST_STRUCT) {
        for (int i = 0; i < node->as.a_struct->field_count; i++) {
            free_node(node->as.a_struct->fields[i]);
        }
        free(node->as.a_struct->name);
        free(node->as.a_struct);
        free(node);
    }
    else if (node->type == AST_UNION) {
        for (int i = 0; i < node->as.a_union->field_count; i++) {
            free_node(node->as.a_union->fields[i]);
        }
        free(node->as.a_union->name);
        free(node->as.a_union);
        free(node);
    }
    else if (node->type == AST_ENUM) {
        for (int i = 0; i < node->as.an_enum->value_count; i++) {
            free(node->as.an_enum->values[i]->name);
            free(node->as.an_enum->values[i]);
        }
        free(node->as.an_enum->name);
        free(node->as.an_enum);
        free(node);
    }
    else if (node->type == AST_CAST) {
        free_node(node->as.cast->right);
        free(node->as.cast);
        free(node);
    }
    else if (node->type == AST_ARR_SUBSCRIPT) {
        free_node(node->as.arr_sub->index);
        free_node(node->as.arr_sub->base);
        free(node->as.arr_sub);
        free(node);
    }
    else if (node->type == AST_TYPEDEF) {
        free(node->as.type_def->identifier);
        free(node->as.type_def);
        free(node);
    }
    else if (node->type == AST_ARRAY_DECLARATION) {
        for (int i = 0; i < node->as.array_decl->dimension_count; i++) {
            free_node(node->as.array_decl->dimensions[i]);
        }
        free(node->as.array_decl->identifier);
        free(node->as.array_decl);
        free(node);
    }
    else if (node->type == AST_FUNCTION_POINTER_DECLARATION) {
        // for (int i = 0; i < node->as.fptr->param_count; i++) {
        //     free(node->as.fptr->param_type_specs);
        // }
        free(node->as.fptr->identifier);
        free(node->as.fptr);
        free(node);
    }
    else {
        printf("unknown ast_type in 'free_node': '%s' (%d)\n", ast_type_to_str(node->type), node->type);
        printf("you probably forgot to add this type to the if-else block.\n");
    }
}

void free_parser(Parser *parser) {
    for (int i = 0; i < parser->node_count; i++) {
        free_node(parser->tree[i]);
    }

    free(parser->tree);
    free(parser);
}

static AstNode *init_node(void *value, AstType type){
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node) {
        perror("Error allocating node.");
        return NULL;
    }

    node->type = type;

    if (type == AST_LITERAL_INT) {
        node->as.lit_int = (AstLiteralInt *)value;
    }
    else if (type == AST_LITERAL_CHAR) {
        node->as.lit_char = (AstLiteralChar *)value;
    }
    else if (type == AST_VARIABLE_DECLARATION) {
        node->as.var_dec = (AstVariableDeclaration *)value;
    }
    else if (type == AST_FUNCTION) {
        node->as.func = (AstFunctionDeclaration *)value;
    }
    else if (type == AST_RETURN) {
        node->as.ret = (AstReturn *)value;
    }
    else if (type == AST_IDENTIFIER) {
        node->as.ident = (AstIdentifier *)value;
    }
    else if (type == AST_BINARY) {
        node->as.binary = (AstBinaryExpr *)value;
    }
    else if (type == AST_CALL_EXPR) {
        node->as.call = (AstCallExpr *)value;
    }
    else if (type == AST_INLINE_ASM_BLOCK) {
        node->as.asm_inl = (AstInlineAsmBlock *)value;
    }
    else if (type == AST_ASSIGNMENT) {
        node->as.assign = (AstAssignment *)value;
    }
    else if (type == AST_IF) {
        node->as.if_stmt = (AstIfStatement *)value;
    }
    else if (type == AST_WHILE) {
        node->as.while_stmt = (AstWhile *)value;
    }
    else if (type == AST_BREAK) {
        node->as.brk = (AstBreak *)value;
    }
    else if (type == AST_CONTINUE) {
        node->as.cont = (AstContinue *)value;
    }
    else if (type == AST_UNARY) {
        node->as.unary = (AstUnary *)value;
    }
    else if (type == AST_FOR) {
        node->as.for_stmt = (AstFor *)value;
    }
    else if (type == AST_TERNARY) {
        node->as.ternary = (AstTernary *)value;
    }
    else if (type == AST_BLOCK) {
        node->as.block = (AstBlock *)value;
    }
    else if (type == AST_DO_WHILE) {
        node->as.do_while = (AstDoWhile *)value;
    }
    else if (type == AST_STRUCT) {
        node->as.a_struct = (AstStruct *)value;
    }
    else if (type == AST_ENUM) {
        node->as.an_enum = (AstEnum *)value;
    }
    else if (type == AST_UNION) {
        node->as.a_union = (AstUnion *)value;
    }
    else if (type == AST_CAST) {
        node->as.cast = (AstCast *)value;
    }
    else if (type == AST_ARR_SUBSCRIPT) {
        node->as.arr_sub = (AstArraySubscript *)value;
    }
    else if (type == AST_TYPEDEF) {
        node->as.type_def = (AstTypedef *)value;
    }
    else if (type == AST_ARRAY_DECLARATION) {
        node->as.array_decl = (AstArrayDeclaration *)value;
    }
    else if (type == AST_FUNCTION_POINTER_DECLARATION) {
        node->as.fptr = (AstFunctionPointerDeclaration *)value;
    }
    else {
        printf("unknown ast_type in 'init_node': '%s'\n", ast_type_to_str(type));
        printf("you probably forgot to add this type to the if-else block.\n");
    }

    return node;
}

static AstDataType token_to_ast_data_type(Token token) {
    switch (token.type) {
        case TOKEN_INT: return AST_TYPE_INT;
        case TOKEN_CHAR: return AST_TYPE_CHAR;
        case TOKEN_VOID: return AST_TYPE_VOID;
        case TOKEN_SHORT: return AST_TYPE_SHORT;
        case TOKEN_DOUBLE: return AST_TYPE_DOUBLE;
        case TOKEN_LONG: return AST_TYPE_LONG;
        case TOKEN_FLOAT: return AST_TYPE_FLOAT;

        // defaults to int
        case TOKEN_UNSIGNED: return AST_TYPE_INT;
        case TOKEN_SIGNED: return AST_TYPE_INT;
        
        default: return AST_TYPE_INVALID;
    }
}

static int is_valid_type(Token token) {
    return token_to_ast_data_type(token) != AST_TYPE_INVALID;
}

static inline Token current_token(Parser *parser) {
    return parser->tokens[parser->current];
}

static inline void advance(Parser *parser) {
    parser->current++;
}

static inline void recede(Parser *parser) {
    parser->current--;
}

static inline int is_end(Parser *parser) {
    return current_token(parser).type == TOKEN_EOF;
}

static inline int match(TokenType type, Parser *parser) {
    return current_token(parser).type == type;
}

static inline void *parser_err(ParseErr err, Parser *parser) {
    parser->err = err;
    return NULL;
}

int len_helper(unsigned x) {
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

static void pretty_error(Parser *parser) {
    Token errTok;

    recede(parser);
    errTok = current_token(parser);

    if (errTok.lexeme == NULL) {
        advance(parser);
        for (int i = parser->current; i >= 0; i--) {
            if (parser->tokens[i].type != TOKEN_EOF) {
                errTok = parser->tokens[i];
                break;
            }
        }
    }

    printf("\n%s:%d\n", parser->file, errTok.line);
    printf("error: %s\n", parser_err_to_str(parser->err));

    // const int line_num_len = 6; // '   XX | ' is 6 characters
    printf("   %d | ", errTok.line);

    int char_pos = 0;
    int caret_pos = -1;

    for (int i = 0; parser->tokens[i].type != TOKEN_EOF; i++) {
        Token tok = parser->tokens[i];
        if (tok.line != errTok.line) continue;

        if (caret_pos == -1 &&
            tok.line == errTok.line &&
            strcmp(tok.lexeme, errTok.lexeme) == 0) {
            caret_pos = char_pos;
        }

        printf("%s", tok.lexeme);
        char_pos += strlen(tok.lexeme);

        if (tok.has_whitespace_after) {
            printf(" ");
            char_pos++;
        }
    }

    if (caret_pos == -1) caret_pos = char_pos;

    printf("\n     | ");
    for (int i = 0; i < caret_pos + 1; i++) {
        printf(" ");
    }
    printf("^\n");
}


// static int is_literal_or_ident(Parser *parser) {
//     return match(TOKEN_INTEGER_LITERAL, parser)
//         || match(TOKEN_CHAR_LITERAL, parser)
//         || match(TOKEN_FLOAT_LITERAL, parser)
//         || match(TOKEN_OCTAL_LITERAL, parser)
//         || match(TOKEN_BINARY_LITERAL, parser)
//         || match(TOKEN_STRING_LITERAL, parser)
//         || match(TOKEN_HEX_LITERAL, parser)
//         || match(TOKEN_IDENTIFIER, parser);
// }

static inline int expect(TokenType type, Parser *parser) {
    if (parser->tokens[parser->current].type == type) {
        advance(parser);
        return 1;
    }

    return 0;
}

static AstCallExpr *init_call_expr(char *identifier) {
    AstCallExpr *expr = malloc(sizeof(AstCallExpr));
    expr->identifier = strdup(identifier);

    return expr;
}

static AstBinaryExpr *init_binary_node(AstNode *left, Token op, AstNode *right) {
    AstBinaryExpr *binary = malloc(sizeof(AstBinaryExpr));
    binary->left = left;
    binary->op = op;
    binary->right = right;

    return binary;
}

static AstUnary *init_unary_node(AstNode *left, Token op, int is_postfix) {
    AstUnary *unary = malloc(sizeof(AstUnary));
    unary->left = left;
    unary->op = op;
    unary->is_postfix = is_postfix;

    return unary;
}

static AstArraySubscript *init_array_subscript(AstNode *base, AstNode* index) {
    AstArraySubscript *arr_sub = malloc(sizeof(AstArraySubscript));
    arr_sub->base = base;
    arr_sub->index = index;

    return arr_sub;
}

static AstNode *parse_primary(Parser *parser) {
    Token token = current_token(parser);
    
    if (token.type == TOKEN_LEFT_PAREN) {
        advance(parser);

        AstNode *expr = parse_expression(parser);
        if (!expect(TOKEN_RIGHT_PAREN, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        return expr;
    }

    advance(parser);

    if (token.type == TOKEN_INTEGER_LITERAL) {
        AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
        lit->value = atoi(token.lexeme);
        AstNode *node = init_node(lit, AST_LITERAL_INT);

        return node;
    }
    else if (token.type == TOKEN_HEX_LITERAL) {
        AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
        lit->value = (int)strtol(token.lexeme, NULL, 16);
        AstNode *node = init_node(lit, AST_LITERAL_INT);

        return node;
    }
    else if (token.type == TOKEN_BINARY_LITERAL) {
        AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
        
        int value = 0;
        for (int i = 0; token.lexeme[i] != '\0'; i++) {
            value <<= 1;
            if (token.lexeme[i] == '1') {
                value |= 1;
            } else if (token.lexeme[i] != '0') {
                value = 0;
                break;
            }
        }
    
        lit->value = value;
        AstNode *node = init_node(lit, AST_LITERAL_INT);
        return node;
    }
    else if (token.type == TOKEN_OCTAL_LITERAL) {
        AstLiteralInt *lit = malloc(sizeof(AstLiteralInt));
        lit->value = (int)strtol(token.lexeme, NULL, 8);
        AstNode *node = init_node(lit, AST_LITERAL_INT);
        return node;
    }
    else if (token.type == TOKEN_CHAR_LITERAL) {
        AstLiteralChar *lit = malloc(sizeof(AstLiteralChar));
        lit->value = token.lexeme[0];
        AstNode *node = init_node(lit, AST_LITERAL_CHAR);

        return node;

    }
    else if (token.type == TOKEN_IDENTIFIER) {
        char *name = strdup(token.lexeme);

        if (match(TOKEN_LEFT_PAREN, parser)) {
            advance(parser);
            
            if (!expect(TOKEN_RIGHT_PAREN, parser)) {
                return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
            }
        
            AstCallExpr *expr = init_call_expr(name);
            AstNode *node = init_node(expr, AST_CALL_EXPR);

            return node;
        }

        AstIdentifier *ident = malloc(sizeof(AstIdentifier));
        ident->name = name;
        AstNode *node = init_node(ident, AST_IDENTIFIER);

        return node;
    }
    else {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }
}

static AstNode *parse_postfix_unary(Parser *parser) {
    AstNode *expr = parse_primary(parser);

    while (1) {
        if (match(TOKEN_SQUARE_BRACKET_LEFT, parser)) {
            advance(parser);
            AstNode *index = parse_expression(parser);
            if (!index || !expect(TOKEN_SQUARE_BRACKET_RIGHT, parser)) {
                return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
            }

            AstArraySubscript *arr_sub = init_array_subscript(expr, index);
            expr = init_node(arr_sub, AST_ARR_SUBSCRIPT);
        }
        else if (match(TOKEN_INCREMENT, parser) || match(TOKEN_DECREMENT, parser)) {
            Token op = current_token(parser);
            advance(parser);

            AstUnary *unary = init_unary_node(expr, op, 1);
            expr = init_node(unary, AST_UNARY);
        }
        else {
            break;
        }
    }

    return expr;
}

static AstNode *parse_prefix_unary(Parser *parser) {
    if (match(TOKEN_PLUS, parser) || match(TOKEN_MINUS, parser) || 
        match(TOKEN_EXCLAMATION, parser) || match(TOKEN_BITWISE_NOT, parser) ||
        match(TOKEN_INCREMENT, parser) || match(TOKEN_DECREMENT, parser)
    ) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_prefix_unary(parser);
        if (!right) return NULL;

        AstUnary *unary = init_unary_node(right, op, 0);
        return init_node(unary, AST_UNARY);
    }

    return parse_postfix_unary(parser);
}

static AstNode *parse_cast(Parser *parser) {
    advance(parser);
    Token next_token = current_token(parser);
    recede(parser);

    if (match(TOKEN_LEFT_PAREN, parser) && is_valid_type(next_token)) {
        advance(parser);

        Token type = current_token(parser);
        advance(parser);

        int pointer_level = 0;
        while (match(TOKEN_STAR, parser)) {
            advance(parser);
            pointer_level++;
        }

        if (!expect(TOKEN_RIGHT_PAREN, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        AstNode *right = parse_expression(parser);
        if (!right) return NULL;

        AstCast *cast = malloc(sizeof(AstCast));
        cast->right = right;
        cast->type = type;
        cast->pointer_level = pointer_level;

        return init_node(cast, AST_CAST);
    }

    return parse_prefix_unary(parser);
}

static AstNode *parse_pointer_operator(Parser *parser) {
    if (match(TOKEN_BITWISE_AND, parser) || match(TOKEN_STAR, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_cast(parser);
        if (!right) return NULL;

        AstUnary *unary = init_unary_node(right, op, 0);
        return init_node(unary, AST_UNARY);
    }

    return parse_cast(parser);
}

static AstNode *parse_sizeof(Parser *parser) {
    if (match(TOKEN_SIZEOF, parser)) {
        Token op = current_token(parser);
        advance(parser);

        if (!expect(TOKEN_LEFT_PAREN, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        AstNode *right = parse_expression(parser);
        if (!right) return NULL;

        AstUnary *unary = init_unary_node(right, op, 0);

        if (!expect(TOKEN_RIGHT_PAREN, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        return init_node(unary, AST_UNARY);
    }

    return parse_pointer_operator(parser);
}

static AstNode *parse_factor(Parser *parser) {
    AstNode *left = parse_sizeof(parser);
    
    while (match(TOKEN_STAR, parser) || match(TOKEN_SLASH, parser) || match(TOKEN_MODULO, parser)) {
        Token op = current_token(parser);
        advance(parser);
        
        AstNode *right = parse_sizeof(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_term(Parser *parser) {
    AstNode *left = parse_factor(parser);

    while (match(TOKEN_PLUS, parser) || match(TOKEN_MINUS, parser)) {
        Token op = current_token(parser);
        advance(parser);
        
        AstNode *right = parse_factor(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_bitwise_shift(Parser *parser) {
    AstNode *left = parse_term(parser);

    while (match(TOKEN_BITWISE_LEFT_SHIFT, parser) || match(TOKEN_BITWISE_RIGHT_SHIFT, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_term(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_relational(Parser *parser) {
    AstNode *left = parse_bitwise_shift(parser);

    while (
        match(TOKEN_GREATER_THAN, parser) || match(TOKEN_LESS_THAN_EQUALS, parser) ||
        match(TOKEN_GREATER_THAN_EQUALS, parser) || match(TOKEN_LESS_THAN, parser)
    ) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_bitwise_shift(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_equality(Parser *parser) {
    AstNode *left = parse_relational(parser);

    while (match(TOKEN_EQUALS, parser) || match(TOKEN_NOT_EQUALS, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_relational(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_bitwise_and(Parser *parser) {
    AstNode *left = parse_equality(parser);
        
    while (match(TOKEN_BITWISE_AND, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_equality(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        return init_node(binary, AST_BINARY);
    }

    return left;

}

static AstNode *parse_bitwise_xor(Parser *parser) {
    AstNode *left = parse_bitwise_and(parser);
    
    while (match(TOKEN_BITWISE_XOR, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_bitwise_and(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        return init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_bitwise_or(Parser *parser) {
    AstNode *left = parse_bitwise_xor(parser);

    while (match(TOKEN_BITWISE_OR, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_bitwise_xor(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        return init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_logical_and(Parser *parser) {
    AstNode *left = parse_bitwise_or(parser);

    while (match(TOKEN_AND, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_bitwise_or(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_logical_or(Parser *parser) {
    AstNode *left = parse_logical_and(parser);

    while (match(TOKEN_OR, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_logical_and(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstTernary *init_ternary_node(AstNode *condition, AstNode *true_expr, AstNode *false_expr) {
    AstTernary *ternary = malloc(sizeof(AstTernary));
    ternary->condition = condition;
    ternary->true_expr = true_expr;
    ternary->false_expr = false_expr;
    
    return ternary;
}

static AstNode *parse_ternary(Parser *parser) {
    AstNode *left = parse_logical_or(parser);
    
    while (match(TOKEN_QUESTION, parser)) {
        advance(parser);

        AstNode *true_expr = parse_expression(parser);
        if (!true_expr) return NULL;

        if (!expect(TOKEN_COLON, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        AstNode *false_expr = parse_ternary(parser);
        if (!false_expr) return NULL;

        AstTernary *ternary = init_ternary_node(left, true_expr, false_expr);
        return init_node(ternary, AST_TERNARY);
    }

    return left;
}

static AstNode *parse_compound(Parser *parser) {
    AstNode *left = parse_ternary(parser);

    if (match(TOKEN_EQUALS, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_compound(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        return init_node(binary, AST_BINARY);
    }

    if (
        match(TOKEN_PLUS_EQUALS, parser) || match(TOKEN_MINUS_EQUALS, parser) || 
        match(TOKEN_STAR_EQUALS, parser) || match(TOKEN_SLASH_EQUALS, parser) ||
        match(TOKEN_MODULO_EQUALS, parser) || match(TOKEN_BITWISE_AND_EQUALS, parser) ||
        match(TOKEN_BITWISE_OR_EQUALS, parser) || match(TOKEN_BITWISE_XOR_EQUALS, parser) ||
        match(TOKEN_BITWISE_LEFT_SHIFT_EQUALS, parser) || match(TOKEN_BITWISE_RIGHT_SHIFT_EQUALS, parser)
    ) {
        Token compound_op = current_token(parser);
        advance(parser);

        AstNode *right = parse_compound(parser);
        if (!right) return NULL;
        
        TokenType binary_type;
        switch (compound_op.type) {
            case TOKEN_PLUS_EQUALS: binary_type = TOKEN_PLUS; break;
            case TOKEN_MINUS_EQUALS: binary_type = TOKEN_MINUS; break;
            case TOKEN_STAR_EQUALS: binary_type = TOKEN_STAR; break;
            case TOKEN_SLASH_EQUALS: binary_type = TOKEN_SLASH; break;
            case TOKEN_MODULO_EQUALS: binary_type = TOKEN_MODULO; break;
            case TOKEN_BITWISE_AND_EQUALS: binary_type = TOKEN_BITWISE_AND; break;
            case TOKEN_BITWISE_OR_EQUALS: binary_type = TOKEN_BITWISE_OR; break;
            case TOKEN_BITWISE_XOR_EQUALS: binary_type = TOKEN_BITWISE_XOR; break;
            case TOKEN_BITWISE_LEFT_SHIFT_EQUALS: binary_type = TOKEN_BITWISE_LEFT_SHIFT; break;
            case TOKEN_BITWISE_RIGHT_SHIFT_EQUALS: binary_type = TOKEN_BITWISE_RIGHT_SHIFT; break;
            default: return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        // x += 1;
        // x = x + 1;

        Token binary_tok = compound_op;
        binary_tok.type = binary_type;
        binary_tok.lexeme = "+";

        AstBinaryExpr *binary = init_binary_node(left, binary_tok, right);
        AstNode *node = init_node(binary, AST_BINARY);

        AstAssignment *assign = init_assignment(left->as.ident->name, node);
        return init_node(assign, AST_ASSIGNMENT);
    }

    return left;
}

static AstNode *parse_comma(Parser *parser) {
    AstNode *left = parse_compound(parser);
    if (parser->is_var_dec) return left;

    while (match(TOKEN_COMMA, parser)) {
        Token op = current_token(parser);
        advance(parser);

        AstNode *right = parse_compound(parser);
        if (!right) return NULL;
        
        AstBinaryExpr *binary = init_binary_node(left, op, right);
        left = init_node(binary, AST_BINARY);
    }

    return left;
}

static AstNode *parse_expression(Parser *parser) {
    return parse_comma(parser);
}

static AstVariableDeclaration *init_var_dec(AstDeclarator **declarators, int declarator_count) {
    AstVariableDeclaration *var_dec = (AstVariableDeclaration *)malloc(sizeof(AstVariableDeclaration));
    var_dec->declarators= declarators;
    var_dec->declarator_count = declarator_count;

    return var_dec;
}

static AstNode *parse_variable_declaration(Parser *parser, TypeSpecifier type_specs) {
    parser->is_var_dec = 1;
    advance(parser);

    AstDeclarator **declarators = malloc(sizeof(AstDeclarator));
    int declarator_count = 0;
    int declarator_capacity = 1;

    recede(parser);
    do {
        advance(parser);

        int pointer_level = 0;
        while (match(TOKEN_STAR, parser)) {
            pointer_level++;
            advance(parser);
        }

        Token id = current_token(parser);
        if (!expect(TOKEN_IDENTIFIER, parser)) {
            parser->is_var_dec = 0;
            return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
        }

        AstNode *initializer = NULL;
        if (match(TOKEN_SINGLE_EQUALS, parser)) {
            advance(parser);

            AstNode *expr = parse_expression(parser);
            if (!expr) {
                parser->is_var_dec = 0;
                return NULL;
            }
            initializer = expr;
        }

        AstDeclarator *declarator = malloc(sizeof(AstDeclarator));
        declarator->identifier = strdup(id.lexeme);
        declarator->pointer_level = pointer_level;
        declarator->value = initializer;

        if (declarator_count >= declarator_capacity) {
            declarator_capacity *= 2;
            declarators = realloc(declarators, sizeof(AstDeclarator) * declarator_capacity);
        }

        declarators[declarator_count++] = declarator;

    } while (match(TOKEN_COMMA, parser));

    if (!expect(TOKEN_SEMICOLON, parser)) {
        parser->is_var_dec = 0;
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstVariableDeclaration *var_dec = init_var_dec(declarators, declarator_count);
    var_dec->type_specifier = type_specs;

    AstNode *node = init_node(var_dec, AST_VARIABLE_DECLARATION);

    parser->is_var_dec = 0;
    return node;
}

static AstFunctionDeclaration *init_function_node(AstNode **body, int body_count, char *identifier, AstFunctionParameter **params, int params_count, int is_void_params) {
    AstFunctionDeclaration *func = (AstFunctionDeclaration *)malloc(sizeof(AstFunctionDeclaration));
    func->body = body;
    func->body_count = body_count;
    func->identifier = strdup(identifier);
    func->params = params;
    func->params_count = params_count;
    func->is_void_params = is_void_params;

    return func;
}

static TypeSpecifier init_type_specifier() {
    TypeSpecifier specs;
    specs.is_const = 0;
    specs.is_short = 0;
    specs.is_signed = 0;
    specs.is_static = 0;
    specs.is_unsigned = 0;
    specs.is_volatile = 0;
    specs.long_count = 0;
    specs.pointer_level = 0;

    return specs;
}

static TypeSpecifier parse_type_specifiers(Parser *parser) {
    TypeSpecifier specs = init_type_specifier();
    specs.pointer_level = 0;

    while (1) {
        if (match(TOKEN_CONST, parser)) {
            specs.is_const = 1;
            advance(parser);
        }
        else if (match(TOKEN_STATIC, parser)) {
            specs.is_static = 1;
            advance(parser);
        }
        else if (match(TOKEN_VOLATILE, parser)) {
            specs.is_volatile = 1;
            advance(parser);
        }
        else if (match(TOKEN_UNSIGNED, parser)) {
            specs.is_unsigned = 1;
            advance(parser);
        }
        else if (match(TOKEN_SIGNED, parser)) {
            specs.is_signed = 1;
            advance(parser);
        }
        else if (match(TOKEN_LONG, parser)) {
            specs.long_count++;
            advance(parser);
        }
        else if (match(TOKEN_SHORT, parser)) {
            specs.is_short = 1;
            advance(parser);
        }
        else if (
            match(TOKEN_INT, parser) || match(TOKEN_CHAR, parser) ||
            match(TOKEN_FLOAT, parser) || match(TOKEN_DOUBLE, parser) ||
            match(TOKEN_VOID, parser)
         ) {
            specs.type = current_token(parser).type;
            advance(parser);
        }
        else if (match(TOKEN_STAR, parser)) {
            specs.pointer_level += 1;
            advance(parser);
        }
        else {
            break;
        }
    }

    // if no non-combinable type has been declared, such with:
    //   short x;
    //   long long x;
    //
    //  specs.type will hold garbage, so it is assigned here:
    if (specs.type != TOKEN_CHAR) {
        if (specs.is_signed) {
            specs.type = TOKEN_INT;
        }
        if (specs.is_unsigned) {
            specs.type = TOKEN_INT;
        }
        if (specs.is_short) {
            specs.type = TOKEN_SHORT;
        } 
        if (specs.long_count != 0 && specs.type != TOKEN_DOUBLE) {
            specs.type = TOKEN_LONG;
        }
    }

    
    return specs;
}

static AstFunctionParameter *init_func_parameter(char *id, TypeSpecifier type_specs) {
    AstFunctionParameter *param = malloc(sizeof(AstFunctionParameter));
    param->name = strdup(id);
    param->type_specifier = type_specs;

    return param;
}

static AstNode *parse_function(Parser *parser, TypeSpecifier type_specs) {
    advance(parser);

    Token identifier_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstFunctionParameter **params = malloc(sizeof(AstFunctionParameter *));
    if (!params) {
        return parser_err(PARSE_ERR_OUT_OF_MEMORY, parser);
    }

    int capacity = 1;
    int params_count = 0;
    int is_void_params = 0;

    if (match(TOKEN_VOID, parser)) {
        is_void_params = 1;
        advance(parser);
    } else {
        recede(parser);

        do {
            advance(parser);
            if (match(TOKEN_RIGHT_PAREN, parser)) break;
    
            TypeSpecifier type_specs = parse_type_specifiers(parser);
    
            Token id = current_token(parser);
            if (!match(TOKEN_IDENTIFIER, parser)) {
                return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
            }
            advance(parser);
    
            AstFunctionParameter *param = init_func_parameter(id.lexeme, type_specs);
            if (params_count >= capacity) {
                capacity *= 2;
                params = realloc(params, sizeof(AstFunctionParameter *) * capacity);
            }
            params[params_count++] = param;
        } while (match(TOKEN_COMMA, parser));
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);

        AstFunctionDeclaration *func = init_function_node(NULL, 0, identifier_token.lexeme, params, params_count, is_void_params);
        AstNode *node = init_node(func, AST_FUNCTION);

        return node;
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_statement_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *) * body_capacity);
    
    while (!match(TOKEN_RIGHT_BRACE, parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) return NULL;

        if (body_statement_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, sizeof(AstNode *) * body_capacity);
        }
        body[body_statement_count++] = node;
    }

    AstFunctionDeclaration *func = init_function_node(
        body, body_statement_count, identifier_token.lexeme, 
        params, params_count, is_void_params
    );
    func->type_specifier = type_specs;

    AstNode *node = init_node(func, AST_FUNCTION);

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        // free some shit here
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    return node;
}

static AstReturn *init_return(AstNode *value) {
    AstReturn *ret = (AstReturn *)malloc(sizeof(AstReturn));
    ret->value = value;

    return ret;
}

static AstNode *parse_return(Parser *parser) {
    advance(parser);

    AstNode *return_expr = parse_expression(parser);
    if (!return_expr) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstReturn *ret = init_return(return_expr);
    AstNode *node = init_node(ret, AST_RETURN);

    return node;
}

static AstInlineAsmBlock *init_inline_asm_node(char **asm_lines, int line_count) {
    AstInlineAsmBlock *asm_inl = malloc(sizeof(AstInlineAsmBlock));
    asm_inl->lines = asm_lines;
    asm_inl->line_count = line_count;

    return asm_inl;
}

static AstNode *parse_inline_asm(Parser *parser) {
    advance(parser);
    
    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int capacity = 1;
    int count = 0;
    char **asm_lines = malloc(sizeof(char *) * capacity);

    recede(parser);
    do {
        advance(parser);
        
        if (!match(TOKEN_STRING_LITERAL, parser)) {
            free(asm_lines);
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        Token str_token = current_token(parser);
        
        if (count >= capacity) {
            capacity *= 2;
            char **new_lines = realloc(asm_lines, sizeof(char *) * capacity);
            if (!new_lines) {
                free(asm_lines);
                return parser_err(PARSE_ERR_OUT_OF_MEMORY, parser);
            }
            asm_lines = new_lines;
        }

        asm_lines[count++] = strdup(str_token.lexeme);
        advance(parser);

    } while (match(TOKEN_COMMA, parser));

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        for (int i = 0; i < count; i++) free(asm_lines[i]);
        free(asm_lines);
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        for (int i = 0; i < count; i++) free(asm_lines[i]);
        free(asm_lines);
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstInlineAsmBlock *asm_inl = init_inline_asm_node(asm_lines, count);
    AstNode *node = init_node(asm_inl, AST_INLINE_ASM_BLOCK);

    return node;
}

// static AstDeclarator *init_declarator(char *name, int pointer_level) {
//     AstDeclarator *declarator = malloc(sizeof(AstDeclarator));
//     declarator->identifier = name;
//     declarator->pointer_level = pointer_level;

//     return declarator;
// }

// uhhhh
// static AstNode *parse_declarator(Parser *parser) {
//     int pointer_depth = 0;
    
//     while (match(TOKEN_STAR, parser)) {
//         advance(parser);
//         pointer_depth++;
//     }

//     if (!match(TOKEN_IDENTIFIER, parser)) {
//         return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
//     }

//     Token ident = current_token(parser);
//     advance(parser);

//     AstNode *node = init_declarator(ident.lexeme, pointer_depth);
//     return node;
// }

static AstNode * init_array_declaration(char *identifier, TypeSpecifier type_specs, AstNode *dimensions, int dimension_count) {
    AstArrayDeclaration *arr_decl = malloc(sizeof(AstArrayDeclaration));
    arr_decl->identifier = strdup(identifier);
    arr_decl->type_specs = type_specs;
    arr_decl->dimension_count = dimension_count;
    arr_decl->dimensions = dimensions;

    return arr_decl;
}

static AstNode *parse_array_declaration(Parser *parser, TypeSpecifier type_specs) {
    advance(parser);

    Token array_identifier = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    AstNode **dimensions = malloc(sizeof(AstNode *));
    int dimension_count = 0;
    int capacity = 1;

    do {
        advance(parser);
        AstNode *dimension = parse_expression(parser);
        if (!dimension) return NULL;

        if (!expect(TOKEN_SQUARE_BRACKET_RIGHT, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        if (dimension_count >= capacity) {
            capacity *= 2;
            dimensions = realloc(dimensions, capacity * sizeof(AstNode *));
        }
        dimensions[dimension_count++] = dimension;

    } while (match(TOKEN_SQUARE_BRACKET_LEFT, parser));

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstArrayDeclaration *arr_decl = init_array_declaration(array_identifier.lexeme, type_specs, dimensions, 
    dimension_count);
    AstNode *node = init_node(arr_decl, AST_ARRAY_DECLARATION);

    return node;
}

static AstFunctionPointerDeclaration *init_function_pointer(char *identifier, TypeSpecifier return_type_specs, TypeSpecifier *param_type_specs) {
    AstFunctionPointerDeclaration *fptr = malloc(sizeof(AstFunctionPointerDeclaration));
    fptr->identifier = strdup(identifier);
    fptr->param_type_specs = param_type_specs;
    fptr->return_type_specs = return_type_specs;

    return fptr;
}

static AstNode *parse_function_pointer_declaration(Parser *parser, TypeSpecifier type_specs) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_STAR, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    Token identifier = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }
    advance(parser);

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    // types

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstFunctionPointerDeclaration *fptr = init_function_pointer(identifier.lexeme, type_specs, NULL);
    AstNode *node = init_node(fptr, AST_FUNCTION_POINTER_DECLARATION);

    return node;
}

static AstNode *parse_type_statement(Parser *parser) {
    TypeSpecifier type_specs = parse_type_specifiers(parser);

    if (match(TOKEN_LEFT_PAREN, parser)) {
        recede(parser);
        return parse_function_pointer_declaration(parser, type_specs);
    }

    // advance to the '(' at the start of a function
    advance(parser);

    if (match(TOKEN_LEFT_PAREN, parser)) {
        recede(parser);
        recede(parser);
        return parse_function(parser, type_specs);
    }
    else if (match(TOKEN_SQUARE_BRACKET_LEFT, parser)) {
        recede(parser);
        recede(parser);
        return parse_array_declaration(parser, type_specs);
    }
    else if (
        match(TOKEN_SINGLE_EQUALS, parser) || match(TOKEN_COMMA, parser) || 
        match(TOKEN_IDENTIFIER, parser) || match(TOKEN_STAR, parser)
    ) {
        recede(parser);
        recede(parser);

        if (match(TOKEN_VOID, parser)) {
            return parser_err(PARSE_ERR_VOID_NOT_ALLOWED, parser);
        }

        return parse_variable_declaration(parser, type_specs);
    }
    else if (match(TOKEN_SEMICOLON, parser)) {
        recede(parser);
        recede(parser);

        if (match(TOKEN_VOID, parser)) {
            return parser_err(PARSE_ERR_VOID_NOT_ALLOWED, parser);
        }

        return parse_variable_declaration(parser, type_specs);
    }
    else {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }
}

static AstNode *parse_empty_expression(Parser *parser) {
    AstNode *expr = parse_expression(parser);
    if (!expr) return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        free_node(expr);
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    return expr;
}

static AstAssignment *init_assignment(char *identifier, AstNode *value) {
    AstAssignment *assign = malloc(sizeof(AstAssignment));
    assign->identifier = strdup(identifier);
    assign->value = value;

    return assign;
}

static AstNode *parse_assignment(Parser *parser) {
    Token id = current_token(parser);
    advance(parser);

    if (!expect(TOKEN_SINGLE_EQUALS, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *expr = parse_expression(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstAssignment *assign = init_assignment(id.lexeme, expr);
    AstNode *node = init_node(assign, AST_ASSIGNMENT);

    return node;
}

static AstIfStatement *init_if_statement(AstNode **body, AstNode **else_body, AstNode *condition, int body_count, int else_body_count) {
    AstIfStatement *if_stmt = malloc(sizeof(AstIfStatement));
    if_stmt->condition = condition;
    if_stmt->body = body;
    if_stmt->body_count = body_count;
    if_stmt->else_body = else_body;
    if_stmt->else_body_count = else_body_count;

    return if_stmt;
}

static AstNode *parse_if(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *if_condition = parse_expression(parser);
    if (!if_condition) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *));

    do {
        if (match(TOKEN_RIGHT_BRACE, parser)) break;
        AstNode *statement = parse_statement(parser);

        if (body_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, body_capacity * sizeof(AstNode *));
        }
        body[body_count++] = statement;

    } while (!match(TOKEN_RIGHT_BRACE, parser));

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int else_body_count = 0;
    int else_body_capacity = 1;
    AstNode **else_body = malloc(sizeof(AstNode *));

    int has_else = 0;

    if (match(TOKEN_ELSE, parser)) {
        has_else = 1;
        advance(parser);
        
        if (!expect(TOKEN_LEFT_BRACE, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }
            
        do {
            if (match(TOKEN_RIGHT_BRACE, parser)) break;
            
            AstNode *statement = parse_statement(parser);

            if (else_body_count >= else_body_capacity) {
                else_body_capacity *= 2;
                else_body = realloc(else_body, else_body_capacity * sizeof(AstNode *));
            }
            else_body[else_body_count++] = statement;

        } while (!match(TOKEN_RIGHT_BRACE, parser));
    }

    if (has_else) {
        advance(parser);
    }

    AstIfStatement *if_stmt = init_if_statement(body, else_body, if_condition, body_count, else_body_count);
    AstNode *node = init_node(if_stmt, AST_IF);

    return node;
}

static AstWhile *init_while(AstNode *condition, AstNode **body, int body_count) {
    AstWhile *while_stmt = malloc(sizeof(AstWhile));
    while_stmt->condition = condition;
    while_stmt->body = body;
    while_stmt->body_count = body_count;

    return while_stmt;
}

static AstNode *parse_while(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *condition = parse_expression(parser);
    if (!condition) {
        return parser_err(PARSE_ERR_EXPECTED_EXPRESSION, parser);
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *));

    do {
        if (match(TOKEN_RIGHT_BRACE, parser)) break;
        AstNode *statement = parse_statement(parser);

        if (body_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, body_capacity * sizeof(AstNode *));
        }
        body[body_count++] = statement;

    } while (!match(TOKEN_RIGHT_BRACE, parser));

    advance(parser);

    AstWhile *while_stmt = init_while(condition, body, body_count);
    AstNode *node = init_node(while_stmt, AST_WHILE);

    return node;
}

static AstNode *parse_break(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstBreak *brk = malloc(sizeof(AstBreak));
    AstNode *node = init_node(brk, AST_BREAK);
    brk->dummy = 0;

    return node;
}

static AstNode *parse_continue(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstContinue *cont = malloc(sizeof(AstContinue));
    AstNode *node = init_node(cont, AST_CONTINUE);
    cont->dummy = 0;

    return node;
}

static AstFor *init_for(AstNode *initializer, AstNode *condition, AstNode *alteration, AstNode *block) {
    AstFor *for_stmt = malloc(sizeof(AstFor));
    for_stmt->condition = condition;
    for_stmt->initializer = initializer;
    for_stmt->alteration = alteration;
    for_stmt->block = block;

    return for_stmt;
}

static AstBlock *init_block(AstNode **body, int body_count) {
    AstBlock *block = malloc(sizeof(AstBlock));
    block->body = body;
    block->body_count = body_count;

    return block;
}

static AstNode *parse_block(Parser *parser) {
    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int body_count = 0;
    int body_capacity = 1;
    AstNode **body = malloc(sizeof(AstNode *));

    do {
        if (match(TOKEN_RIGHT_BRACE, parser)) break;
        AstNode *statement = parse_statement(parser);

        if (body_count >= body_capacity) {
            body_capacity *= 2;
            body = realloc(body, body_capacity * sizeof(AstNode *));
        }
        body[body_count++] = statement;

    } while (!match(TOKEN_RIGHT_BRACE, parser));

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstBlock *block = init_block(body, body_count);
    AstNode *node = init_node(block, AST_BLOCK);

    return node;
}

static AstNode *parse_for(Parser *parser) {
    advance(parser);

    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *initializer = NULL;
    AstNode *condition = NULL;
    AstNode *alteration = NULL;

    if (match(TOKEN_SEMICOLON, parser)) {
        advance(parser);
    } else {
        AstNode *expr = parse_statement(parser);
        if (!expr) return NULL;

        initializer = expr;
    }

    if (match(TOKEN_SEMICOLON, parser)) {
        // do not advance for this semicolon, there are only two in a for loop construct
    } else {
        AstNode *expr = parse_expression(parser);
        if (!expr) return NULL;

        condition = expr;
    }
    advance(parser);

    if (!match(TOKEN_RIGHT_PAREN, parser)) {
        AstNode *expr = parse_expression(parser);
        if (!expr) return NULL;

        alteration = expr;
    }

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *block = parse_block(parser);
    if (!block) return NULL;

    AstFor *for_stmt = init_for(initializer, condition, alteration, block);
    AstNode *node = init_node(for_stmt, AST_FOR);

    return node; 
}

static AstDoWhile *init_do_while(AstNode *condition, AstNode *block) {
    AstDoWhile *do_while = malloc(sizeof(AstDoWhile));
    do_while->condition = condition;
    do_while->block = block->as.block;

    return do_while;
}

static AstNode *parse_do_while(Parser *parser) {
    advance(parser);

    AstNode *block = parse_block(parser);
    if (!block) return NULL;

    if (!expect(TOKEN_WHILE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }
    
    if (!expect(TOKEN_LEFT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    AstNode *condition = parse_expression(parser);

    if (!expect(TOKEN_RIGHT_PAREN, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstDoWhile *do_while = init_do_while(condition, block);
    AstNode *node = init_node(do_while, AST_DO_WHILE);

    return node;
}

static AstStruct *init_struct(char *name, AstNode **fields, int field_count) {
    AstStruct *a_struct = malloc(sizeof(AstStruct));
    a_struct->name = strdup(name);
    a_struct->fields = fields;
    a_struct->field_count = field_count;

    return a_struct;
}

static AstUnion *init_union(char *name, AstNode **fields, int field_count) {
    AstUnion *a_union = malloc(sizeof(AstUnion));
    a_union->name = strdup(name);
    a_union->fields = fields;
    a_union->field_count = field_count;

    return a_union;
}

static AstNode *parse_struct_or_union(Parser *parser) {
    int is_union = 0;
    if (match(TOKEN_UNION, parser)) {
        is_union = 1;
    }
    advance(parser);

    Token name_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int member_capacity = 1;
    int member_count = 0;
    AstNode **members = malloc(sizeof(AstNode *) * member_capacity);

    do {
        AstNode *member = parse_statement(parser);
        if (member->type != AST_VARIABLE_DECLARATION) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }

        if (member_count >= member_capacity) {
            member_capacity *= 2;
            members = realloc(members, sizeof(AstNode *) * member_capacity);
        }
        members[member_count++] = member;

    } while (!match(TOKEN_RIGHT_BRACE, parser));
    advance(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstNode *node;
    if (!is_union) {
        AstStruct *a_struct = init_struct(name_token.lexeme, members, member_count);
        node = init_node(a_struct, AST_STRUCT);
    } else {
        AstUnion *a_union = init_union(name_token.lexeme, members, member_count);
        node = init_node(a_union, AST_UNION);
    }

    return node;
}

static AstEnum *init_enum(char *name, AstEnumValue **values, int value_count) {
    AstEnum *an_enum = malloc(sizeof(AstEnum));
    an_enum->name = strdup(name);
    an_enum->values = values;
    an_enum->value_count = value_count;

    return an_enum;
}

static AstNode *parse_enum(Parser *parser) {
    advance(parser);

    Token enum_name_token = current_token(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    advance(parser);

    if (!expect(TOKEN_LEFT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    int value_capacity = 1;
    int value_count = 0;
    AstEnumValue **values = malloc(sizeof(AstEnumValue) * value_capacity);

    int current_value = 0;
    while (!match(TOKEN_RIGHT_BRACE, parser)) {
        if (!match(TOKEN_IDENTIFIER, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }
        
        Token identifier = current_token(parser);
        advance(parser);

        int has_explicit_value = 0;
        int enum_value = current_value;

        if (match(TOKEN_SINGLE_EQUALS, parser)) {
            advance(parser);

            if (!match(TOKEN_INTEGER_LITERAL, parser)) {
                return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
            }

            enum_value = atoi(current_token(parser).lexeme);
            has_explicit_value = 1;
            advance(parser);
        }

        AstEnumValue *enum_val = malloc(sizeof(AstEnumValue));
        enum_val->name = strdup(identifier.lexeme);
        enum_val->explicit_value = has_explicit_value;
        enum_val->value = enum_value;

        values[value_count++] = enum_val;
        current_value = enum_value + 1;

        if (match(TOKEN_COMMA, parser)) {
            advance(parser);
        } else if (!match(TOKEN_RIGHT_BRACE, parser)) {
            return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
        }
    }

    if (!expect(TOKEN_RIGHT_BRACE, parser)) {
        return parser_err(PARSE_ERR_INVALID_SYNTAX, parser);
    }

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstEnum *an_enum = init_enum(enum_name_token.lexeme, values, value_count);
    AstNode *node = init_node(an_enum, AST_ENUM);

    return node;
}

static AstTypedef *init_typedef(char *identifier, TypeSpecifier type_specs) {
    AstTypedef *type_def = malloc(sizeof(AstTypedef));
    type_def->identifier = strdup(identifier);
    type_def->type_specs = type_specs;

    return type_def;
}

static AstNode *parse_typedef(Parser *parser) {
    advance(parser);

    TypeSpecifier type_specs = parse_type_specifiers(parser);
    if (!match(TOKEN_IDENTIFIER, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_IDENTIFIER, parser);
    }
    Token identifier = current_token(parser);
    advance(parser);

    if (!expect(TOKEN_SEMICOLON, parser)) {
        return parser_err(PARSE_ERR_EXPECTED_SEMICOLON, parser);
    }

    AstTypedef *type_def = init_typedef(identifier.lexeme, type_specs);
    AstNode *node = init_node(type_def, AST_TYPEDEF);

    return node;
}

static AstNode *parse_typedef_declaration(Parser *parser) {

    TypeSpecifier type_specs = init_type_specifier();
    type_specs.type = current_token(parser).type;

    return parse_variable_declaration(parser, type_specs);
}

static AstNode *parse_statement(Parser *parser) {
    if (is_valid_type(current_token(parser))) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_CONST, parser)) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_STATIC, parser)) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_VOLATILE, parser)) {
        return parse_type_statement(parser);
    }
    else if (match(TOKEN_RETURN, parser)) {
        return parse_return(parser);
    }
    else if (match(TOKEN_ASM, parser)) {
        return parse_inline_asm(parser);
    }
    else if (match(TOKEN_IF, parser)) {
        return parse_if(parser);
    }
    else if (match(TOKEN_WHILE, parser)) {
        return parse_while(parser);
    }
    else if (match(TOKEN_BREAK, parser)) {
        return parse_break(parser);
    }
    else if (match(TOKEN_CONTINUE, parser)) {
        return parse_continue(parser);
    }
    else if (match(TOKEN_FOR, parser)) {
        return parse_for(parser);
    }
    else if (match(TOKEN_DO, parser)) {
        return parse_do_while(parser);
    }
    else if (match(TOKEN_STRUCT, parser)) {
        return parse_struct_or_union(parser);
    }
    else if (match(TOKEN_ENUM, parser)) {
        return parse_enum(parser);
    }
    else if (match(TOKEN_UNION, parser)) {
        return parse_struct_or_union(parser);
    }
    else if (match(TOKEN_TYPEDEF, parser)) {
        return parse_typedef(parser);
    }
    else if (match(TOKEN_IDENTIFIER, parser)) {
        advance(parser);

        if (match(TOKEN_SINGLE_EQUALS, parser)) {
            recede(parser);
            return parse_assignment(parser);
        }
        else if (match(TOKEN_IDENTIFIER, parser)) {
            recede(parser);
            return parse_typedef_declaration(parser);
        }
        else {
            recede(parser);
            return parse_empty_expression(parser);
        }
    }
    else {
        return parse_empty_expression(parser);
    }
}

void parse_ast(Parser *parser) {
    while (!is_end(parser)) {
        AstNode *node = parse_statement(parser);
        if (!node) {
            break;
        }
        
        if (parser->node_count >= parser->node_capacity) {
            parser->node_capacity *= 2;
            parser->tree = realloc(parser->tree, sizeof(AstNode *) * parser->node_capacity);
        }
    
        parser->tree[parser->node_count++] = node;

        if (is_end(parser)) break;
    }

    if (parser->err != NO_PARSER_ERROR) {
        pretty_error(parser);
        return;
    }

    if (parser->debug) parser_print(parser);
}