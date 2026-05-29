#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern int g_failures;

#if defined(__GNUC__) || defined(__clang__)
#define TEST_UNUSED __attribute__((unused))
#else
#define TEST_UNUSED
#endif

#define EXPECT_TRUE(expr) do { \
	if (!(expr)) { \
		g_failures++; \
		printf("[FAIL] %s:%d: EXPECT_TRUE(%s)\n", __FILE__, __LINE__, #expr); \
	} \
} while (0)

#define EXPECT_EQ_INT(actual, expected) do { \
	int a__ = (actual); \
	int e__ = (expected); \
	if (a__ != e__) { \
		g_failures++; \
		printf("[FAIL] %s:%d: EXPECT_EQ_INT got=%d expected=%d\n", __FILE__, __LINE__, a__, e__); \
	} \
} while (0)

#define EXPECT_EQ_SIZE(actual, expected) do { \
	size_t a__ = (actual); \
	size_t e__ = (expected); \
	if (a__ != e__) { \
		g_failures++; \
		printf("[FAIL] %s:%d: EXPECT_EQ_SIZE got=%zu expected=%zu\n", __FILE__, __LINE__, a__, e__); \
	} \
} while (0)

static TEST_UNUSED int streq_safe(const char *a, const char *b) {
	if (a == NULL || b == NULL) return a == b;
	return strcmp(a, b) == 0;
}

#define EXPECT_STREQ(actual, expected) do { \
	const char *a__ = (actual); \
	const char *e__ = (expected); \
	if (!streq_safe(a__, e__)) { \
		g_failures++; \
		printf("[FAIL] %s:%d: EXPECT_STREQ got=\"%s\" expected=\"%s\"\n", __FILE__, __LINE__, a__ ? a__ : "(null)", e__ ? e__ : "(null)"); \
	} \
} while (0)

#endif
