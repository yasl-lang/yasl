#include "VM.h"

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "interpreter/builtins.h"
#include "data-structures/YASL_String.h"
#include "data-structures/YASL_Table.h"
#include "interpreter/refcount.h"

#include "interpreter/table_methods.h"
#include "interpreter/list_methods.h"
#include "interpreter/str_methods.h"
#include "yasl_state.h"
#include "yasl_error.h"
#include "yasl_include.h"
#include "opcode.h"
#include "operator_names.h"
#include "YASL_Object.h"
#include "upvalue.h"

static struct YASL_Table **builtins_htable_new(struct VM *const vm) {
    struct YASL_Table **ht = (struct YASL_Table **)malloc(sizeof(struct YASL_Table*) * NUM_TYPES);
    ht[Y_UNDEF] = undef_builtins(vm);
    ht[Y_FLOAT] = float_builtins(vm);
    ht[Y_INT] = int_builtins(vm);
    ht[Y_BOOL] = bool_builtins(vm);
    ht[Y_STR] = str_builtins(vm);
    ht[Y_LIST] = list_builtins(vm);
    ht[Y_TABLE] = table_builtins(vm);

    return ht;
}

void vm_init(struct VM *const vm,
	     unsigned char *const code,    // pointer to bytecode
             const size_t pc,              // address of instruction to be executed first -- entrypoint
             const size_t datasize) {      // total params size required to perform a program operations
	vm->code = code;
	vm->headers = (unsigned char **)calloc(sizeof(unsigned char *), datasize);
	// vm->headers[0] = code;
	vm->headers_size = datasize;
	vm->num_globals = datasize;
	vm->frame_num = -1;
	vm->loopframe_num = -1;
	vm->globals = (struct YASL_Table **)calloc(sizeof(struct YASL_Table *), datasize);
	vm->out = NEW_IO(stdout);
	vm->err = NEW_IO(stderr);
	for (size_t i = 0; i < datasize; i++) {
		vm->headers[i] = NULL;
		vm->globals[i] = NULL;
	}
	vm->headers[datasize - 1] = code;
	vm->globals[0] = YASL_Table_new();
	vm->pc = code + pc;
	vm->fp = -1;
	vm->sp = -1;
	vm->global_vars = (struct YASL_Object *)calloc(sizeof(struct YASL_Object), NUM_GLOBALS);

	vm->stack = (struct YASL_Object *)calloc(sizeof(struct YASL_Object), STACK_SIZE);

#define DEF_SPECIAL_STR(enum_val, str) vm->special_strings[enum_val] = YASL_String_new_sized(strlen(str), str)

	DEF_SPECIAL_STR(S___ADD, "__add");
	DEF_SPECIAL_STR(S___GET, "__get");
	DEF_SPECIAL_STR(S___SET, "__set");
	DEF_SPECIAL_STR(S_CLEAR, "clear");
	DEF_SPECIAL_STR(S_COPY, "copy");
	DEF_SPECIAL_STR(S_COUNT, "count");
	DEF_SPECIAL_STR(S_ENDSWITH, "endswith");
	DEF_SPECIAL_STR(S_EXTEND, "extend");
	DEF_SPECIAL_STR(S_ISAL, "isal");
	DEF_SPECIAL_STR(S_ISALNUM, "isalnum");
	DEF_SPECIAL_STR(S_ISNUM, "isnum");
	DEF_SPECIAL_STR(S_ISSPACE, "isspace");
	DEF_SPECIAL_STR(S_JOIN, "join");
	DEF_SPECIAL_STR(S_SORT, "sort");
	DEF_SPECIAL_STR(S_KEYS, "keys");
	DEF_SPECIAL_STR(S_LTRIM, "ltrim");
	DEF_SPECIAL_STR(S_POP, "pop");
	DEF_SPECIAL_STR(S_PUSH, "push");
	DEF_SPECIAL_STR(S_REMOVE, "remove");
	DEF_SPECIAL_STR(S_REP, "rep");
	DEF_SPECIAL_STR(S_REPLACE, "replace");
	DEF_SPECIAL_STR(S_REVERSE, "reverse");
	DEF_SPECIAL_STR(S_RTRIM, "rtrim");
	DEF_SPECIAL_STR(S_SEARCH, "search");
	// DEF_SPECIAL_STR(S_SLICE, "slice");
	DEF_SPECIAL_STR(S_SPLIT, "split");
	DEF_SPECIAL_STR(S_STARTSWITH, "startswith");
	DEF_SPECIAL_STR(S_TOBOOL, "tobool");
	DEF_SPECIAL_STR(S_TOFLOAT, "tofloat");
	DEF_SPECIAL_STR(S_TOINT, "toint");
	DEF_SPECIAL_STR(S_TOLOWER, "tolower");
	DEF_SPECIAL_STR(S_TOSTR, "tostr");
	DEF_SPECIAL_STR(S_TOUPPER, "toupper");
	DEF_SPECIAL_STR(S_TRIM, "trim");
	DEF_SPECIAL_STR(S_VALUES, "values");

#undef DEF_SPECIAL_STR

	vm->builtins_htable = builtins_htable_new(vm);
	vm->pending = NULL;
}

