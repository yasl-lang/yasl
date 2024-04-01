#include "data-structures/YASL_Set.h"
#include "data-structures/YASL_String.h"

struct YASL_StringSet {
	struct YASL_Set impl;
};

struct YASL_StringSet *YASL_StringSet_new(void);
void YASL_StringSet_del(struct YASL_StringSet *set);

struct YASL_String *YASL_StringSet_maybe_insert(struct YASL_StringSet *const set, const char *chars, const size_t len);
