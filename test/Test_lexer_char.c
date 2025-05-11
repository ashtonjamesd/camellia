#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "lexer_tests.h"

void setUp() {}
void tearDown() {}

void test_declare_char() {
    Lexer *lexer = init_lexer("char x;", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 4);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(3, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_char_with_initialization() {
    Lexer *lexer = init_lexer("char x = 'a';", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 6);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(3, TOKEN_CHAR_LITERAL, "a");
    ASSERT_TOKEN(4, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(5, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_escape_char() {
    Lexer *lexer = init_lexer("char x = '\t';", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 6);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(3, TOKEN_CHAR_LITERAL, "\t");
    ASSERT_TOKEN(4, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(5, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_invalid_escape_char() {
    Lexer *lexer = init_lexer("char x = '\\c';", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == INVALID_ESCAPE_SEQUENCE);

    free(lexer);
}

void test_declare_too_many_chars_in_char_literal() {
    Lexer *lexer = init_lexer("char x = 'aa';", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == TOO_MANY_CHARS_IN_CHAR_LITERAL);

    free(lexer);
}

void test_declare_empty_char_pointer() {
    Lexer *lexer = init_lexer("char *x = \"\";", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 7);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_STAR, "*");
    ASSERT_TOKEN(2, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(3, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(4, TOKEN_STRING_LITERAL, "");
    ASSERT_TOKEN(5, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(6, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_char_pointer() {
    Lexer *lexer = init_lexer("char *x = \"Hello, World!\";", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 7);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_STAR, "*");
    ASSERT_TOKEN(2, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(3, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(4, TOKEN_STRING_LITERAL, "Hello, World!");
    ASSERT_TOKEN(5, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(6, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_char_pointer_with_escape_chars() {
    Lexer *lexer = init_lexer("char *x = \" \\t \\t \\a\\a\\a\\a \";", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 7);

    ASSERT_TOKEN(0, TOKEN_CHAR, "char");
    ASSERT_TOKEN(1, TOKEN_STAR, "*");
    ASSERT_TOKEN(2, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(3, TOKEN_SINGLE_EQUALS, "=");
    ASSERT_TOKEN(4, TOKEN_STRING_LITERAL, " \\t \\t \\a\\a\\a\\a ");
    ASSERT_TOKEN(5, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(6, TOKEN_EOF, "");

    free(lexer);
}

void test_declare_char_pointer_invalid_escape_char() {
    Lexer *lexer = init_lexer("char *x = \" \\ \"", 0);
    tokenize(lexer);
    Token *tokens = lexer->tokens;

    TEST_ASSERT_TRUE(lexer->err == INVALID_ESCAPE_SEQUENCE);

    free(lexer);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_declare_char);
    RUN_TEST(test_declare_char_with_initialization);
    RUN_TEST(test_declare_escape_char);
    RUN_TEST(test_declare_invalid_escape_char);
    RUN_TEST(test_declare_too_many_chars_in_char_literal);
    RUN_TEST(test_declare_empty_char_pointer);
    RUN_TEST(test_declare_char_pointer);
    RUN_TEST(test_declare_char_pointer_with_escape_chars);
    RUN_TEST(test_declare_char_pointer_invalid_escape_char);

    return UNITY_END();
}