#include <interpreter/YASL_Object.h>
#include "hash_function.h"

#include "prime.h"

size_t hash_function(const struct YASL_Object s, const size_t a, const size_t m) {
	size_t hash = 0;
	if (obj_isstr(&s)) {
		const int64_t len_s = YASL_String_len(s.value.sval);
		const char *str = YASL_String_chars(s.value.sval);
		for (int64_t i = 0; i < len_s; i++) {
			hash = (hash * a) ^ str[i];
			hash %= m;
		}
		return (size_t) hash;
	} else {
		int64_t ll = s.value.ival & 0xFFFF;
		int64_t lu = (s.value.ival & 0xFFFF0000) >> 16;
		int64_t ul = (s.value.ival & 0xFFFF00000000) >> 32;
		int64_t uu = (s.value.ival & 0xFFFF000000000000) >> 48;
		return (size_t) (((size_t) a * ll * ll * ll * ll ^ a * a * lu * lu * lu ^ a * a * a * ul * ul ^
				  a * a * a * a * uu) % m);
	}
}

size_t get_hash(const struct YASL_Object s, const size_t num_buckets, const size_t attempt) {
	const size_t hash_a = hash_function(s, PRIME_A, num_buckets);
	if (attempt == 0) {
		return ((size_t)hash_a) % num_buckets;
	}
	const size_t hash_b = hash_function(s, PRIME_B, num_buckets);
	return ((size_t) (hash_a + (attempt * (hash_b + (hash_b == 0))))) % num_buckets;
}
