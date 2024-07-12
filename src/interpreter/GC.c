
#include "GC.h"
#include "debug.h"
#include "YASL_Object.h"
#include "userdata.h"

#include "data-structures/YASL_List.h"
#include "data-structures/YASL_Table.h"
#include "closure.h"
#include "upvalue.h"

void gc_init(struct GC *gc) {
	gc->allocs = YASL_Set_new();
	gc->total_alloc_size = 0;
}

void gc_cleanup(struct GC *gc) {
	YASL_UNUSED(gc);
	YASL_ASSERT(gc_total_alloc_size(gc) == 0, "expected to have no memory allocated when we destroy GC.");
}

size_t gc_total_alloc_count(struct GC *gc) {
	return YASL_Set_length(gc->allocs);
}

size_t gc_total_alloc_size(struct GC *gc) {
	return gc->total_alloc_size;
}

struct YASL_Object gc_alloc_list(struct GC *gc) {
	struct YASL_Object obj = YASL_LIST(rcls_new(NULL));
	YASL_Set_insert_any(gc->allocs, obj);

	// gc->total_alloc_size += LIST_BASESIZE * sizeof(struct YASL_Object);
	return obj;
}

void gc_free(struct GC *gc, struct YASL_Object *obj) {
	YASL_UNUSED(gc);
	YASL_UNUSED(obj);
	// free(obj->value.pval);
}

/*
 * "white" is the condemned set, "black" is the live set.
 */
#define WHITE true
#define BLACK false

static void mark(struct YASL_Object *obj, bool color) {
	switch (obj->type) {
	case Y_CLOSURE: {
		struct Closure *closure = obj->value.lval;
		if (obj->value.lval->rc.is_condemned != color) {
			obj->value.lval->rc.is_condemned = color;
			for (size_t i = 0; i < closure->num_upvalues; i++) {
				mark(closure->upvalues[i]->location, color);
			}
		}
		break;
	}
	case Y_LIST: {
		struct YASL_List *ls = YASL_GETLIST(*obj);
		/* We have a check to avoid cycles while iterating over the list. */
		if (obj->value.uval->rc.is_condemned != color) {
			obj->value.uval->rc.is_condemned = color;
			for (size_t i = 0; i < ls->count; i++) {
				mark(ls->items + i, color);
			}
		}
		break;
	}
	case Y_TABLE: {
		struct YASL_Table *ht = YASL_GETTABLE(*obj);
		/* we check here in order to avoid cycles while recursing. */
		if (obj->value.uval->rc.is_condemned != color) {
			obj->value.uval->rc.is_condemned = color;
			FOR_TABLE(i, item, ht) {
				mark(&item->key, color);
				mark(&item->value, color);
			}
		}
		break;
	}
	case Y_STR:
		obj->value.sval->rc.is_condemned = color;
		break;
	default:
		break;
	}
}

static void mark_white(struct YASL_Object *obj) {
	mark(obj, WHITE);
}

static void mark_black(struct YASL_Object *obj) {
	mark(obj, BLACK);
}

static bool is_condemned(struct YASL_Object *obj) {
	switch (obj->type) {
	case Y_LIST:
		return obj->value.uval->rc.is_condemned;
	default:
		return false;
	}
}

void gc_collect(struct GC *gc, struct YASL_Object *root, size_t root_size) {
	FOR_SET(i, item, gc->allocs) {
		mark_white(item);
	}

	for (size_t i = 0; i < root_size; i++) {
		mark_black(root + i);
	}

	struct YASL_Set *condemned = YASL_Set_new();

	FOR_SET(i, obj, gc->allocs) {
		if (is_condemned(obj)) {
			YASL_Set_insert_any(condemned, *obj);
		}
	}

	gc->allocs = YASL_Set_difference_any(gc->allocs, condemned);

	FOR_SET(i, condemned_obj, condemned) {
		gc_free(gc, condemned_obj);
	}
}

