#include "YASL_Object.h"

#include <stdio.h>
#include <string.h>

#include "data-structures/YASL_Table.h"
#include "interpreter/userdata.h"

char *float64_to_str(yasl_float d);

// Keep up to date with the YASL_Types
const char *YASL_TYPE_NAMES[] = {
	"undef",    // Y_UNDEF,
	"float",    // Y_FLOAT,
	"int",      // Y_INT,
	"bool",     // Y_BOOL,
	"str",      // Y_STR,
	"str",      // Y_STR_W,
	"list",     // Y_LIST,
	"list",     // Y_LIST_W,
	"table",    // Y_TABLE,
	"table",    // Y_TABLE_W,
	"fn",       // Y_FN,
	"mn",       // Y_BFN,
	"userptr",  // Y_USERPTR,
	"userdata", // Y_USERDATA,
	"userdata", // Y_USERDATA_W
};

struct CFunction_s *new_cfn(int (*value)(struct YASL_State *), int num_args) {
	struct CFunction_s *fn = (struct CFunction_s *) malloc(sizeof(struct CFunction_s));
	fn->value = value;
	fn->num_args = num_args;
	fn->rc = rc_new();
	return fn;
}

void cfn_del_data(struct CFunction_s *cfn) {
}

void cfn_del_rc(struct CFunction_s *cfn) {
	rc_del(cfn->rc);
	free(cfn);
}

struct YASL_Object *YASL_Table() {
	struct YASL_Object *table = (struct YASL_Object *) malloc(sizeof(struct YASL_Object));
	table->type = Y_TABLE;
	table->value.uval = rcht_new();
	return table;
}

struct YASL_Object *YASL_UserData(void *userdata, int tag, struct YASL_Table *mt, void (*destructor)(void *)) {
	struct YASL_Object *obj = (struct YASL_Object *)malloc(sizeof(struct YASL_Object));
	obj->type = Y_USERDATA;
	obj->value.uval = ud_new(userdata, tag, mt, destructor);
	return obj;
}

struct YASL_Object *YASL_Function(int64_t index) {
	struct YASL_Object *fn = (struct YASL_Object *) malloc(sizeof(struct YASL_Object));
	fn->type = Y_FN;
	fn->value.ival = index;
	return fn;
}

struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *), int num_args) {
	struct YASL_Object *fn = (struct YASL_Object *) malloc(sizeof(struct YASL_Object));
	fn->type = Y_CFN;
	fn->value.pval = malloc(sizeof(struct CFunction_s));
	fn->value.cval->value = value;
	fn->value.cval->num_args = num_args;
	fn->value.cval->rc = rc_new();
	return fn;
}

int yasl_object_cmp(struct YASL_Object a, struct YASL_Object b) {
	if (YASL_ISSTR(a) && YASL_ISSTR(b)) {
		return YASL_String_cmp(YASL_GETSTR(a), YASL_GETSTR(b));
	} else if (YASL_ISNUM(a) && YASL_ISNUM(b)) {
		yasl_float aVal, bVal;
		if(YASL_ISINT(a)) {
			aVal = (yasl_float)YASL_GETINT(a);
		} else {
			aVal = YASL_GETFLOAT(a);
		}
		if(YASL_ISINT(b)) {
			bVal = (yasl_float)YASL_GETINT(b);
		} else {
			bVal = YASL_GETFLOAT(b);
		}

		if (aVal < bVal) return -1;
		if (aVal > bVal) return 1;
		return 0;
	} else {
		printf("Cannot apply object compare to types %s and %s.\n", YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);
		exit(-1);
	}
}

int isfalsey(struct YASL_Object v) {
	/*
	 * Falsey values are:
	 * 	undef
	 * 	false
	 * 	''
	 * 	NaN
	 */
	return (
		YASL_ISUNDEF(v) ||
		(YASL_ISBOOL(v) && YASL_GETBOOL(v) == 0) ||
		(YASL_ISSTR(v) && YASL_String_len(YASL_GETSTR(v)) == 0) ||
		(YASL_ISFLOAT(v) && YASL_GETFLOAT(v) != YASL_GETFLOAT(v))
	);
}

