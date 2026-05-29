# Strings Kata (C) — simple folder

Everything is in one directory (no `src/` vs `tests/`).

## Run

```sh
bash ./run.sh
```

If `./run.sh` works on your machine, feel free to use it. (Some mounts show `text file busy`.)

Or manually:

```sh
gcc -std=c11 -Wall -Wextra -O0 -g ./*.c -o kata_tests
./kata_tests
```

## Problems (TODO in the `.c` files)

1) `reverse_in_place(char *s)`
2) `is_palindrome_alnum_ci(const char *s) -> int`
3) `is_anagram_ascii(const char *a, const char *b) -> int`
4) `first_unique_char_index(const char *s) -> int`
5) `longest_unique_substring_len(const char *s) -> size_t`
6) `str_find(const char *haystack, const char *needle) -> int`
7) `trim_in_place(char *s) -> size_t`
8) `split_ws_in_place(char *buf, char **argv, size_t cap) -> int`
9) `tokenize_shell_simple(char *buf, char **argv, size_t cap) -> int`
