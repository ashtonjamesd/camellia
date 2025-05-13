#ifndef AST_TESTS_H
#define AST_TESTS_H

#define ASSERT_AST_FUNCTION(idx, id, ret_type) TEST_ASSERT_TRUE(parser->tree[idx]->type == AST_FUNCTION); \
    TEST_ASSERT_EQUAL_STRING(parser->tree[idx]->as.func->identifier, id); \
    TEST_ASSERT_TRUE(parser->tree[idx]->as.func->returnType == ret_type);

#endif