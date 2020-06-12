#include "VM.h"

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stdarg.h>

#include "interpreter/builtins.h"
#include "data-structures/YASL_String.h"
#include "data-structures/YASL_Table.h"
#include "interpreter/refcount.h"

#include "util/varint.h"
#include "interpreter/table_methods.h"
#include "interpreter/list_methods.h"
#include "interpreter/str_methods.h"
#include "yasl_state.h"
#include "yasl_error.h"
#include "yasl_include.h"
#include "opcode.h"
#include "operator_names.h"
#include "YASL_Object.h"
#include "closure.h"

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
	vm->metatables = YASL_Table_new();
	vm->headers[datasize - 1] = code;
	vm->globals[0] = YASL_Table_new();
	vm->pc = code + pc;
	vm->fp = -1;
	vm->sp = -1;
	vm->constants = NULL;
	vm->stack = (struct YASL_Object *)calloc(sizeof(struct YASL_Object), STACK_SIZE);

#define DEF_SPECIAL_STR(enum_val, str) vm->special_strings[enum_val] = YASL_String_new_sized(strlen(str), str)

	DEF_SPECIAL_STR(S___ADD, "__add");
	DEF_SPECIAL_STR(S___BOR, "__bor");
	DEF_SPECIAL_STR(S___EQ, "__eq");
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
	// YASL_ASSERT(vm->prev_fp == -1, "VM frame pointer should have been -1.");
	// YASL_ASSERT(vm->frame_num == (size_t)-1, "frame num should be 0.");
	for (size_t i = 0; i < STACK_SIZE; i++) dec_ref(vm->stack + i);
	free(vm->stack);

	for (int64_t i = 0; i < vm->num_constants; i++) {
		dec_ref(vm->constants + i);
	}
	free(vm->constants);

	for (size_t i = 0; i < vm->headers_size; i++) {
		free(vm->headers[i]);
	}
	free(vm->headers);

	for (size_t i = 0; i < vm->num_globals; i++) {
		YASL_Table_del(vm->globals[i]);
	}
	free(vm->globals);

	YASL_Table_del(vm->metatables);

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

YASL_FORMAT_CHECK static void vm_print_err_wrapper(struct VM *vm, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vm->err.print(&vm->err, fmt, args);
	va_end(args);
}

static void vm_exitframe(struct VM *const vm);

static size_t vm_getcurrline(struct VM *vm) {
	size_t start = ((int64_t *)vm->code)[0];
	size_t line_start = ((int64_t *)vm->code)[1];
	const unsigned char *tmp = vm->code + line_start;
	long unsigned i = 0;
	while (vint_decode(tmp) < vm->pc - vm->code - start) {
		tmp = vint_next(tmp);
		i++;
	}
	return i;
}

static void printline(struct VM *vm) {
	size_t line =  vm_getcurrline(vm);

	vm_print_err_wrapper(vm, " (line %" PRI_SIZET ")\n", line);

	if (vm->fp >= 0 && vm->stack[vm->fp].type == Y_CFN) vm_exitframe(vm);

	while (vm->fp >= 0) {
		vm_exitframe(vm);
		size_t line = vm_getcurrline(vm);
		vm_print_err_wrapper(vm, "In function call on line %" PRI_SIZET "\n", line);
	}
}

void vvm_print_err(struct VM *vm, const char *const fmt, va_list args) {
	vm->err.print(&vm->err, fmt, args);
	printline(vm);
}

YASL_FORMAT_CHECK void vm_print_err(struct VM *vm, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vvm_print_err(vm, fmt, args);
	va_end(args);
}

YASL_FORMAT_CHECK static void vm_print_out(struct VM *vm, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vm->out.print(&vm->out, fmt, args);
	va_end(args);
}