void vm_cleanup(struct VM *const vm) {
	// YASL_ASSERT(vm->sp == -1, "VM stack pointer should have been -1.");
	// YASL_ASSERT(vm->fp == -1, "VM frame pointer should have been -1.");
	// YASL_ASSERT(vm->frame_num == (size_t)-1, "frame num should be 0.");
	for (size_t i = 0; i < STACK_SIZE; i++) dec_ref(&vm->stack[i]);
	for (size_t i = 0; i < NUM_GLOBALS; i++) {
		dec_ref(&vm->global_vars[i]);
	}

	free(vm->global_vars);
	free(vm->stack);

	for (size_t i = 0; i < vm->headers_size; i++) {
		free(vm->headers[i]);
	}
	free(vm->headers);

	for (size_t i = 0; i < vm->num_globals; i++) {
		YASL_Table_del(vm->globals[i]);
	}
	free(vm->globals);

	YASL_Table_del(vm->builtins_htable[Y_UNDEF]);
	YASL_Table_del(vm->builtins_htable[Y_FLOAT]);
	YASL_Table_del(vm->builtins_htable[Y_INT]);
	YASL_Table_del(vm->builtins_htable[Y_BOOL]);
	YASL_Table_del(vm->builtins_htable[Y_STR]);
	YASL_Table_del(vm->builtins_htable[Y_LIST]);
	YASL_Table_del(vm->builtins_htable[Y_TABLE]);
	free(vm->builtins_htable);
	// TODO: free upvalues
}

void vm_push(struct VM *const vm, const struct YASL_Object val) {
    vm->sp++;
    dec_ref(vm->stack + vm->sp);
    vm->stack[vm->sp] = val;
    inc_ref(vm->stack + vm->sp);
}

void vm_pushend(struct VM *const vm) {
	vm_push(vm, YASL_END());
}

void vm_pushundef(struct VM *const vm) {
	vm_push(vm, YASL_UNDEF());
}

void vm_pushfloat(struct VM *const vm, yasl_float f) {
	vm_push(vm, YASL_FLOAT(f));
}

void vm_pushint(struct VM *const vm, yasl_int i) {
	vm_push(vm, YASL_INT(i));
}

void vm_pushbool(struct VM *const vm, bool b) {
	vm_push(vm, YASL_BOOL(b));
}

void vm_pushclosure(struct VM *const vm, const unsigned char *const f) {
	struct Closure *c = (struct Closure *)malloc(sizeof(struct Closure));
	c->f = f;
	vm_push(vm, ((struct YASL_Object){.type = Y_CLOSURE, .value = {.lval = c}}));
}

struct YASL_Object vm_pop(struct VM *const vm) {
	return vm->stack[vm->sp--];
}
bool vm_popbool(struct VM *const vm) {
	return (bool)YASL_GETBOOL(vm_pop(vm));
}

yasl_float vm_popfloat(struct VM *const vm) {
	return YASL_GETFLOAT(vm_pop(vm));
}

yasl_int vm_popint(struct VM *const vm) {
	return YASL_GETINT(vm_pop(vm));
}

struct YASL_String *vm_popstr(struct VM *const vm) {
	return YASL_GETSTR(vm_pop(vm));
}

struct YASL_List *vm_poplist(struct VM *const vm) {
	return YASL_GETLIST(vm_pop(vm));
}

static yasl_int vm_read_int(struct VM *const vm) {
    yasl_int val;
    memcpy(&val, vm->pc, sizeof(yasl_int));
    vm->pc += sizeof(yasl_int);
    return val;
}

#define READ_INT(size) \
	static yasl_int vm_read_int##size(struct VM *const vm) {\
		int##size##_t val;\
		memcpy(&val, vm->pc, sizeof(int##size##_t));\
		vm->pc += sizeof(int##size##_t);\
		return (yasl_int) val;\
	}

READ_INT(16)
READ_INT(32)
READ_INT(64)

static yasl_float vm_read_float(struct VM *const vm) {
    yasl_float val;
    memcpy(&val, vm->pc, sizeof(yasl_float));
    vm->pc += sizeof(yasl_float);
    return val;
}

static int vm_GET(struct VM *const vm);
static int vm_INIT_CALL(struct VM *const vm);
static int vm_CALL(struct VM *const vm);

#define INT_BINOP(name, op) yasl_int name(yasl_int left, yasl_int right) { return left op right; }

INT_BINOP(bor, |)
INT_BINOP(bxor, ^)
INT_BINOP(band, &)
INT_BINOP(bandnot, &~)
INT_BINOP(shift_left, <<)
INT_BINOP(shift_right, >>)
INT_BINOP(modulo, %)
INT_BINOP(idiv, /)

static int vm_int_binop(struct VM *const vm, yasl_int (*op)(yasl_int, yasl_int), const char *opstr, const char *overload_name) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_pop(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_INT(op(YASL_GETINT(left), YASL_GETINT(right))));
		return YASL_SUCCESS;
	} else {
		inc_ref(&left);
		inc_ref(&right);
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen(overload_name), overload_name));
		vm_push(vm, left);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm, "%s not supported for operands of types %s and %s.\n",
					      opstr,
					      YASL_TYPE_NAMES[left.type],
					      YASL_TYPE_NAMES[right.type]);
			dec_ref(&left);
			dec_ref(&right);
			return YASL_TYPE_ERROR;
		} else {
			int res;
			vm_INIT_CALL(vm);
			vm_push(vm, left);
			vm_push(vm, right);
			res = vm_CALL(vm);
			dec_ref(&left);
			dec_ref(&right);
			return res;
		}
	}
	return YASL_SUCCESS;
}

#define FLOAT_BINOP(name, op) yasl_float name(yasl_float left, yasl_float right) { return left op right; }
#define NUM_BINOP(name, op) INT_BINOP(int_ ## name, op) FLOAT_BINOP(float_ ## name, op)

