#include <interpreter/YASL_Object.h>
#include "YASL_StringSet.h"

struct YASL_StringSet *YASL_StringSet_new(void) {
	return (struct YASL_StringSet *)YASL_Set_new();
}

void YASL_StringSet_del(struct YASL_StringSet *set) {
	YASL_Set_del(NULL, set);
}

struct YASL_Object *YASL_Set_search_internal(const struct YASL_Set *const set, const struct YASL_Object key);

struct YASL_String *YASL_StringSet_maybe_insert(struct YASL_StringSet *const set, const char *chars, const size_t size) {
	struct YASL_String *string = YASL_String_new_copy_unbound(chars, size);
	const struct YASL_Object *result = YASL_Set_search_internal(&set->impl, YASL_STR(string));
	if (!result) {
		YASL_Set_insert(&set->impl, YASL_STR(string));
		return string;
	}

	str_del(string);
	return result->value.sval;
}