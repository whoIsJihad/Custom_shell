#ifndef STRINGS_KATA_H
#define STRINGS_KATA_H

#include <stddef.h>

void reverse_in_place(char *s);
int is_palindrome_alnum_ci(const char *s);
int is_anagram_ascii(const char *a, const char *b);
int first_unique_char_index(const char *s);
size_t longest_unique_substring_len(const char *s);
int str_find(const char *haystack, const char *needle);
size_t trim_in_place(char *s);
int split_ws_in_place(char *buf, char **argv, size_t argv_cap);
int tokenize_shell_simple(char *buf, char **argv, size_t argv_cap);

#endif