NUM_BINOP(add, +)
NUM_BINOP(sub, -)
NUM_BINOP(mul, *)

static yasl_int int_pow(yasl_int left, yasl_int right) {
    return (yasl_int)pow((double)left, (double)right);
}

static int vm_num_binop(
        struct VM *const vm, yasl_int (*int_op)(yasl_int, yasl_int),
        yasl_float (*float_op)(yasl_float, yasl_float),
        const char *const opstr,
        const char *overload_name) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_pop(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_INT(int_op(YASL_GETINT(left), YASL_GETINT(right))));
	} else if (YASL_ISFLOAT(left) && YASL_ISFLOAT(right)) {
		vm_pushfloat(vm, float_op(YASL_GETFLOAT(left), YASL_GETFLOAT(right)));
	} else if (YASL_ISFLOAT(left) && YASL_ISINT(right)) {
		vm_pushfloat(vm, float_op(YASL_GETFLOAT(left), (yasl_float)YASL_GETINT(right)));
	} else if (YASL_ISINT(left) && YASL_ISFLOAT(right)) {
		vm_pushfloat(vm, float_op((yasl_float)YASL_GETINT(left), YASL_GETFLOAT(right)));
	} else {
		inc_ref(&left);
		inc_ref(&right);
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen(overload_name), overload_name));
		vm_push(vm, left);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm, "%s not supported for operands of types %s and %s.\n",
					      opstr,
					      YASL_TYPE_NAMES[left.type],
					      YASL_TYPE_NAMES[right.type]);
			dec_ref(&left);
			dec_ref(&right);
			return YASL_TYPE_ERROR;
		} else {
			int res;
			vm_INIT_CALL(vm);
			vm_push(vm, left);
			vm_push(vm, right);
			res = vm_CALL(vm);
			dec_ref(&left);
			dec_ref(&right);
			return res;
		}
	}
	return YASL_SUCCESS;
}

static int vm_fdiv(struct VM *const vm) {
	const char *overload_name = OP_BIN_FDIV;
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_pop(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right)) {
		vm_pushfloat(vm, (yasl_float) YASL_GETINT(left) / (yasl_float) YASL_GETINT(right));
	} else if (YASL_ISFLOAT(left) && YASL_ISFLOAT(right)) {
		vm_pushfloat(vm, YASL_GETFLOAT(left) / YASL_GETFLOAT(right));
	} else if (YASL_ISINT(left) && YASL_ISFLOAT(right)) {
		vm_pushfloat(vm, (yasl_float) YASL_GETINT(left) / YASL_GETFLOAT(right));
	} else if (YASL_ISFLOAT(left) && YASL_ISINT(right)) {
		vm_pushfloat(vm, YASL_GETFLOAT(left) / (yasl_float) YASL_GETINT(right));
	} else {
		inc_ref(&left);
		inc_ref(&right);
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen(overload_name), overload_name));
		vm_push(vm, left);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm, "/ not supported for operands of types %s and %s.\n",
					      YASL_TYPE_NAMES[left.type],
					      YASL_TYPE_NAMES[right.type]);
			dec_ref(&left);
			dec_ref(&right);
			return YASL_TYPE_ERROR;
		} else {
			int res;
			vm_INIT_CALL(vm);
			vm_push(vm, left);
			vm_push(vm, right);
			res = vm_CALL(vm);
			dec_ref(&left);
			dec_ref(&right);
			return res;
		}
	}
	return YASL_SUCCESS;
}

static int vm_pow(struct VM *const vm) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_peek(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right) && YASL_GETINT(right) < 0) {
		vm_pop(vm);
		vm_pushfloat(vm, pow((double)YASL_GETINT(left), (double)YASL_GETINT(right)));
	} else {
		vm->sp++;
		int res = vm_num_binop(vm, &int_pow, &pow, "**", OP_BIN_POWER);
		return res;
	}
	return YASL_SUCCESS;
}

#define INT_UNOP(name, op) yasl_int name(yasl_int val) { return op val; }
#define FLOAT_UNOP(name, op) yasl_float name(yasl_float val) { return op val; }
#define NUM_UNOP(name, op) INT_UNOP(int_ ## name, op) FLOAT_UNOP(float_ ## name, op)

INT_UNOP(bnot, ~)
NUM_UNOP(neg, -)
NUM_UNOP(pos, +)

static int vm_int_unop(struct VM *const vm, yasl_int (*op)(yasl_int), const char *opstr, const char *overload_name) {
	struct YASL_Object a = vm_pop(vm);
	if (YASL_ISINT(a)) {
		vm_push(vm, YASL_INT(op(YASL_GETINT(a))));
		return YASL_SUCCESS;
	} else {
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen(overload_name), overload_name));
		vm_push(vm, a);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm, "%s not supported for operand of type %s.\n",
					      opstr,
					      YASL_TYPE_NAMES[a.type]);
			return YASL_TYPE_ERROR;
		} else {
			vm_INIT_CALL(vm);
			vm_push(vm, a);
			vm_CALL(vm);
		}
	}
	return YASL_SUCCESS;
}

