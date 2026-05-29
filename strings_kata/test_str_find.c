#include "test.h"
#include "strings_kata.h"

void test_str_find(void) {
	EXPECT_EQ_INT(str_find("hello", "ll"), 2);
	EXPECT_EQ_INT(str_find("aaaaa", "bba"), -1);
	EXPECT_EQ_INT(str_find("", ""), 0);
	EXPECT_EQ_INT(str_find("", "a"), -1);
	EXPECT_EQ_INT(str_find("abc", "c"), 2);
}
