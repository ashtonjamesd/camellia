#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "lexer_tests.h"
#include "ast_tests.h"

void setUp() {}
void tearDown() {}

void test_declare_void_empty_body_function_implicit_int() {
    Token tokens[] = {
        {"main", TOKEN_IDENTIFIER},
        {"(", TOKEN_LEFT_PAREN},
        {")", TOKEN_RIGHT_PAREN},
        {";", TOKEN_SEMICOLON},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == NO_PARSER_ERROR);
    TEST_ASSERT_TRUE(parser->node_count == 1);

    ASSERT_AST_FUNCTION(0, "main", AST_TYPE_INT);
    TEST_ASSERT_NULL(parser->tree[0]->as.func->body);
}

void test_declare_void_with_return_function() {
    Token tokens[] = {
        {"int", TOKEN_INT},
        {"main", TOKEN_IDENTIFIER},
        {"(", TOKEN_LEFT_PAREN},
        {")", TOKEN_RIGHT_PAREN},
        {";", TOKEN_SEMICOLON},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == NO_PARSER_ERROR);
    TEST_ASSERT_TRUE(parser->node_count == 1);

    ASSERT_AST_FUNCTION(0, "main", AST_TYPE_INT);
    TEST_ASSERT_NULL(parser->tree[0]->as.func->body);
}

void test_define_int_empty_body_function() {
    Token tokens[] = {
        {"int", TOKEN_INT},
        {"main", TOKEN_IDENTIFIER},
        {"(", TOKEN_LEFT_PAREN},
        {")", TOKEN_RIGHT_PAREN},
        {"{", TOKEN_LEFT_BRACE},
        {"return", TOKEN_RETURN},
        {"0", TOKEN_INTEGER_LITERAL},
        {";", TOKEN_SEMICOLON},
        {"}", TOKEN_RIGHT_BRACE},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == NO_PARSER_ERROR);
    TEST_ASSERT_TRUE(parser->node_count == 1);

    ASSERT_AST_FUNCTION(0, "main", AST_TYPE_INT);
    ASSERT_RETURN(parser->tree[0]->as.func->body[0], AST_LITERAL_INT, 0);
}

void test_define_char_empty_body_function() {
    Token tokens[] = {
        {"int", TOKEN_INT},
        {"main", TOKEN_IDENTIFIER},
        {"(", TOKEN_LEFT_PAREN},
        {")", TOKEN_RIGHT_PAREN},
        {"{", TOKEN_LEFT_BRACE},
        {"return", TOKEN_RETURN},
        {"a", TOKEN_CHAR_LITERAL},
        {";", TOKEN_SEMICOLON},
        {"}", TOKEN_RIGHT_BRACE},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == NO_PARSER_ERROR);
    TEST_ASSERT_TRUE(parser->node_count == 1);

    ASSERT_AST_FUNCTION(0, "main", AST_TYPE_INT);
    ASSERT_RETURN(parser->tree[0]->as.func->body[0], AST_LITERAL_CHAR, 'a');
}

int main(void) {
    RUN_TEST(test_define_int_empty_body_function);
    RUN_TEST(test_declare_void_with_return_function);
    RUN_TEST(test_define_char_empty_body_function);
    // RUN_TEST(test_declare_void_empty_body_function_implicit_int);
}