#include "test.h"
#include "strings_kata.h"

void test_split_ws_in_place(void) {
	{
		char buf[] = "ls -l /tmp";
		char *argv[8] = {0};
		int argc = split_ws_in_place(buf, argv, 8);
		EXPECT_EQ_INT(argc, 3);
		EXPECT_STREQ(argv[0], "ls");
		EXPECT_STREQ(argv[1], "-l");
		EXPECT_STREQ(argv[2], "/tmp");
	}
	{
		char buf[] = "   echo\t\thello   world  ";
		char *argv[8] = {0};
		int argc = split_ws_in_place(buf, argv, 8);
		EXPECT_EQ_INT(argc, 3);
		EXPECT_STREQ(argv[0], "echo");
		EXPECT_STREQ(argv[1], "hello");
		EXPECT_STREQ(argv[2], "world");
	}
	{
		char buf[] = "";
		char *argv[4] = {0};
		int argc = split_ws_in_place(buf, argv, 4);
		EXPECT_EQ_INT(argc, 0);
	}
}
