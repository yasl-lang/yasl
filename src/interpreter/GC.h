#ifndef YASL_INTERPRETER_GC_H_
#define YASL_INTERPRETER_GC_H_

#include <stddef.h>

#include "data-structures/YASL_Set.h"

struct GC {
	struct YASL_Set *allocs;
	size_t total_alloc_size;
};

void gc_init(struct GC *gc);
void gc_cleanup(struct GC *gc);

struct YASL_Object gc_alloc_list(struct GC *gc);
void gc_free(struct GC *gc, struct YASL_Object *obj);

size_t gc_total_alloc_count(struct GC *gc);

void gc_collect(struct GC *gc, struct YASL_Object *root, size_t root_size);

#endif