#include "test.h"
#include "strings_kata.h"

void test_tokenize_shell_simple(void) {
	{
		char buf[] = "echo \"hello world\" 'x y'";
		char *argv[8] = {0};
		int argc = tokenize_shell_simple(buf, argv, 8);
		EXPECT_EQ_INT(argc, 3);
		EXPECT_STREQ(argv[0], "echo");
		EXPECT_STREQ(argv[1], "hello world");
		EXPECT_STREQ(argv[2], "x y");
	}
	{
		char buf[] = "echo a\\ b";
		char *argv[8] = {0};
		int argc = tokenize_shell_simple(buf, argv, 8);
		EXPECT_EQ_INT(argc, 2);
		EXPECT_STREQ(argv[0], "echo");
		EXPECT_STREQ(argv[1], "a b");
	}
	{
		char buf[] = "echo \"unterminated";
		char *argv[8] = {0};
		int argc = tokenize_shell_simple(buf, argv, 8);
		EXPECT_EQ_INT(argc, -1);
	}
}