static int vm_num_unop(struct VM *const vm, yasl_int (*int_op)(yasl_int), yasl_float (*float_op)(yasl_float), const char *opstr, const char *overload_name) {
	struct YASL_Object expr = vm_pop(vm);
	if (YASL_ISINT(expr)) {
		vm_push(vm, YASL_INT(int_op(YASL_GETINT(expr))));
	} else if (YASL_ISFLOAT(expr)) {
		vm_pushfloat(vm, float_op(YASL_GETFLOAT(expr)));
	} else {
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen(overload_name), overload_name));
		vm_push(vm, expr);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm,  "%s not supported for operand of type %s.\n",
					      opstr,
					      YASL_TYPE_NAMES[expr.type]);
			return YASL_TYPE_ERROR;
		} else {
			vm_INIT_CALL(vm);
			vm_push(vm, expr);
			vm_CALL(vm);
		}
	}
	return YASL_SUCCESS;
}

static int vm_len_unop(struct VM *const vm) {
	struct YASL_Object v = vm_pop(vm);
	if (YASL_ISSTR(v)) {
		vm_pushint(vm, (yasl_int) YASL_String_len(YASL_GETSTR(v)));
	} else if (YASL_ISTABLE(v)) {
		vm_pushint(vm, (yasl_int)YASL_GETTABLE(v)->count);
	} else if (YASL_ISLIST(v)) {
		vm_pushint(vm, (yasl_int)YASL_GETLIST(v)->count);
	} else {
		struct YASL_Object op_name = YASL_STR(YASL_String_new_sized(strlen("__len"), "__len"));
		vm_push(vm, v);
		vm_push(vm, op_name);
		vm_GET(vm);
		if (vm_isundef(vm)) {
			vm_print_err_type(vm,  "len not supported for operand of type %s.\n",
					      YASL_TYPE_NAMES[v.type]);
			return YASL_TYPE_ERROR;
		} else {
			vm_INIT_CALL(vm);
			vm_push(vm, v);
			vm_CALL(vm);
		}
	}
	return YASL_SUCCESS;
}

static int vm_CNCT(struct VM *const vm) {
		vm_stringify_top(vm);
		struct YASL_String *b = vm_popstr(vm);
		vm_stringify_top(vm);
		struct YASL_String *a = vm_popstr(vm);

		size_t size = YASL_String_len((a)) + YASL_String_len((b));
		char *ptr = (char *)malloc(size);
		memcpy(ptr, (a)->str + (a)->start,
		       YASL_String_len((a)));
		memcpy(ptr + YASL_String_len((a)),
		       ((b))->str + (b)->start,
		       YASL_String_len((b)));
		vm_pushstr(vm, YASL_String_new_sized_heap(0, size, ptr));
		return YASL_SUCCESS;
}

int vm_stringify_top(struct VM *const vm) {
	enum YASL_Types index = vm_peek(vm, vm->sp).type;
	if (YASL_ISFN(vm_peek(vm, vm->sp)) || YASL_ISCFN(vm_peek(vm, vm->sp))) {
		size_t n = (size_t)snprintf(NULL, 0, "<fn: %d>", (int)YASL_GETINT(vm_peek(vm))) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<fn: %d>", (int)vm_pop(vm).value.ival);
		vm_pushstr(vm, YASL_String_new_sized_heap(0, strlen(buffer), buffer));
	} else if (YASL_ISUSERDATA(vm_peek(vm, vm->sp))) {
		struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = YASL_Table_search(vm_peek(vm).value.uval->mt, key);
		str_del(YASL_GETSTR(key));
		if (result.type == Y_END) {
			exit(EXIT_FAILURE);
		}
		YASL_GETCFN(result)->value((struct YASL_State *)vm);
		// vm_push(vm, result);
	} else {
		struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = YASL_Table_search(vm->builtins_htable[index], key);
		str_del(YASL_GETSTR(key));
		YASL_GETCFN(result)->value((struct YASL_State *)vm);
	}
	return YASL_SUCCESS;
}

static int vm_SLICE(struct VM *const vm) {
	if (!vm_isint(vm) || !YASL_ISINT(vm_peek(vm, vm->sp - 1))) {
		vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
				      YASL_TYPE_NAMES[vm_peek(vm, vm->sp - 1).type],
				      YASL_TYPE_NAMES[vm_peek(vm, vm->sp).type]
		);
		return YASL_TYPE_ERROR;
	}

	yasl_int end = vm_popint(vm);
	yasl_int start = vm_popint(vm);

	if (vm_islist(vm)) {
		struct YASL_List *list = vm_poplist(vm);
		yasl_int len = list->count;
		if (end < 0)
			end += len;

		if (start < 0)
			start += len;

		if (end > len)
			end = len;

		if (start < 0)
			start = 0;

		struct RC_UserData *new_ls = rcls_new();

		for (yasl_int i = start; i <end; ++i) {
			YASL_List_append((struct YASL_List *) new_ls->data, list->items[i]);
		}
		vm_push(vm, YASL_LIST(new_ls));
		return YASL_SUCCESS;
	}

	if (vm_isstr(vm)) {
		struct YASL_String *str = vm_popstr(vm);
		yasl_int len = YASL_String_len(str);
		if (end < 0)
			end += len;

		if (start < 0)
			start += len;

		if (end > len)
			end = len;

		if (start < 0)
			start = 0;

		vm_push(vm, YASL_STR(YASL_String_new_substring(start, end, str)));
		return YASL_SUCCESS;
	}

	vm_print_err_type(vm,  "slice is not defined for objects of type %s.", YASL_TYPE_NAMES[vm_pop(vm).type]);
	return YASL_TYPE_ERROR;

}

