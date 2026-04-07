#include "glob.h"

#include <assert.h>
#include <stdio.h>

static int num_failed = 0;
static int num_ran = 0;

void check_match_internal(const char *pattern, const char *str, int line) {
	num_ran++;
	bool no_error = true;
	if (!glob(pattern, str, &no_error)) {
		fprintf(stderr, "expected %s to match %s (line %d)\n", pattern, str, line);
		num_failed++;
	}
}

void check_no_match_internal(const char *pattern, const char *str, int line) {
	num_ran++;
	bool no_error = true;
	if (glob(pattern, str, &no_error)) {
		fprintf(stderr, "expected %s not to match %s (line %d)\n", pattern, str, line);
		num_failed++;
	}
}

void check_failed_internal(const char *pattern, const char *str, int line) {
	num_ran++;
	bool no_error = true;
	bool success = glob(pattern, str, &no_error);
	if (success || no_error) {
		fprintf(stderr, "expected %s to fail trying to match %s (line %d)\n", pattern, str, line);
		num_failed++;
	}
}

#define check_match(pattern, str) check_match_internal(pattern, str, __LINE__)
#define check_no_match(pattern, str) check_no_match_internal(pattern, str, __LINE__)
#define check_failed(pattern, str) check_failed_internal(pattern, str, __LINE__)

int globtest(void) {
	check_match("abc", "abc");
	check_match("?bc", "abc");
	check_match("a?c", "abc");
	check_match("ab?", "abc");
	check_match("??c", "abc");
	check_match("a??", "abc");
	check_match("???", "abc");

	check_no_match("abcd", "abc");
	check_no_match("abc", "abcd");
	check_no_match("a?d", "abcd");

	// Invalid brackets
	check_failed("[ssadsad", "s");
	check_failed("[", "[");
	check_failed("[!", "!");
	check_failed("[^", "^");

	check_match("a[]]c", "a]c");
	check_match("a]c", "a]c");
	check_match("a[[]c", "a[c");
	check_match("a[][]c", "a]c");
	check_match("a[][]c", "a[c");
	check_match("a[]?]c", "a?c");
	check_match("a[abc]c", "abc");
	check_match("a[bc]c", "abc");
	check_match("a[a-zA-Z]c", "aBc");
	check_no_match("a[bc]d", "abc");
	check_no_match("a[adc]c", "abc");
	check_match("[A-Z]at", "Cat");
	check_match("[-Z]at", "-at");

	check_no_match("a[^]]c", "a]c");
	check_no_match("a[^abc]c", "abc");
	check_match("a[^adc]c", "abc");

	check_match("*", "abc");
	check_match("*d", "abcd");
	check_match("x*d", "xabcd");
	check_match("x*", "xabcd");
	check_no_match("x*d", "axabcd");
	check_no_match("a*d", "axabcde");

	check_match("da*da*da*", "daaadabadmanda");
	check_match("da*", "da");
	check_match("da*******", "da");
	check_no_match("da*******?", "da");
	check_match("*****ba*****ab", "baaabab");
	check_match("baaa?ab", "baaabab");
	check_match("ba*a?", "baaabab");

	check_match("*?", "XX");

	check_no_match("_[[-]]_", "_]_");
	check_no_match("_[[-]A-Z]_", "_]_");

	// Escape sequences inside brackets
	check_match("_[*]_", "_*_");
	check_match("_[-]_", "_-_");
	check_match("_[[]_", "_[_");
	check_match("_[]]_", "_]_");
	check_match("_]_", "_]_");
	check_match("_[?]_", "_?_");
	check_match("_[#]_", "_#_");

	check_no_match("_[*]_", "_**_");
	check_no_match("_[[]_", "_]_");
	check_no_match("_[]]_", "_[_");
	check_no_match("_[?]_", "___");

	check_match("####-##-##", "2000-01-02");
	check_no_match("#", "#");

	check_match("\\*", "*");
	check_match("\\\\", "\\");
	check_match("\\[", "[");
	check_match("\\?", "?");
	check_match("\\#", "#");

	// printf("failed %d/%d tests.\n", num_failed, num_ran);
	return num_failed;
}