struct YASL_Object isequal(struct YASL_Object a, struct YASL_Object b) {
	const struct YASL_Object false_c = FALSE_C;
	const struct YASL_Object true_c = TRUE_C;
	const struct YASL_Object undef_c = UNDEF_C;
	if (YASL_ISUNDEF(a) && YASL_ISUNDEF(b)) {
		return true_c;
	}
	switch (a.type) {
	case Y_BOOL:
		if (YASL_ISBOOL(b)) {
			if (YASL_GETBOOL(a) == YASL_GETBOOL(b)) {
				return true_c;
			} else {
				return false_c;
			}
		} else {
			return false_c;
		}
	case Y_TABLE:
	case Y_TABLE_W:
		if (YASL_ISTABLE(b)) {
			puts("Warning: comparison of hashes currently is not implemented.");
			return undef_c;
		}
		return false_c;
	case Y_LIST:
	case Y_LIST_W:
		if (YASL_ISLIST(b)) {
			puts("Warning: comparison of lists currently is not implemented.");
			return undef_c;
		}
		return false_c;
	case Y_STR:
	case Y_STR_W:
		if (YASL_ISSTR(b)) {
			if (YASL_GETSTR(a) == YASL_GETSTR(b)) {
				return true_c;
			}
			if (YASL_String_len(YASL_GETSTR(a)) != YASL_String_len(YASL_GETSTR(b))) {
				return false_c;
			} else {
				return memcmp(YASL_GETSTR(a)->str + YASL_GETSTR(a)->start,
					      YASL_GETSTR(b)->str + YASL_GETSTR(b)->start,
					      YASL_String_len(YASL_GETSTR(a))) ? false_c : true_c;
			}
		}
		return false_c;
	default:
		if (YASL_ISBOOL(b) || YASL_ISTABLE(b)) {
			return false_c;
		}
		int c;
		if (YASL_ISINT(a) && YASL_ISINT(b)) {
			c = YASL_GETINT(a) == YASL_GETINT(b);
		} else if (YASL_ISFLOAT(a) && YASL_ISINT(b)) {
			c = YASL_GETFLOAT(a) == (yasl_float) YASL_GETINT(b);
		} else if (YASL_ISINT(a) && YASL_ISFLOAT(b)) {
			c = (yasl_float) YASL_GETINT(a) == YASL_GETFLOAT(b);
		} else if (YASL_ISFLOAT(a) && YASL_ISFLOAT(b)) {
			c = YASL_GETFLOAT(a) == YASL_GETFLOAT(b);
		} else {
			// printf("== and != not supported for operands of types %x and %x.\n", a.type, b.type);
			return undef_c;
		}
		return YASL_BOOL(c);
	}
}

int print(struct YASL_Object v) {
	int64_t i;
	switch (v.type) {
	case Y_INT:
		printf("%" PRId64 "", YASL_GETINT(v));
		break;
	case Y_FLOAT: {
		char *tmp = float64_to_str(YASL_GETFLOAT(v));
		printf("%s", tmp);
		free(tmp);
		break;
	}
	case Y_BOOL:
		if (YASL_GETBOOL(v) == 0) printf("false");
		else printf("true");
		break;
	case Y_UNDEF:printf("undef");
		break;
	case Y_STR:
		for (i = 0; i < (yasl_int) YASL_String_len(YASL_GETSTR(v)); i++) {
			printf("%c", YASL_GETSTR(v)->str[i + YASL_GETSTR(v)->start]);
		}
		break;
	case Y_TABLE:
		printf("<table>");
		break;
	case Y_LIST:
		printf("<list>");
		break;
	case Y_FN:printf("<fn>");
		break;
	case Y_CFN:printf("<fn>");
		break;
	case Y_USERPTR:printf("0x%p", YASL_GETUSERPTR(v));
		break;
	default:
		printf("Error, unknown type: %x", v.type);
		return -1;
	}
	return 0;
}
