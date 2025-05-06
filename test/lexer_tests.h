#ifndef LEXER_TESTS_H
#define LEXER_TESTS_H

#define ASSERT_TOKEN(i, expected_type, expected_lexeme) \
    TEST_ASSERT_EQUAL_INT(expected_type, tokens[i].type); \
    TEST_ASSERT_EQUAL_STRING(expected_lexeme, tokens[i].lexeme);

#endif