#include <stdlib.h>

#include "unity.h"
#include "lexer.h"
#include "lexer_tests.h"

void setUp() {}
void tearDown() {}

void test_unterminated_string_literal() {
    Lexer *lexer = init_lexer("int x;", 0);
    tokenize(lexer);

    TEST_ASSERT_TRUE(lexer->err == NO_LEXER_ERROR);
    TEST_ASSERT_TRUE(lexer->token_count == 4);

    ASSERT_TOKEN(0, TOKEN_INT, "int");
    ASSERT_TOKEN(1, TOKEN_IDENTIFIER, "x");
    ASSERT_TOKEN(2, TOKEN_SEMICOLON, ";");
    ASSERT_TOKEN(3, TOKEN_EOF, "");

    free(lexer);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_unterminated_string_literal);

    return UNITY_END();
}