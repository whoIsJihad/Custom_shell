#include "test.h"
#include "strings_kata.h"

void test_reverse_in_place(void) {
	{
		char s[] = "abc";
		reverse_in_place(s);
		EXPECT_STREQ(s, "cba");
	}
	{
		char s[] = "";
		reverse_in_place(s);
		EXPECT_STREQ(s, "");
	}
	{
		char s[] = "a";
		reverse_in_place(s);
		EXPECT_STREQ(s, "a");
	}
	{
		char s[] = "ab";
		reverse_in_place(s);
		EXPECT_STREQ(s, "ba");
	}
	{
		char s[] = "racecar";
		reverse_in_place(s);
		EXPECT_STREQ(s, "racecar");
	}
}
