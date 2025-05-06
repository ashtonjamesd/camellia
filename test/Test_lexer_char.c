#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "lexer_tests.h"

void setUp() {}
void tearDown() {}

void test_declare_char() {
    Lexer *lexer = init_lexer("char x;");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 3);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SEMICOLON, ";");

    free(lexer);
}

void test_declare_char_with_initialization() {
    Lexer *lexer = init_lexer("char x = 'a';");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 5);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(3, TOKEN_CHAR_LITERAL, "a");
    ASSERT_TOKEN(4, TOKEN_SEMICOLON, ";");

    free(lexer);
}

void test_declare_escape_char() {
    Lexer *lexer = init_lexer("char x = '\t';");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->token_count == 5);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(3, TOKEN_CHAR_LITERAL, "\t");
    ASSERT_TOKEN(4, TOKEN_SEMICOLON, ";");

    free(lexer);
}

void test_declare_invalid_escape_char() {
    Lexer *lexer = init_lexer("char x = '\a';");
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);

    for (int i = 0; i < lexer->token_count; i++) {
        printf("%d '%s': %s\n", i, lexer->tokens[i].lexeme, token_type_to_str(lexer->tokens[i].type));
    }

    free(lexer);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_declare_char);
    RUN_TEST(test_declare_char_with_initialization);
    RUN_TEST(test_declare_escape_char);
    RUN_TEST(test_declare_invalid_escape_char);

    return UNITY_END();
}