static int vm_GET(struct VM *const vm) {
	vm->sp--;
	int index = vm_peek(vm).type;
	if (vm_islist(vm)) {
		vm->sp++;
		if (!list___get((struct YASL_State *) vm)) {
			return YASL_SUCCESS;
		}
	} else if (vm_istable(vm)) {
		vm->sp++;
		if (!table___get((struct YASL_State *) vm)) {
			return YASL_SUCCESS;
		}
	} else if (vm_isstr(vm) && YASL_ISINT(vm_peek(vm, vm->sp + 1))) {
		vm->sp++;
		if (!str___get((struct YASL_State *) vm)) {
			return YASL_SUCCESS;
		}
	} else if (vm_isuserdata(vm)) {
		vm->sp++;
		struct YASL_Object key = vm_pop(vm);
		struct YASL_Object result = YASL_Table_search(vm_peek(vm).value.uval->mt, key);
		vm_pop(vm);
		if (result.type == Y_END) {
			vm_pushundef(vm);
		} else {
			vm_push(vm, result);
		}
		return YASL_SUCCESS;
	} else {
		vm->sp++;
	}

	struct YASL_Object key = vm_pop(vm);
	struct YASL_Object result = YASL_Table_search(vm->builtins_htable[index], key);
	vm_pop(vm);
	if (result.type == Y_END) {
		vm_pushundef(vm);
	} else {
		vm_push(vm, result);
	}
	return YASL_SUCCESS;
}

