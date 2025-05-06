#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "lexer_tests.h"

void setUp() {}
void tearDown() {}

void test_declare_int() {
    Lexer *lexer = init_lexer("int x;");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 3);

    ASSERT_TOKEN(0, TOKEN_INT, "int");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SEMICOLON, ";");

    free(lexer);
}

void test_declare_int_with_initialization() {
    Lexer *lexer = init_lexer("int x = 10;");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 5);

    ASSERT_TOKEN(0, TOKEN_INT, "int");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(3, TOKEN_INTEGER_LITERAL, "10");
    ASSERT_TOKEN(4, TOKEN_SEMICOLON, ";");

    free(lexer);
}

void test_declare_multiple_int() {
    Lexer *lexer = init_lexer("int x, y, z;");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 7);

    ASSERT_TOKEN(0, TOKEN_INT, "int");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_COMMA, ",");
    ASSERT_TOKEN(3, TOKEN_IDENTIFIER, "y");
    ASSERT_TOKEN(4, TOKEN_COMMA, ",");
    ASSERT_TOKEN(5, TOKEN_IDENTIFIER, "z");
    ASSERT_TOKEN(6, TOKEN_SEMICOLON, ";");

    free(lexer);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_declare_int);
    RUN_TEST(test_declare_int_with_initialization);
    RUN_TEST(test_declare_multiple_int);

    return UNITY_END();
}