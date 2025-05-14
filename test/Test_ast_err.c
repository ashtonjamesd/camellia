#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "lexer_tests.h"
#include "ast_tests.h"

void setUp() {}
void tearDown() {}

void test_expect_identifier_int() {
    Token tokens[] = {
        {"int", TOKEN_INT},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == PARSE_ERR_EXPECTED_IDENTIFIER);
}

void test_expect_semicolon_int() {
    Token tokens[] = {
        {"int", TOKEN_INT},
        {"x", TOKEN_IDENTIFIER},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == PARSE_ERR_EXPECTED_SEMICOLON);
}

void test_invalid_void_token() {
    Token tokens[] = {
        {"void", TOKEN_VOID},
        {"x", TOKEN_IDENTIFIER},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == PARSE_ERR_EXPECTED_SEMICOLON);
}

void test_invalid_void_token_with_equals() {
    Token tokens[] = {
        {"void", TOKEN_VOID},
        {"x", TOKEN_IDENTIFIER},
        {"=", TOKEN_SINGLE_EQUALS},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == PARSE_ERR_VOID_NOT_ALLOWED);
}

void test_invalid_func() {
    Token tokens[] = {
        {"void", TOKEN_VOID},
        {"x", TOKEN_IDENTIFIER},
        {"=", TOKEN_SINGLE_EQUALS},
        {NULL, TOKEN_EOF},
    };

    Parser *parser = init_parser(tokens, 0, "");
    parse_ast(parser);

    TEST_ASSERT_TRUE(parser->err == PARSE_ERR_VOID_NOT_ALLOWED);
}

int main(void) {
    RUN_TEST(test_expect_identifier_int);
    RUN_TEST(test_expect_semicolon_int);
    RUN_TEST(test_invalid_void_token);
    RUN_TEST(test_invalid_void_token_with_equals);
}