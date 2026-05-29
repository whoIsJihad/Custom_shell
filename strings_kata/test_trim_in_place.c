#include "test.h"
#include "strings_kata.h"

void test_trim_in_place(void) {
	{
		char s[] = "  hello ";
		size_t n = trim_in_place(s);
		EXPECT_STREQ(s, "hello");
		EXPECT_EQ_SIZE(n, 5);
	}
	{
		char s[] = " \t\n";
		size_t n = trim_in_place(s);
		EXPECT_STREQ(s, "");
		EXPECT_EQ_SIZE(n, 0);
	}
	{
		char s[] = "hello";
		size_t n = trim_in_place(s);
		EXPECT_STREQ(s, "hello");
		EXPECT_EQ_SIZE(n, 5);
	}
	{
		char s[] = "   a b  ";
		size_t n = trim_in_place(s);
		EXPECT_STREQ(s, "a b");
		EXPECT_EQ_SIZE(n, 3);
	}
}
