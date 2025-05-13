#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "lexer_tests.h"

void setUp() {}
void tearDown() {}

void test_unterminated_string_literal() {
    Lexer *lexer = init_lexer("char *x = \"", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == UNTERMINATED_STRING_LITERAL);

    free(lexer);
}

void test_invalid_char() {
    Lexer *lexer = init_lexer("int x = @asdtest;", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == INVALID_SYMBOL);

    free(lexer);
}

void empty_char_literal() {
    Lexer *lexer = init_lexer("char x = '';", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == EMPTY_CHAR_LITERAL);

    free(lexer);
}

void test_declare_too_many_chars_in_char_literal() {
    Lexer *lexer = init_lexer("char x = 'aa';", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == TOO_MANY_CHARS_IN_CHAR_LITERAL);

    free(lexer);
}

void test_invalid_escape_sequence() {
    Lexer *lexer = init_lexer("char x = '\\d';", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == INVALID_ESCAPE_SEQUENCE);

    free(lexer);
}

void test_invalid_numeric_token() {
    Lexer *lexer = init_lexer("char x = 2.2.2;", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == INVALID_NUMERIC_TOKEN);

    free(lexer);
}

void test_invalid_symbol_slash() {
    Lexer *lexer = init_lexer("\\", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == INVALID_SYMBOL);

    free(lexer);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_unterminated_string_literal);
    RUN_TEST(test_invalid_char);
    RUN_TEST(test_invalid_escape_sequence);
    RUN_TEST(test_declare_too_many_chars_in_char_literal);
    RUN_TEST(empty_char_literal);
    RUN_TEST(test_invalid_numeric_token);
    RUN_TEST(test_invalid_symbol_slash);

    return UNITY_END();
}