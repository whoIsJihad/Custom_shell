#include "test.h"
#include "strings_kata.h"

void test_is_palindrome_alnum_ci(void) {
	EXPECT_EQ_INT(is_palindrome_alnum_ci("A man, a plan, a canal: Panama"), 1);
	EXPECT_EQ_INT(is_palindrome_alnum_ci("race a car"), 0);
	EXPECT_EQ_INT(is_palindrome_alnum_ci(""), 1);
	EXPECT_EQ_INT(is_palindrome_alnum_ci("0P"), 0);
}
