#include "test.h"
#include "strings_kata.h"

void test_first_unique_char_index(void) {
	EXPECT_EQ_INT(first_unique_char_index("leetcode"), 0);
	EXPECT_EQ_INT(first_unique_char_index("loveleetcode"), 2);
	EXPECT_EQ_INT(first_unique_char_index("aabb"), -1);
	EXPECT_EQ_INT(first_unique_char_index(""), -1);
}