void vm_push(struct VM *const vm, const struct YASL_Object val) {
	vm->sp++;
	if (vm->sp >= STACK_SIZE) {
		vm->status = YASL_STACK_OVERFLOW_ERROR;
		vm_print_err(vm, "StackOverflow.");
		longjmp(vm->buf, 1);
	}

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

struct YASL_Object *vm_pop_p(struct VM *const vm) {
	return vm->stack + vm->sp--;
}
struct YASL_Object vm_pop(struct VM *const vm) {
	return vm->stack[vm->sp--];
}
bool vm_popbool(struct VM *const vm) {
	return obj_getbool(vm_pop_p(vm));
}

yasl_float vm_popfloat(struct VM *const vm) {
	return obj_getfloat(vm_pop_p(vm));
}

yasl_int vm_popint(struct VM *const vm) {
	return obj_getint(vm_pop_p(vm));
}

struct YASL_String *vm_popstr(struct VM *const vm) {
	return obj_getstr(vm_pop_p(vm));
}

struct YASL_List *vm_poplist(struct VM *const vm) {
	return YASL_GETLIST(vm_pop(vm));
}

struct YASL_Table *vm_poptable(struct VM *const vm) {
	return YASL_GETTABLE(vm_pop(vm));
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

static int vm_lookup_method(struct VM *const vm, const char *const method_name);
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
	if (obj_isint(&left) && obj_isint(&right)) {
		vm_push(vm, YASL_INT(op(obj_getint(&left), obj_getint(&right))));
		return YASL_SUCCESS;
	} else {
		inc_ref(&left);
		inc_ref(&right);
		vm_push(vm, left);
		if (vm_lookup_method(vm, overload_name)) {
			vm_print_err_type(vm, "%s not supported for operands of types %s and %s.",
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
	if (obj_isint(&left) && obj_isint(&right)) {
		vm_pushint(vm, int_op(obj_getint(&left), obj_getint(&right)));
	} else if (obj_isnum(&left) && obj_isnum(&right)) {
		vm_pushfloat(vm, float_op(obj_getnum(&left), obj_getnum(&right)));
	} else {
		inc_ref(&left);
		inc_ref(&right);
		vm_push(vm, left);
		if (vm_lookup_method(vm, overload_name)) {
			vm_print_err_type(vm, "%s not supported for operands of types %s and %s.",
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
	if (obj_isnum(&left) && obj_isnum(&right)) {
		vm_pushfloat(vm, obj_getnum(&left) / obj_getnum(&right));
	} else {
		inc_ref(&left);
		inc_ref(&right);
		vm_push(vm, left);
		if (vm_lookup_method(vm, overload_name)) {
			vm_print_err_type(vm, "/ not supported for operands of types %s and %s.",
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
	if (obj_isint(&left) && obj_isint(&right) && obj_getint(&right) < 0) {
		vm_pop(vm);
		vm_pushfloat(vm, pow((double)obj_getint(&left), (double)obj_getint(&right)));
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
	if (obj_isint(&a)) {
		vm_pushint(vm, op(obj_getint(&a)));
		return YASL_SUCCESS;
	} else {
		vm_push(vm, a);
		if (vm_lookup_method(vm, overload_name)) {
			vm_print_err_type(vm, "%s not supported for operand of type %s.",
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
	if (obj_isint(&expr)) {
		vm_pushint(vm, int_op(obj_getint(&expr)));
	} else if (obj_isfloat(&expr)) {
		vm_pushfloat(vm, float_op(obj_getfloat(&expr)));
	} else {
		vm_push(vm, expr);
		if (vm_lookup_method(vm, overload_name)) {
			vm_print_err_type(vm,  "%s not supported for operand of type %s.",
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
	if (obj_isstr(&v)) {
		vm_pushint(vm, (yasl_int) YASL_String_len(obj_getstr(&v)));
	} else if (obj_istable(&v)) {
		vm_pushint(vm, (yasl_int)YASL_GETTABLE(v)->count);
	} else if (obj_islist(&v)) {
		vm_pushint(vm, (yasl_int)YASL_GETLIST(v)->count);
	} else {
		vm_push(vm, v);
		if (vm_lookup_method(vm, "__len")) {
			vm_print_err_type(vm,  "len not supported for operand of type %s.",
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

int vm_EQ(struct VM *const vm) {
	struct YASL_Object b = vm_pop(vm);
	struct YASL_Object a = vm_pop(vm);
	if (obj_isuserdata(&a) && obj_isuserdata(&b) ||
	    obj_istable(&a) && obj_istable(&b) ||
	    obj_islist(&a) && obj_islist(&b)) {
		inc_ref(&a);
		inc_ref(&b);
		vm_push(vm, a);
		if (vm_lookup_method(vm, "__eq")) {
			vm_print_err_type(vm, "== not supported for operands of types %s and %s.", YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);
			dec_ref(&a);
			dec_ref(&b);
			return YASL_TYPE_ERROR;
		} else {
			vm_INIT_CALL(vm);
			vm_push(vm, a);
			vm_push(vm, b);
			vm_CALL(vm);
			dec_ref(&a);
			dec_ref(&b);
		}
	} else {
		vm_pushbool(vm, isequal(&a, &b));
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

#define MAKE_COMP(name, opstr, op, overload_name) \
static int vm_##name(struct VM *const vm) {\
	struct YASL_Object right = vm_pop(vm);\
	struct YASL_Object left = vm_pop(vm);\
	bool c;\
	if (obj_isstr(&left) && obj_isstr(&right)) {\
		vm_pushbool(vm, YASL_String_cmp(obj_getstr(&left), obj_getstr(&right)) op 0);\
		return YASL_SUCCESS;\
	}\
	if (obj_isnum(&left) && obj_isnum(&right)) {\
		COMP(vm, left, right, name);\
		return YASL_SUCCESS;\
	}\
	inc_ref(&left);\
	inc_ref(&right);\
	vm_push(vm, left);\
	if (vm_lookup_method(vm, overload_name)) {\
		vm_print_err_type(vm, "%s not supported for operands of types %s and %s.",\
		opstr,\
		YASL_TYPE_NAMES[left.type],\
		YASL_TYPE_NAMES[right.type]);\
		dec_ref(&left);\
		dec_ref(&right);\
		return YASL_TYPE_ERROR;\
	} else {\
		int res;\
		vm_INIT_CALL(vm);\
		vm_push(vm, left);\
		vm_push(vm, right);\
		res = vm_CALL(vm);\
		dec_ref(&left);\
		dec_ref(&right);\
		return res;\
	}\
	return YASL_SUCCESS;\
}

MAKE_COMP(GT, "< and >", >, "__gt")
MAKE_COMP(GE, "<= and >=", >=, "__ge")

int vm_stringify_top(struct VM *const vm) {
	enum YASL_Types index = vm_peek(vm, vm->sp).type;
	if (vm_isfn(vm) || vm_iscfn(vm) || vm_isclosure(vm)) {
		size_t n = (size_t)snprintf(NULL, 0, "<fn: %d>", (int)vm_peekint(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<fn: %d>", (int)vm_pop(vm).value.ival);
		vm_pushstr(vm, YASL_String_new_sized_heap(0, strlen(buffer), buffer));
	} else if (vm_isuserdata(vm)) {
		struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = YASL_Table_search(vm_peek(vm).value.uval->mt, key);
		str_del(obj_getstr(&key));
		if (result.type == Y_END) {
			exit(EXIT_FAILURE);
		}
		YASL_GETCFN(result)->value((struct YASL_State *)vm);
	} else if (vm_isuserptr(vm)) {
		// TODO clean up
		size_t n = (size_t)snprintf(NULL, 0, "<userptr: %p>", (void *)vm_peekint(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<userptr: %p>", (void *)vm_popint(vm));
		vm_pushstr(vm, YASL_String_new_sized_heap(0, strlen(buffer), buffer));
	} else {
		struct YASL_Object key = YASL_STR(YASL_String_new_sized(strlen("tostr"), "tostr"));
		struct YASL_Object result = YASL_Table_search(vm->builtins_htable[index], key);
		str_del(obj_getstr(&key));
		YASL_GETCFN(result)->value((struct YASL_State *)vm);
	}
	return YASL_SUCCESS;
}

static struct Upvalue *add_upvalue(struct VM *const vm, struct YASL_Object *const location) {
	if (vm->pending == NULL) {
		return (vm->pending = upval_new(location));
	}

	struct Upvalue *curr = vm->pending;
	while (curr) {
		if (curr->location > location) {
			curr = curr->next;
			continue;
		}
		if (curr->location == location) {
			return curr;
		}
		if (curr->location < location) {
			struct Upvalue *upval = upval_new(location);
			upval->next = curr->next;
			curr->next = upval;
			return upval;
		}
		return (curr->next = upval_new(location));
	}
	return NULL;
}

static int vm_CCONST(struct VM *const vm) {
	yasl_int len = vm_read_int(vm);
	unsigned char *start = vm->pc;
	vm->pc += len;

	const size_t num_upvalues = NCODE(vm);
	struct Closure *closure = (struct Closure *)malloc(sizeof(struct Closure) + num_upvalues*sizeof(struct Upvalue *));
	closure->f = start;
	closure->num_upvalues = num_upvalues;
	closure->rc = rc_new();

	for (size_t i = 0; i < num_upvalues; i++) {
		unsigned char u = NCODE(vm);
		if ((signed char)u >= 0) {
			closure->upvalues[i] = add_upvalue(vm, &vm_peek(vm, vm->fp + 1 + u));
		} else {
			closure->upvalues[i] = vm->stack[vm->fp].value.lval->upvalues[~(signed char)u];
		}
		closure->upvalues[i]->rc->refs++;
	}

	vm_push(vm, ((struct YASL_Object){.type = Y_CLOSURE, .value = {.lval = closure}}));

	return YASL_SUCCESS;
}

static int vm_SLICE(struct VM *const vm) {
	if (vm_islist(vm, vm->sp - 2)) {
		yasl_int len = vm_peeklist(vm, vm->sp - 2)->count;
		yasl_int end;
		yasl_int start;

		if (vm_isundef(vm)) {
			end = len;
		} else if (vm_isint(vm)) {
			end = vm_peekint(vm);
			if (end < 0) end += len;
			if (end > len) end = len;
		} else {
			vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp - 1).type],
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp).type]
			);
			return YASL_TYPE_ERROR;
		}

		if (vm_isundef(vm, vm->sp - 1)) {
			start = 0;
		} else if (vm_isint(vm, vm->sp - 1)) {
			start = vm_peekint(vm, vm->sp -1);
			if (start < 0) start += len;

			if (start < 0) start = 0;
		} else {
			vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp - 1).type],
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp).type]
			);
			return YASL_TYPE_ERROR;
		}

		vm_pop(vm);
		vm_pop(vm);

		struct YASL_List *list = vm_poplist(vm);
		struct RC_UserData *new_ls = rcls_new();

		for (yasl_int i = start; i <end; ++i) {
			YASL_List_append((struct YASL_List *) new_ls->data, list->items[i]);
		}
		vm_push(vm, YASL_LIST(new_ls));
		return YASL_SUCCESS;
	}

	if (vm_isstr(vm, vm->sp - 2)) {
		yasl_int len = YASL_String_len(vm_peekstr(vm, vm->sp - 2));
		yasl_int end;
		yasl_int start;

		if (vm_isundef(vm)) {
			end = len;
		} else if (vm_isint(vm)) {
			end = vm_peekint(vm);
			if (end < 0) end += len;
			if (end > len) end = len;
		} else {
			vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp - 1).type],
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp).type]
			);
			return YASL_TYPE_ERROR;
		}

		if (vm_isundef(vm, vm->sp - 1)) {
			start = 0;
		} else if (vm_isint(vm, vm->sp - 1)) {
			start = vm_peekint(vm, vm->sp -1);
			if (start < 0) start += len;

			if (start < 0) start = 0;
		} else {
			vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp - 1).type],
					  YASL_TYPE_NAMES[vm_peek(vm, vm->sp).type]
			);
			return YASL_TYPE_ERROR;
		}

		vm_pop(vm);
		vm_pop(vm);

		struct YASL_String *str = vm_popstr(vm);

		vm_push(vm, YASL_STR(YASL_String_new_substring((size_t)start, (size_t)end, str)));
		return YASL_SUCCESS;
	}

	vm_pop(vm);
	vm_pop(vm);
	vm_print_err_type(vm,  "slice is not defined for objects of type %s.", YASL_TYPE_NAMES[vm_pop(vm).type]);
	return YASL_TYPE_ERROR;

}

static void nop_del_data(void *data) {
	(void) data;
}

void vm_get_metatable(struct VM *const vm) {
	struct YASL_Object v = vm_pop(vm);
	switch (v.type) {
	case Y_USERDATA:
	case Y_USERDATA_W:
	case Y_LIST:
	case Y_LIST_W:
	case Y_TABLE:
	case Y_TABLE_W:
		vm_push(vm, YASL_TABLE(ud_new(YASL_GETUSERDATA(v)->mt, T_TABLE, vm->builtins_htable[Y_TABLE], nop_del_data)));
		break;
	default:
		vm_push(vm, YASL_TABLE(ud_new(vm->builtins_htable[v.type], T_TABLE, vm->builtins_htable[Y_TABLE], nop_del_data)));
		break;
	}
}

static int lookup(struct VM *vm, struct YASL_Object obj, struct YASL_Table *mt, struct YASL_Object index) {
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_push(vm,search);
		return YASL_SUCCESS;
	}

	struct YASL_String *get = YASL_String_new_sized(strlen("__get"), "__get");
	search = YASL_Table_search(mt, YASL_STR(get));
	str_del(get);
	int result;
	if (search.type != Y_END) {
		vm_push(vm, search);
		if ((result = vm_INIT_CALL(vm))) return result;
		vm_push(vm, obj);
		vm_push(vm, index);
		if ((result = vm_CALL(vm))) return result;
		return YASL_SUCCESS;
	}
	return YASL_VALUE_ERROR;
}

static int vm_lookup_method_helper(struct VM *vm, struct YASL_Object obj, struct YASL_Table *mt, struct YASL_Object index) {
	(void) obj;
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_push(vm,search);
		return YASL_SUCCESS;
	}

	return YASL_VALUE_ERROR;
}

static int vm_lookup_method(struct VM *const vm, const char *const method_name) {
	struct YASL_Object index = YASL_STR(YASL_String_new_sized(strlen(method_name), method_name));
	struct YASL_Object val = vm_peek(vm);

	if (obj_istable(&val)) {
		struct YASL_Object search = YASL_Table_search((struct YASL_Table *)YASL_GETUSERDATA(val)->data, index);
		if (search.type != Y_END) {
			vm_push(vm, search);
			str_del(obj_getstr(&index));
			return YASL_SUCCESS;
		}
	}

	inc_ref(&val);
	vm_get_metatable(vm);
	struct YASL_Table *mt = YASL_GETTABLE(vm_pop(vm));
	void (*old_print)(struct IO *const, const char *const, va_list) = vm->err.print;
	vm->err.print = io_print_none;
	int result = vm_lookup_method_helper(vm, val, mt, index);
	vm->err.print = old_print;
	if (result) {
		dec_ref(&val);
		str_del(obj_getstr(&index));
		return YASL_VALUE_ERROR;
	}
	dec_ref(&val);
	str_del(obj_getstr(&index));
	return YASL_SUCCESS;
}

static int vm_GET_helper(struct VM *const vm, struct YASL_Object index) {
	struct YASL_Object val = vm_peek(vm);
	inc_ref(&val);

	if (obj_istable(&val)) {
		struct YASL_Object search = YASL_Table_search(YASL_GETTABLE(val), index);
		if (search.type != Y_END) {
			vm_pop(vm);
			vm_push(vm, search);
			dec_ref(&val);
			return YASL_SUCCESS;
		}
	}

	vm_get_metatable(vm);
	struct YASL_Table *mt = YASL_GETTABLE(vm_pop(vm));
	void (*old_print)(struct IO *const, const char *const, va_list) = vm->err.print;
	vm->err.print = io_print_none;
	int result = lookup(vm, val, mt, index);
	vm->err.print = old_print;
	if (result) {
		vm_print_err_value(vm, "unable to index %s with value of type %s.", YASL_TYPE_NAMES[val.type], YASL_TYPE_NAMES[index.type]);
		dec_ref(&val);
		return YASL_VALUE_ERROR;
	}
	dec_ref(&val);
	return YASL_SUCCESS;
}

static int vm_GET(struct VM *const vm) {
	struct YASL_Object index = vm_pop(vm);
	inc_ref(&index);
	int res = vm_GET_helper(vm, index);
	dec_ref(&index);
	return res;
}

static int vm_SET(struct VM *const vm) {
	vm->sp -= 2;
	if (vm_islist(vm)) {
		vm->sp += 2;
		return list___set((struct YASL_State *) vm);
	} else if (vm_istable(vm)) {
		vm->sp += 2;
		return table___set((struct YASL_State *) vm);
	}
	vm->sp += 2;
	vm_print_err_type(vm,  "object of type %s is immutable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
	return YASL_TYPE_ERROR;
}

static int vm_NEWSTR(struct VM *const vm) {
	// yasl_int table = vm_read_int(vm);
	yasl_int addr = vm_read_int(vm);

	vm_push(vm, vm->constants[addr]);
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
		if ((yasl_int) YASL_String_len(obj_getstr(&frame->iterable)) <= frame->iter) {
			vm_push(vm, YASL_BOOL(0));
		} else {
			size_t i = (size_t)frame->iter;
			vm_push(vm, YASL_STR(YASL_String_new_substring(i, i + 1, obj_getstr(&frame->iterable))));
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
	// yasl_int table = vm_read_int(vm);
	yasl_int addr = vm_read_int(vm);

	YASL_Table_insert_fast(vm->globals[0], vm->constants[addr], vm_pop(vm));

	return YASL_SUCCESS;
}

static int vm_GLOAD_8(struct VM *const vm) {
	yasl_int addr = vm_read_int(vm);

	vm_push(vm, YASL_Table_search(vm->globals[0], vm->constants[addr]));

	YASL_ASSERT(vm_peek(vm).type != Y_END, "global not found");

	return YASL_SUCCESS;
}

void vm_CLOSE(struct VM *const vm) {
	(void) vm;
	// TODO
}

static void vm_enterframe(struct VM *const vm) {
	if (++vm->frame_num >= NUM_FRAMES) {
		vm->frame_num--;
		vm->status = YASL_STACK_OVERFLOW_ERROR;
		vm_print_err(vm, "StackOverflow.");
		longjmp(vm->buf, 1);
	}

	int next_fp = vm->next_fp;
	vm->next_fp = vm->sp;
	vm->frames[vm->frame_num] = ((struct CallFrame) { vm->pc, vm->fp, next_fp, vm->loopframe_num });
}

static void vm_exitframe(struct VM *const vm) {
	struct YASL_Object v = vm_pop(vm);
	vm->sp = vm->fp;
	vm_pop(vm);

	vm->pc = vm->frames[vm->frame_num].pc;
	vm->fp = vm->frames[vm->frame_num].prev_fp;
	vm->next_fp = vm->frames[vm->frame_num].curr_fp;
	while (vm->loopframe_num > vm->frames[vm->frame_num].lp) {
		dec_ref(&vm->loopframes[vm->loopframe_num--].iterable);
	}

	vm->frame_num--;

	vm_push(vm, v);
}

static int vm_INIT_CALL(struct VM *const vm) {
	if (!vm_isfn(vm) && !vm_iscfn(vm) && !vm_isclosure(vm)) {
		vm_print_err_type(vm,  "%s is not callable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
		return YASL_TYPE_ERROR;
	}

	vm_enterframe(vm);

	return YASL_SUCCESS;
}

static int vm_INIT_MC(struct VM *const vm) {
	struct YASL_Object top = vm_peek(vm);
	inc_ref(&top);
	yasl_int addr = vm_read_int(vm);
	vm_GET_helper(vm, vm->constants[addr]);
	vm_INIT_CALL(vm);
	vm_push(vm, top);
	dec_ref(&top);
	return YASL_SUCCESS;
}

static int vm_CALL_closure(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;
	unsigned char *const code = vm_peek(vm, vm->fp).value.lval->f;

	while (vm->sp - vm->fp < code[0]) {
		vm_pushundef(vm);
	}

	while (vm->sp - vm->fp > code[0]) {
		vm_pop(vm);
	}

	vm->pc =  code + 2;
	return YASL_SUCCESS;
}

static int vm_CALL_fn(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;
	unsigned char *const code = vm_peek(vm, vm->fp).value.fval;

	while (vm->sp - vm->fp < code[0]) {
		vm_pushundef(vm);
	}

	while (vm->sp - vm->fp > code[0]) {
		vm_pop(vm);
	}

	vm->pc =  code + 2;
	return YASL_SUCCESS;
}

static int vm_CALL_cfn(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;
	if (vm_peekcfn(vm, vm->fp)->num_args == -1) {
		YASL_VM_DEBUG_LOG("vm->sp - vm->prev_fp: %d\n", vm->sp - (vm->fp));
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
	if (vm_isfn(vm, vm->fp)) {
		return vm_CALL_fn(vm);
	} else if (vm_iscfn(vm, vm->fp)) {
		return vm_CALL_cfn(vm);
	} else if (vm_isclosure(vm, vm->fp)) {
		return vm_CALL_closure(vm);
	}
	YASL_ASSERT(false, "We never reach this point");
	return 0;
}

static int vm_RET(struct VM *const vm) {
	// TODO: handle multiple returns
	vm_exitframe(vm);
	return YASL_SUCCESS;
}

static struct Upvalue *vm_close_all_helper(struct YASL_Object *const end, struct Upvalue *const curr) {
	if (curr == NULL) return curr;
	if (curr->location < end) return curr;
	inc_ref(curr->location);
	curr->closed = upval_get(curr);
	curr->location = &curr->closed;
	return (vm_close_all_helper(end, curr->next));
}

void vm_close_all(struct VM *const vm) {
	vm->pending = vm_close_all_helper(vm->stack + vm->fp, vm->pending);
}

static int vm_CRET(struct VM *const vm) {
	vm_close_all(vm);
	vm_exitframe(vm);

	return YASL_SUCCESS;
}

static void vm_PRINT(struct VM *const vm) {
	vm_stringify_top(vm);
	struct YASL_String *v = vm_popstr(vm);
	vm_print_out(vm, "%.*s\n", (int)YASL_String_len(v), v->start + v->str);
}

int vm_run(struct VM *const vm) {
	if (setjmp(vm->buf)) {
		return vm->status;
	}

	vm->num_constants = ((int64_t *)vm->code)[2];
	vm->constants = (struct YASL_Object *)malloc(sizeof(struct YASL_Object) * vm->num_constants);
	unsigned char *tmp = vm->code + 24;
	for (int64_t i = 0; i < vm->num_constants; i++) {
		int64_t len = *((int64_t *)tmp);
		tmp += sizeof(int64_t);
		char *str = (char *)malloc((size_t)len);
		memcpy(str, tmp, (size_t)len);
		vm->constants[i] = YASL_STR(YASL_String_new_sized_heap(0, (size_t)len, str));
		inc_ref(vm->constants + i);
		tmp += len;
	}

	while (true) {
		unsigned char opcode = NCODE(vm);        // fetch
		signed char offset;
		struct YASL_Object a, b;
		yasl_int c;
		yasl_float d;
		int res;
		YASL_VM_DEBUG_LOG("----------------"
				  "opcode: %x\n"
				  "vm->sp, vm->prev_fp, vm->curr_fp: %d, %d, %d\n\n", opcode, vm->sp, vm->fp, vm->next_fp);
		switch (opcode) {
		case O_EXPORT:
			vm_close_all(vm);
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
			vm_pushbool(vm, (bool)(opcode & 0x01));
			break;
		case O_NCONST:
			vm_pushundef(vm);
			break;
		case O_FCONST:
			c = vm_read_int(vm);
			vm_pushfn(vm, vm->pc);
			vm->pc += c;
			break;
		case O_CCONST:
			vm_CCONST(vm);
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
			}
			if ((res = vm_int_binop(vm, &idiv, "//", OP_BIN_IDIV))) return res;
			break;
		case O_MOD:
			// TODO: handle undefined C behaviour for negative numbers.
			if (vm_isint(vm) && vm_peekint(vm) == 0) {
				vm_print_err_divide_by_zero(vm);
				return YASL_DIVIDE_BY_ZERO_ERROR;
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
			vm_pushbool(vm, isfalsey(vm_pop_p(vm)));
			break;
		case O_LEN:
			if ((res = vm_len_unop(vm))) return res;
			break;
		case O_CNCT:
			if ((res = vm_CNCT(vm))) return res;
			break;
		case O_GT:
			if ((res = vm_GT(vm))) return res;
			break;
		case O_GE:
			if ((res = vm_GE(vm))) return res;
			break;
		case O_EQ:
			if ((res = vm_EQ(vm))) return res;
			break;
		case O_ID:     // TODO: clean-up
			b = vm_pop(vm);
			a = vm_pop(vm);
			vm_pushbool(vm, a.type == b.type && obj_getint(&a) == obj_getint(&b));
			break;
		case O_NEWSTR:
			if ((res = vm_NEWSTR(vm))) return res;
			break;
		case O_NEWTABLE: {
			struct RC_UserData *table = rcht_new();
			table->mt = vm->builtins_htable[Y_TABLE];
			struct YASL_Table *ht = (struct YASL_Table *)table->data;
			while (vm_peek(vm).type != Y_END) {
				struct YASL_Object val = vm_pop(vm);
				struct YASL_Object key = vm_pop(vm);
				if (!YASL_Table_insert(ht, key, val)) {
					vm_print_err_type(vm, "unable to use mutable object of type %s as key.", YASL_TYPE_NAMES[key.type]);
					return YASL_TYPE_ERROR;
				}
			}
			vm_pop(vm);
			vm_push(vm, YASL_TABLE(table));
			break;
		}
		case O_NEWLIST: {
			struct RC_UserData *ls = rcls_new();
			ls->mt = vm->builtins_htable[Y_LIST];
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
			if (isfalsey(vm_pop_p(vm))) vm->pc += c;
			break;
		case O_BRT_8:
			c = vm_read_int(vm);
			if (!(isfalsey(vm_pop_p(vm)))) vm->pc += c;
			break;
		case O_BRN_8:
			c = vm_read_int(vm);
			if (!obj_isundef(vm_pop_p(vm))) vm->pc += c;
			break;
		case O_GLOAD_8:
			if ((res = vm_GLOAD_8(vm))) return res;
			break;
		case O_GSTORE_8:
			if ((res = vm_GSTORE_8(vm))) return res;
			break;
		case O_LLOAD:
			offset = NCODE(vm);
			vm_push(vm, vm_peek(vm, vm->fp + offset + 1));
			break;
		case O_LSTORE:
			offset = NCODE(vm);
			dec_ref(&vm_peek(vm, vm->fp + offset + 1));
			vm_peek(vm, vm->fp + offset + 1) = vm_pop(vm);
			inc_ref(&vm_peek(vm, vm->fp + offset + 1));
			break;
		case O_ULOAD:
			offset = NCODE(vm);
			vm_push(vm, upval_get(vm_peek(vm, vm->fp).value.lval->upvalues[offset]));
			break;
		case O_USTORE:
			offset = NCODE(vm);
			inc_ref(&vm_peek(vm));
			upval_set(vm_peek(vm, vm->fp).value.lval->upvalues[offset], vm_pop(vm));
			break;
		case O_INIT_MC:
			if ((res = vm_INIT_MC(vm))) return res;
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
		case O_ASS:
			if (isfalsey(vm_peek_p(vm))) {
				vm_stringify_top(vm);
				vm_print_err(vm, "AssertError: %.*s.", (int)YASL_String_len(vm_peekstr(vm)), vm_peekstr(vm)->str + vm_peekstr(vm)->start);
				vm_pop(vm);
				return YASL_ASSERT_ERROR;
			}
			vm_pop(vm);
			break;
		default:
			vm_print_err(vm, "ERROR UNKNOWN OPCODE: %x\n", opcode);
			return YASL_ERROR;
		}
	}
}