static int vm_SET(struct VM *const vm) {
	vm->sp -= 2;
	if (vm_islist(vm)) {
		vm->sp += 2;
		list___set((struct YASL_State *) vm);
	} else if (vm_istable(vm)) {
		vm->sp += 2;
		table___set((struct YASL_State *) vm);
	} else {
		vm->sp += 2;
		vm_print_err_type(vm,  "object of type %s is immutable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

static int vm_NEWSPECIALSTR(struct VM *const vm) {
	vm_pushstr(vm, vm->special_strings[NCODE(vm)]);
	return YASL_SUCCESS;
}

static int vm_NEWSTR(struct VM *const vm) {
	yasl_int table = vm_read_int(vm);
	yasl_int addr = vm_read_int(vm);

	yasl_int size;
	memcpy(&size, vm->headers[table] + addr, sizeof(yasl_int));

	addr += sizeof(yasl_int);
	struct YASL_String *string = YASL_String_new_sized(size, ((char *) vm->headers[table]) + addr);
	vm_pushstr(vm, string);
	return YASL_SUCCESS;
}

static int vm_ITER_1(struct VM *const vm) {
	struct LoopFrame *frame = &vm->loopframes[vm->loopframe_num];
	switch (frame->iterable.type) {
	case Y_LIST:
		if ((yasl_int)YASL_GETLIST(frame->iterable)->count <= frame->iter) {
			vm_pushbool(vm, 0);
		} else {
			vm_push(vm, YASL_GETLIST(frame->iterable)->items[frame->iter++]);
			vm_pushbool(vm, 1);
		}
		return YASL_SUCCESS;
	case Y_TABLE:
		while (YASL_GETTABLE(frame->iterable)->size > (size_t) frame->iter &&
		       (YASL_GETTABLE(frame->iterable)->items[frame->iter].key.type == Y_END ||
			YASL_GETTABLE(frame->iterable)->items[frame->iter].key.type == Y_UNDEF)
			) {
			frame->iter++;
		}
		if (YASL_GETTABLE(frame->iterable)->size <=
		    (size_t) frame->iter) {
			vm_pushbool(vm, 0);
			return YASL_SUCCESS;
		}
		vm_push(vm,
			YASL_GETTABLE(frame->iterable)->items[frame->iter++].key);
		vm_pushbool(vm, 1);
		return YASL_SUCCESS;
	case Y_STR:
		if ((yasl_int) YASL_String_len(YASL_GETSTR(frame->iterable)) <= frame->iter) {
			vm_push(vm, YASL_BOOL(0));
		} else {
			size_t i = (size_t)frame->iter;
			vm_push(vm, YASL_STR(YASL_String_new_substring(i, i + 1, YASL_GETSTR(frame->iterable))));
			frame->iter++;
			vm_pushbool(vm, 1);
		}
		return YASL_SUCCESS;
	default:
		vm_print_err_type(vm,  "object of type %s is not iterable.\n", YASL_TYPE_NAMES[frame->iterable.type]);
		return YASL_TYPE_ERROR;
	}
}

static int vm_GSTORE_8(struct VM *const vm) {
	yasl_int table = vm_read_int(vm);
	yasl_int addr = vm_read_int(vm);

	yasl_int size;
	memcpy(&size, vm->headers[table] + addr, sizeof(yasl_int));

	addr += sizeof(yasl_int);
	struct YASL_String *string = YASL_String_new_sized(size, ((char *) vm->headers[table]) + addr);

	YASL_Table_insert(vm->globals[table], YASL_STR(string), vm_pop(vm));
	return YASL_SUCCESS;
}

static int vm_GLOAD_8(struct VM *const vm) {
	yasl_int table = vm_read_int(vm);
	yasl_int addr = vm_read_int(vm);

	yasl_int size;
	memcpy(&size, vm->headers[table] + addr, sizeof(yasl_int));

	addr += sizeof(yasl_int);
	struct YASL_String *string = YASL_String_new_sized((size_t) size, ((char *) vm->headers[table]) + addr);

	vm_push(vm, YASL_Table_search(vm->globals[table], YASL_STR(string)));

	YASL_ASSERT(vm_peek(vm).type != Y_END, "global not found");

	str_del(string);
	return YASL_SUCCESS;
}

void vm_CLOSE(struct VM *const vm) {
	// TODO
}

static void vm_enterframe(struct VM *const vm) {
	int next_fp = vm->next_fp;
	vm->next_fp = vm->sp;
	vm->frames[++vm->frame_num] = ((struct CallFrame) { vm->pc, vm->fp, next_fp, vm->loopframe_num });
}

static void vm_exitframe(struct VM *const vm) {
	struct YASL_Object v = vm_pop(vm);
	vm->sp = vm->fp;
	vm_pop(vm);

	vm->pc = vm->frames[vm->frame_num].pc;
	vm->fp = vm->frames[vm->frame_num].fp;
	vm->next_fp = vm->frames[vm->frame_num].next_fp;
	while (vm->loopframe_num > vm->frames[vm->frame_num].lp) {
		dec_ref(&vm->loopframes[vm->loopframe_num--].iterable);
	}

	vm->frame_num--;

	vm_push(vm, v);
}

static int vm_INIT_CALL(struct VM *const vm) {
	if (!YASL_ISFN(vm_peek(vm)) && !YASL_ISCFN(vm_peek(vm))) {
		vm_print_err_type(vm,  "%s is not callable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
		return YASL_TYPE_ERROR;
	}

	vm_enterframe(vm);

	return YASL_SUCCESS;
}

static int vm_INIT_MC(struct VM *const vm) {
	struct YASL_Object top = vm_peek(vm);
	inc_ref(&top);
	vm_NEWSTR(vm);
	vm_GET(vm);
	vm_INIT_CALL(vm);
	vm_push(vm, top);
	dec_ref(&top);
	return YASL_SUCCESS;
}

static int vm_INIT_MC_SPECIAL(struct VM *const vm) {
	struct YASL_Object top = vm_peek(vm);
	inc_ref(&top);
	vm_NEWSPECIALSTR(vm);
	vm_GET(vm);
	vm_INIT_CALL(vm);
	vm_push(vm, top);
	dec_ref(&top);
	return YASL_SUCCESS;
}

static int vm_CALL_fn(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;

	while (vm->sp - vm->fp < vm_peek(vm, vm->fp).value.fval[0]) {
		vm_pushundef(vm);
	}

	while (vm->sp - vm->fp > vm_peek(vm, vm->fp).value.fval[0]) {
		vm_pop(vm);
	}

	// vm->sp += vm_peek(vm, vm->fp).value.fval[1];

	vm->pc =  vm_peek(vm, vm->fp).value.fval + 2;
	return YASL_SUCCESS;
}

static int vm_CALL_cfn(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;
	if (vm_peekcfn(vm, vm->fp)->num_args == -1) {
		YASL_VM_DEBUG_LOG("vm->sp - vm->fp: %d\n", vm->sp - (vm->fp));
		vm_pushint(vm, vm->sp - (vm->fp));
	} else {
		while (vm->sp - (vm->fp) < vm_peekcfn(vm, vm->fp)->num_args) {
			vm_pushundef(vm);
		}

		while (vm->sp - (vm->fp) > vm_peekcfn(vm, vm->fp)->num_args) {
			vm_pop(vm);
		}
	}
	int result;
	if ((result = vm_peekcfn(vm, vm->fp)->value((struct YASL_State *) vm))) {
		return result;
	};

	vm_exitframe(vm);
	return YASL_SUCCESS;
}

static int vm_CALL(struct VM *const vm) {
	vm->fp = vm->next_fp;
	if (YASL_ISFN(vm_peek(vm, vm->fp))) {
		return vm_CALL_fn(vm);
	} else if (YASL_ISCFN(vm_peek(vm, vm->fp))) {
		return vm_CALL_cfn(vm);
	} else {
		printf("ERROR: %s is not callable", YASL_TYPE_NAMES[vm_peek(vm).type]);
		return YASL_TYPE_ERROR;
	}
}

static int vm_RET(struct VM *const vm) {
	// TODO: handle multiple returns
	vm_exitframe(vm);
	return YASL_SUCCESS;
}

static int vm_CRET(struct VM *const vm) {
	int tmp = vm->fp;
	vm_exitframe(vm);
	vm_close_all(vm, tmp);
	return YASL_SUCCESS;
}

static void vm_PRINT(struct VM *const vm) {
	vm_stringify_top(vm);
	struct YASL_String *v = vm_popstr(vm);
	vm->out.print(&vm->out, v->start + v->str, YASL_String_len(v));
	vm->out.print(&vm->out, "\n", 1);
}

int vm_run(struct VM *const vm) {
	while (1) {
		unsigned char opcode = NCODE(vm);        // fetch
		signed char offset;
		// size_t size;
		yasl_int addr;
		struct YASL_Object a, b, v;
		yasl_int c;
		yasl_float d;
		int res;
		YASL_VM_DEBUG_LOG("----------------"
				  "opcode: %x\n"
				  "vm->sp, vm->fp, vm->next_fp: %d, %d, %d\n\n", opcode, vm->sp, vm->fp, vm->next_fp);
		switch (opcode) {
		case O_EXPORT:
			return YASL_MODULE_SUCCESS;
		case O_HALT:
			return YASL_SUCCESS;
		case O_ICONST_M1:
		case O_ICONST_0:
		case O_ICONST_1:
		case O_ICONST_2:
		case O_ICONST_3:
		case O_ICONST_4:
		case O_ICONST_5:
			vm_pushint(vm, opcode - O_ICONST_0); // make sure no changes to opcodes ruin this
			break;
		case O_DCONST_0:
		case O_DCONST_1:
		case O_DCONST_2:
			vm_pushfloat(vm, (yasl_float)(opcode - O_DCONST_0)); // make sure no changes to opcodes ruin this
			break;
		case O_DCONST_N:
			vm_pushfloat(vm, NAN);
			break;
		case O_DCONST_I:
			vm_pushfloat(vm, INFINITY);
			break;
		case O_DCONST:        // constants have native endianness
			d = vm_read_float(vm);
			vm_pushfloat(vm, d);
			break;
		case O_ICONST_B1:
			vm_pushint(vm, (signed char)NCODE(vm));
			break;
		case O_ICONST_B2:
			vm_pushint(vm, vm_read_int16(vm));
			break;
		case O_ICONST_B4:
			vm_pushint(vm, vm_read_int32(vm));
			break;
		case O_ICONST_B8:
			vm_pushint(vm, vm_read_int64(vm));
			break;
		case O_ICONST:        // constants have native endianness
			vm_pushint(vm, vm_read_int(vm));
			break;
		case O_BCONST_F:
		case O_BCONST_T:
			vm_pushbool(vm, opcode & 0x01);
			break;
		case O_NCONST:
			vm_pushundef(vm);
			break;
		case O_FCONST:
			c = vm_read_int(vm);
			vm_pushfn(vm, vm->code + c);
			break;
		case O_CCONST:
			c = vm_read_int(vm);
			vm_pushclosure(vm, vm->code + c);
			break;
		case O_BOR:
			if ((res = vm_int_binop(vm, &bor, "|", OP_BIN_BAR))) return res;
			break;
		case O_BXOR:
			if ((res = vm_int_binop(vm, &bxor, "^", OP_BIN_CARET))) return res;
			break;
		case O_BAND:
			if ((res = vm_int_binop(vm, &band, "&", OP_BIN_AMP))) return res;
			break;
		case O_BANDNOT:
			if ((res = vm_int_binop(vm, &bandnot, "&^", OP_BIN_AMPCARET))) return res;
			break;
		case O_BNOT:
			if ((res = vm_int_unop(vm, &bnot, "^", OP_UN_CARET))) return res;
			break;
		case O_BSL:
			if ((res = vm_int_binop(vm, &shift_left, "<<", OP_BIN_SHL))) return res;
			break;
		case O_BSR:
			if ((res = vm_int_binop(vm, &shift_right, ">>", OP_BIN_SHR))) return res;
			break;
		case O_ADD:
			if ((res = vm_num_binop(vm, &int_add, &float_add, "+", OP_BIN_PLUS))) return res;
			break;
		case O_MUL:
			if ((res = vm_num_binop(vm, &int_mul, &float_mul, "*", OP_BIN_TIMES))) return res;
			break;
		case O_SUB:
			if ((res = vm_num_binop(vm, &int_sub, &float_sub, "-", OP_BIN_MINUS))) return res;
			break;
		case O_FDIV:
			if ((res = vm_fdiv(vm))) return res;   // handled differently because we always convert to float
			break;
		case O_IDIV:
			if (vm_isint(vm) && vm_peekint(vm) == 0) {
				vm_print_err_divide_by_zero(vm);
				return YASL_DIVIDE_BY_ZERO_ERROR;
				break;
			}
			if ((res = vm_int_binop(vm, &idiv, "//", OP_BIN_IDIV))) return res;
			break;
		case O_MOD:
			// TODO: handle undefined C behaviour for negative numbers.
			if (vm_isint(vm) && vm_peekint(vm) == 0) {
				vm_print_err_divide_by_zero(vm);
				return YASL_DIVIDE_BY_ZERO_ERROR;
				break;
			}
			if ((res = vm_int_binop(vm, &modulo, "%", OP_BIN_MOD))) return res;
			break;
		case O_EXP:
			if ((res = vm_pow(vm))) return res;
			break;
		case O_NEG:
			if ((res = vm_num_unop(vm, &int_neg, &float_neg, "-", OP_UN_MINUS))) return res;
			break;
		case O_POS:
			if ((res = vm_num_unop(vm, &int_pos, &float_pos, "+", OP_UN_PLUS))) return res;
			break;
		case O_NOT:
			c = isfalsey(vm_pop(vm));
			vm_pushbool(vm, c);
			break;
		case O_LEN:
			if ((res = vm_len_unop(vm))) return res;
			break;
		case O_CNCT:
			if ((res = vm_CNCT(vm))) return res;
			break;
		case O_GT:
			b = vm_pop(vm);
			a = vm_pop(vm);
			if (YASL_ISSTR(a) && YASL_ISSTR(b)) {
				vm_pushbool(vm, YASL_String_cmp(YASL_GETSTR(a), YASL_GETSTR(b)) > 0);
				break;
			}
			if (!YASL_ISNUM(a) || !YASL_ISNUM(b)) {
				vm_print_err_type(vm,  "< and > not supported for operand of types %s and %s.\n",
				       YASL_TYPE_NAMES[a.type],
				       YASL_TYPE_NAMES[b.type]);
				return YASL_TYPE_ERROR;
			}
			COMP(vm, a, b, GT, ">");
			break;
		case O_GE:
			b = vm_pop(vm);
			a = vm_pop(vm);
			if (YASL_ISSTR(a) && YASL_ISSTR(b)) {
				vm_push(vm, YASL_BOOL(YASL_String_cmp(YASL_GETSTR(a), YASL_GETSTR(b)) >= 0));
				break;
			}
			if (!YASL_ISNUM(a) || !YASL_ISNUM(b)) {
				vm_print_err_type(vm,  "<= and >= not supported for operand of types %s and %s.\n",
				       YASL_TYPE_NAMES[a.type],
				       YASL_TYPE_NAMES[b.type]);
				return YASL_TYPE_ERROR;
			}
			COMP(vm, a, b, GE, ">=");
			break;
		case O_EQ:
			b = vm_pop(vm);
			a = vm_pop(vm);
			v = isequal(a, b);
			vm_push(vm, v);
			break;
		case O_ID: // TODO: clean-up
			b = vm_pop(vm);
			a = vm_pop(vm);
			vm_push(vm, YASL_BOOL(a.type == b.type && YASL_GETINT(a) == YASL_GETINT(b)));
			break;
		case O_NEWSPECIALSTR:
			if ((res = vm_NEWSPECIALSTR(vm))) return res;
			break;
		case O_NEWSTR:
			if ((res = vm_NEWSTR(vm))) return res;
			break;
		case O_NEWTABLE: {
			struct RC_UserData *table = rcht_new();
			struct YASL_Table *ht = (struct YASL_Table *)table->data;
			while (vm_peek(vm).type != Y_END) {
				struct YASL_Object value = vm_pop(vm);
				struct YASL_Object key = vm_pop(vm);
				YASL_Table_insert(ht, key, value);
			}
			vm_pop(vm);
			vm_push(vm, YASL_TABLE(table));
			break;
		}
		case O_NEWLIST: {
			struct RC_UserData *ls = rcls_new();
			while (vm_peek(vm).type != Y_END) {
				YASL_List_append((struct YASL_List *) ls->data, vm_pop(vm));
			}
			YASL_reverse((struct YASL_List *) ls->data);
			vm_pop(vm);
			vm_push(vm, YASL_LIST(ls));
			break;
		}
		case O_INITFOR:
			inc_ref(&vm_peek(vm));
			vm->loopframe_num++;
			vm->loopframes[vm->loopframe_num].iter = 0;
			vm->loopframes[vm->loopframe_num].iterable = vm_pop(vm);
			break;
		case O_ENDFOR:
			dec_ref(&vm->loopframes[vm->loopframe_num].iterable);
			vm->loopframe_num--;
			break;
		case O_ENDCOMP:
			//inc_ref(&vm_peek(vm));
			a = vm_pop(vm);
			vm_pop(vm);
			vm_push(vm, a);
			//dec_ref(&vm_peek(vm));
			dec_ref(&vm->loopframes[vm->loopframe_num].iterable);
			vm->loopframe_num--;
			break;
		case O_ITER_1:
			if ((res = vm_ITER_1(vm))) return res;
			break;
		case O_ITER_2:
			puts("NOT IMPLEMENTED");
			exit(1);
		case O_END:
			vm_pushend(vm);
			break;
		case O_DUP: {
			a = vm_peek(vm);
			vm_push(vm, a);
			break;
		}
		case O_BR_8:
			c = vm_read_int(vm);
			vm->pc += c;
			break;
		case O_BRF_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (isfalsey(v)) vm->pc += c;
			break;
		case O_BRT_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (!(isfalsey(v))) vm->pc += c;
			break;
		case O_BRN_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (!YASL_ISUNDEF(v)) vm->pc += c;
			break;
		case O_GLOAD_8:
			if ((res = vm_GLOAD_8(vm))) return res;
			break;
		case O_GSTORE_8:
			if ((res = vm_GSTORE_8(vm))) return res;
			break;
		case O_GLOAD_1:
			addr = vm->code[vm->pc++ - vm->code];
			vm_push(vm, vm->global_vars[addr]);
			break;
		case O_GSTORE_1:
			addr = vm->code[vm->pc++ - vm->code];
			dec_ref(&vm->global_vars[addr]);
			vm->global_vars[addr] = vm_pop(vm);
			inc_ref(&vm->global_vars[addr]);
			break;
		case O_LLOAD_1:
			offset = NCODE(vm);
			vm_push(vm, vm_peek(vm, vm->fp + offset + 1));
			break;
		case O_LSTORE_1:
			offset = NCODE(vm);
			dec_ref(&vm_peek(vm, vm->fp + offset + 1));
			vm_peek(vm, vm->fp + offset + 1) = vm_pop(vm);
			inc_ref(&vm_peek(vm, vm->fp + offset + 1));
			break;
		case O_ULOAD_1:
			offset = NCODE(vm);
			exit(1);
		case O_INIT_MC:
			if ((res = vm_INIT_MC(vm))) return res;
			break;
		case O_INIT_MC_SPECIAL:
			if ((res = vm_INIT_MC_SPECIAL(vm))) return res;
			break;
		case O_INIT_CALL:
			if ((res = vm_INIT_CALL(vm))) return res;
			break;
		case O_CALL:
			if ((res = vm_CALL(vm))) return res;
			break;
		case O_RET:
			if ((res = vm_RET(vm))) return res;
			break;
		case O_CRET:
			vm_CRET(vm);
			break;
		case O_GET:
			if ((res = vm_GET(vm))) return res;
			break;
		case O_SLICE:
			if ((res = vm_SLICE(vm))) return res;
			break;
		case O_SET:
			if ((res = vm_SET(vm))) return res;
			break;
		case O_POP:
			vm_pop(vm);
			break;
		case O_PRINT:
			vm_PRINT(vm);
			break;
		default:
			vm_print_err(vm, "ERROR UNKNOWN OPCODE: %x\n", opcode);
			return YASL_ERROR;
		}
	}
}
