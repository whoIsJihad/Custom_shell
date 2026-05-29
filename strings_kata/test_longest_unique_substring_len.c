#include "test.h"
#include "strings_kata.h"

void test_longest_unique_substring_len(void) {
	EXPECT_EQ_SIZE(longest_unique_substring_len("abcabcbb"), 3);
	EXPECT_EQ_SIZE(longest_unique_substring_len("bbbbb"), 1);
	EXPECT_EQ_SIZE(longest_unique_substring_len("pwwkew"), 3);
	EXPECT_EQ_SIZE(longest_unique_substring_len(""), 0);
	EXPECT_EQ_SIZE(longest_unique_substring_len(" "), 1);
}
