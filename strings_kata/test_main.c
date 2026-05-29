#include <stdio.h>
#include "test.h"

int g_failures = 0;

void test_reverse_in_place(void);
void test_is_palindrome_alnum_ci(void);
void test_is_anagram_ascii(void);
void test_first_unique_char_index(void);
void test_longest_unique_substring_len(void);
void test_str_find(void);
void test_trim_in_place(void);
void test_split_ws_in_place(void);
void test_tokenize_shell_simple(void);

int main(void) {
	test_reverse_in_place();
	test_is_palindrome_alnum_ci();
	test_is_anagram_ascii();
	test_first_unique_char_index();
	test_longest_unique_substring_len();
	test_str_find();
	test_trim_in_place();
	test_split_ws_in_place();
	test_tokenize_shell_simple();

	if (g_failures == 0) {
		printf("[OK] all tests passed\n");
		return 0;
	}
	printf("[FAIL] failures=%d\n", g_failures);
	return 1;
}
