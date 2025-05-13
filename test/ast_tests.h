#ifndef AST_TESTS_H
#define AST_TESTS_H

#define ASSERT_AST_FUNCTION(idx, id, ret_type) TEST_ASSERT_TRUE(parser->tree[idx]->type == AST_FUNCTION); \
    TEST_ASSERT_EQUAL_STRING(parser->tree[idx]->as.func->identifier, id); \
    TEST_ASSERT_TRUE(parser->tree[idx]->as.func->returnType == ret_type);

#define ASSERT_RETURN(stmt, ret_type, ret_value) \
    do { \
        TEST_ASSERT_NOT_NULL((stmt)); \
        TEST_ASSERT_EQUAL_INT(AST_RETURN, (stmt)->type); \
        TEST_ASSERT_NOT_NULL((stmt)->as.ret->value); \
        TEST_ASSERT_EQUAL_INT((ret_type), (stmt)->as.ret->value->type); \
        if ((ret_type) == AST_LITERAL_INT) { \
            TEST_ASSERT_EQUAL_INT((ret_value), (stmt)->as.ret->value->as.lit_int->value); \
        } \
        if ((ret_type) == AST_LITERAL_CHAR) { \
            TEST_ASSERT_EQUAL_CHAR((ret_value), (stmt)->as.ret->value->as.lit_char->value); \
        } \
    } while (0)


#endif