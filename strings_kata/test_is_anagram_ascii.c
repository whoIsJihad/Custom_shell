#include "test.h"
#include "strings_kata.h"

void test_is_anagram_ascii(void) {
	EXPECT_EQ_INT(is_anagram_ascii("listen", "silent"), 1);
	EXPECT_EQ_INT(is_anagram_ascii("aabb", "bbaa"), 1);
	EXPECT_EQ_INT(is_anagram_ascii("hello", "bello"), 0);
	EXPECT_EQ_INT(is_anagram_ascii("ab", "abc"), 0);
}
