#include "VM.h"

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stdarg.h>

#include "interpreter/builtins.h"
#include "data-structures/YASL_String.h"
#include "data-structures/YASL_Table.h"
#include "data-structures/YASL_StringSet.h"
#include "interpreter/refcount.h"

#include "util/varint.h"
#include "common/migrations.h"
#include "interpreter/methods/table_methods.h"
#include "interpreter/methods/list_methods.h"
#include "interpreter/methods/str_methods.h"
#include "yasl_state.h"
#include "yasl_error.h"
#include "yasl_include.h"
#include "opcode.h"
#include "operator_names.h"
#include "YASL_Object.h"
#include "closure.h"

static struct RC_UserData **builtins_htable_new(struct VM *const vm) {
	struct RC_UserData **ht = (struct RC_UserData **) malloc(sizeof(struct RC_UserData *) * NUM_TYPES);
	ht[Y_UNDEF] = ud_new(undef_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_UNDEF]->rc.refs++;
	ht[Y_FLOAT] = ud_new(float_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_FLOAT]->rc.refs++;
	ht[Y_INT] = ud_new(int_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_INT]->rc.refs++;
	ht[Y_BOOL] = ud_new(bool_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_BOOL]->rc.refs++;
	ht[Y_STR] = ud_new(str_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_STR]->rc.refs++;
	ht[Y_LIST] = ud_new(list_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_LIST]->rc.refs++;
	ht[Y_TABLE] = ud_new(table_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_TABLE]->rc.refs++;
	return ht;
}

void vm_init(struct VM *const vm,
	     unsigned char *const code,    // pointer to bytecode
             const size_t pc,              // address of instruction to be executed first (entrypoint)
             const size_t datasize) {      // total params size required to perform a program operations
	vm->code = code;
	vm->headers = (unsigned char **)calloc(sizeof(unsigned char *), datasize);
	vm->headers_size = datasize;
	vm->frame_num = -1;
	vm->loopframe_num = -1;
	vm->out = NEW_IO(stdout);
	vm->err = NEW_IO(stderr);
	for (size_t i = 0; i < datasize; i++) {
		vm->headers[i] = NULL;
	}
	vm->metatables = YASL_Table_new();
	vm->headers[datasize - 1] = code;
	vm->globals = YASL_Table_new();
	vm->pc = code + pc;
	vm->fp = -1;
	vm->sp = -1;
	vm->num_constants = 0;
	vm->constants = NULL;
	vm->stack = (struct YASL_Object *)calloc(sizeof(struct YASL_Object), STACK_SIZE);
	vm->interned_strings = YASL_StringSet_new();
	vm->builtins_htable = builtins_htable_new(vm);
	vm->pending = NULL;
	vm->buf = NULL;
	vm->format_str = NULL;
}

void vm_close_all(struct VM *const vm);

void vm_cleanup(struct VM *const vm) {
	// If we've exited early somehow, without closing over some upvalues, we need to do that first.
	vm_close_all(vm);

	if (vm->format_str)
		vm->format_str->rc.refs--;

	// Exit out of all loops (in case we're exiting with an error).
	while (vm->loopframe_num >= 0) {
		vm_dec_ref(vm, &vm->loopframes[vm->loopframe_num].iterable);
		vm->loopframe_num--;
	}

	for (size_t i = 0; i < STACK_SIZE; i++) {
 		vm_dec_ref(vm, vm->stack + i);
	}
	free(vm->stack);

	for (int64_t i = 0; i < vm->num_constants; i++) {
		vm_dec_ref(vm, vm->constants + i);
	}
	free(vm->constants);

	for (size_t i = 0; i < vm->headers_size; i++) {
		free(vm->headers[i]);
	}
	free(vm->headers);
	YASL_StringSet_del(vm->interned_strings);
	YASL_Table_del(vm->globals);

	YASL_Table_del(vm->metatables);

	struct YASL_Object v;
	v = YASL_TABLE(vm->builtins_htable[Y_UNDEF]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_FLOAT]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_INT]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_BOOL]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_STR]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_LIST]);
	vm_dec_ref(vm, &v);
	v = YASL_TABLE(vm->builtins_htable[Y_TABLE]);
	vm_dec_ref(vm, &v);
	free(vm->builtins_htable);

	io_cleanup(&vm->out);
	io_cleanup(&vm->err);
}

void *vm_alloc_cyclic(struct VM *vm, size_t size) {
	YASL_UNUSED(vm);
	return malloc(size);
}

void vm_free_cyclic(struct VM *vm, void *ptr) {
	YASL_UNUSED(vm);
	free(ptr);
}

YASL_FORMAT_CHECK static void vm_print_err_wrapper(struct VM *vm, const char *const fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vm->err.print(&vm->err, fmt, args);
	va_end(args);
}

static void vm_exitframe(struct VM *const vm);
void vm_executenext(struct VM *const vm);

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

	if (vm->fp >= 0 && vm_peek(vm, vm->fp).type == Y_CFN) vm_exitframe(vm);

	while (vm->fp >= 0) {
		vm_exitframe(vm);
		line = vm_getcurrline(vm);
		vm_print_err_wrapper(vm, "In function call on line %" PRI_SIZET "\n", line);
	}
}

void vvm_print_err(struct VM *vm, const char *const fmt, va_list args) {
	vm->err.print(&vm->err, fmt, args);
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

void vm_throw_err(struct VM *const vm, int error) {
	vm->status = error;
	longjmp(*vm->buf, 1);
}

void vm_dec_ref(struct VM *const vm, struct YASL_Object *val) {
	dec_strong_ref(vm, val);
}

void vm_push(struct VM *const vm, const struct YASL_Object val) {
	if (vm->sp + 1 >= STACK_SIZE) {
		vm_print_err(vm, "StackOverflow.");
		vm_throw_err(vm, YASL_STACK_OVERFLOW_ERROR);
	}

	vm->sp++;

	vm_dec_ref(vm, vm->stack + vm->sp);
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

void vm_pushstr_bb(struct VM *const vm, YASL_ByteBuffer *bb) {
	vm_pushstr(vm, YASL_String_new_takebb(vm, bb));
}

struct YASL_Object *vm_pop_p(struct VM *const vm) {
	YASL_ASSERT(vm->sp >= 0, "cannot pop from empty stack.");
	return vm->stack + vm->sp--;
}

struct YASL_Object vm_pop(struct VM *const vm) {
	YASL_ASSERT(vm->sp >= 0, "cannot pop from empty stack.");
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

void vm_insert(struct VM *const vm, int index, struct YASL_Object val) {
	if (vm->sp + 1 >= STACK_SIZE) {
		vm_print_err(vm, "StackOverflow.");
		vm_throw_err(vm, YASL_STACK_OVERFLOW_ERROR);
	}

	vm_dec_ref(vm, vm->stack + vm->sp + 1);
	memmove(vm->stack + index + 1, vm->stack + index, (vm->sp - index + 1) * sizeof(struct YASL_Object));
	vm->stack[index] = val;
	vm->sp++;
}

void vm_insertbool(struct VM *const vm, int index, bool val) {
	vm_insert(vm, index, YASL_BOOL(val));
}

void vm_insertint(struct VM *const vm, int index, yasl_int val) {
	vm_insert(vm, index, YASL_INT(val));
}

void vm_rm(struct VM *const vm, int index) {
	int after = vm->sp - index;
	vm_dec_ref(vm, vm->stack + index);
	memmove(vm->stack + index, vm->stack + index + 1, after * sizeof(struct YASL_Object));
	vm->stack[vm->sp] = YASL_END();
	vm->sp--;
}

void vm_rm_range(struct VM *const vm, int start, int end) {
	int after = vm->sp - end + 1;
	int len = end - start;
	for (int i = 0; i < after && i < len; i++) {
		vm_dec_ref(vm, vm->stack + start + i);
	}

	for (int i = 0; i < after && i < len; i++) {
		inc_ref(vm->stack + vm->sp - i);
	}
	memmove(vm->stack + start, vm->stack + end, after * sizeof(struct YASL_Object));
	vm->sp -= len;
}

static yasl_int vm_read_int(struct VM *const vm) {
    yasl_int val;
    memcpy(&val, vm->pc, sizeof(yasl_int));
    vm->pc += sizeof(yasl_int);
    return val;
}

// static void vm_dup(struct VM *const vm, int source);
static void vm_duptop(struct VM *const vm);
static void vm_swaptop(struct VM *const vm);
int vm_lookup_method_helper(struct VM *vm, struct YASL_Table *mt, struct YASL_Object index);
static void vm_GET(struct VM *const vm);
static void vm_INIT_CALL(struct VM *const vm, int expected_returns);
void vm_INIT_CALL_offset(struct VM *const vm, int offset, int expected_returns);
void vm_CALL(struct VM *const vm);
void vm_CALL_now(struct VM *const vm);

#define vm_lookup_method_throwing_source(vm, source, method_name, err_str, ...) do {\
	struct YASL_Object index = YASL_STR(YASL_String_new_copy(vm, method_name, strlen(method_name)));\
	struct YASL_Object maybe_mt = vm_get_metatable_index(vm, source);\
	struct YASL_Table *mt = obj_istable(&maybe_mt) ? YASL_GETTABLE(maybe_mt) : NULL;\
	int result = vm_lookup_method_helper(vm, mt, index);\
	if (result) {\
		vm_print_err_type(vm, err_str, __VA_ARGS__);\
		vm_throw_err(vm, YASL_TYPE_ERROR);\
	}\
} while (0)

#define vm_lookup_method_throwing(vm, method_name, err_str, ...) do {\
	struct YASL_Object index = YASL_STR(YASL_String_new_copy(vm, method_name, strlen(method_name)));\
	vm_get_metatable(vm);\
	struct YASL_Table *mt = vm_istable(vm) ? vm_poptable(vm) : NULL;\
	if (!mt) {\
		vm_pop(vm);\
	}\
	int result = vm_lookup_method_helper(vm, mt, index);\
	if (result) {\
		vm_print_err_type(vm, err_str, __VA_ARGS__);\
		vm_throw_err(vm, YASL_TYPE_ERROR);\
	}\
} while (0)

// TODO: make this not rely on "source" being the top of the stack.
#define vm_call_method_now_1_top(vm, target, source, method_name, ...) do {\
	vm_lookup_method_throwing_source(vm, source, method_name, __VA_ARGS__, vm_peektypename(vm, source));\
	vm_swaptop(vm);\
	vm_INIT_CALL_offset(vm, vm->sp - 1, 1);\
	vm_CALL(vm);\
} while (0)

#define vm_call_binop_method_now(vm, left, right, method_name, format, ...) do {\
	struct YASL_Object index = YASL_STR(YASL_String_new_copy(vm, method_name, strlen(method_name)));\
	vm_push(vm, left);\
	vm_get_metatable(vm);\
	struct YASL_Table *mt = vm_istable(vm) ? vm_poptable(vm) : NULL;\
	if (!mt) {\
		vm_pop(vm);\
	}\
	int result = vm_lookup_method_helper(vm, mt, index);\
	if (result) {\
		vm_push(vm, right);\
		vm_get_metatable(vm);\
		mt = vm_istable(vm) ? vm_poptable(vm) : NULL;\
		if (!mt) {\
			vm_pop(vm);\
		}\
		result = vm_lookup_method_helper(vm, mt, index);\
	}\
	if (result) {\
		vm_print_err_type(vm, format, __VA_ARGS__);\
		vm_throw_err(vm, YASL_TYPE_ERROR);\
	}\
	vm_shifttopdown(vm, 2);\
	vm_INIT_CALL_offset(vm, vm->sp - 2, 1);\
	vm_CALL(vm);\
} while (0)

#define INT_BINOP(name, op) yasl_int name(yasl_int left, yasl_int right) { return left op right; }

INT_BINOP(bor, |)
INT_BINOP(bxor, ^)
INT_BINOP(band, &)
INT_BINOP(bandnot, &~)
INT_BINOP(shift_left, <<)
INT_BINOP(shift_right, >>)
INT_BINOP(modulo, %)
INT_BINOP(idiv, /)

typedef yasl_int int_binop(yasl_int, yasl_int);
typedef yasl_float float_binop(yasl_float, yasl_float);

static void vm_shifttopdown(struct VM *const vm, int depth) {
	struct YASL_Object top = vm_peek(vm);
	inc_ref(&top);
	memmove(vm->stack + vm->sp - depth + 1, vm->stack + vm->sp - depth, depth * sizeof(struct YASL_Object));

	vm->stack[vm->sp - depth] = top;
	vm_dec_ref(vm, &top);
}

static void vm_ECHO(struct VM *const vm);

static void vm_int_binop(struct VM *const vm, int_binop op, const char *opstr, const char *overload_name) {
	struct YASL_Object right = vm_peek(vm);
	struct YASL_Object left = vm_peek(vm, vm->sp - 1);
	if (obj_isint(&left) && obj_isint(&right)) {
		vm_pop(vm);
		vm_pop(vm);
		vm_pushint(vm, op(obj_getint(&left), obj_getint(&right)));
	} else {
		vm_call_binop_method_now(vm, left, right, overload_name, "%s not supported for operands of types %s and %s.", opstr,
					 obj_typename(&left),
					 obj_typename(&right));
	}
}

#define FLOAT_BINOP(name, op) yasl_float name(yasl_float left, yasl_float right) { return left op right; }
#define NUM_BINOP(name, op) INT_BINOP(int_ ## name, op) FLOAT_BINOP(float_ ## name, op)

NUM_BINOP(add, +)
NUM_BINOP(sub, -)
NUM_BINOP(mul, *)

static yasl_int int_pow(yasl_int left, yasl_int right) {
    return (yasl_int)pow((double)left, (double)right);
}

static void vm_num_binop(struct VM *const vm, int_binop int_op, float_binop float_op,
			 const char *const opstr, const char *overload_name) {
	struct YASL_Object right = vm_peek(vm);
	struct YASL_Object left = vm_peek(vm, vm->sp - 1);
	if (obj_isint(&left) && obj_isint(&right)) {
		vm_pop(vm);
		vm_pop(vm);
		vm_pushint(vm, int_op(obj_getint(&left), obj_getint(&right)));
	} else if (obj_isnum(&left) && obj_isnum(&right)) {
		vm_pop(vm);
		vm_pop(vm);
		vm_pushfloat(vm, float_op(obj_getnum(&left), obj_getnum(&right)));
	} else {
		vm_call_binop_method_now(vm, left, right, overload_name, "%s not supported for operands of types %s and %s.", opstr,
					 obj_typename(&left),
					 obj_typename(&right));
	}
}

static void vm_fdiv(struct VM *const vm) {
	const char *overload_name = OP_BIN_FDIV;
	struct YASL_Object right = vm_peek(vm);
	struct YASL_Object left = vm_peek(vm, vm->sp - 1);
	if (obj_isnum(&left) && obj_isnum(&right)) {
		vm_pop(vm);
		vm_pop(vm);
		vm_pushfloat(vm, obj_getnum(&left) / obj_getnum(&right));
	} else {
		vm_call_binop_method_now(vm, left, right, overload_name, "/ not supported for operands of types %s and %s.",
					 obj_typename(&left),
					 obj_typename(&right));
	}
}

static void vm_pow(struct VM *const vm) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_peek(vm);
	if (obj_isint(&left) && obj_isint(&right) && obj_getint(&right) < 0) {
		vm_pop(vm);
		vm_pushfloat(vm, pow((double)obj_getint(&left), (double)obj_getint(&right)));
	} else {
		vm->sp++;
		vm_num_binop(vm, &int_pow, &pow, "**", OP_BIN_POWER);
	}
}

#define INT_UNOP(name, op) yasl_int name(yasl_int val) { return op val; }
#define FLOAT_UNOP(name, op) yasl_float name(yasl_float val) { return op val; }
#define NUM_UNOP(name, op) INT_UNOP(int_ ## name, op) FLOAT_UNOP(float_ ## name, op)

INT_UNOP(bnot, ~)
NUM_UNOP(neg, -)
NUM_UNOP(pos, +)

static void vm_int_unop(struct VM *const vm, int target, int source, yasl_int (*op)(yasl_int), const char *opstr, const char *overload_name) {
	if (vm_isint(vm, vm->fp + 1 + source)) {
		vm->stack[vm->fp + 1 + target] = YASL_INT(op(vm_peekint(vm, vm->fp + 1 + source)));
		return;
	} else {
		vm_call_method_now_1_top(vm, target, source, overload_name, "%s not supported for operand of type %s.", opstr);
	}
}

static void vm_num_unop(struct VM *const vm, int target, int source, yasl_int (*int_op)(yasl_int), yasl_float (*float_op)(yasl_float), const char *opstr, const char *overload_name) {
	if (vm_isint(vm, vm->fp + 1 + source)) {
		vm->stack[vm->fp + 1 + target] = YASL_INT(int_op(vm_peekint(vm, vm->fp + 1 + source)));
	} else if (vm_isfloat(vm, vm->fp + 1 + source)) {
		vm->stack[vm->fp + 1 + target] = YASL_FLOAT(float_op(vm_peekfloat(vm, vm->fp + 1 + source)));
	} else {
		vm_call_method_now_1_top(vm, target, source, overload_name, "%s not supported for operand of type %s.", opstr);
	}
}

void vm_len_unop(struct VM *const vm, int target, int source) {
	YASL_UNUSED(target);
	vm_call_method_now_1_top(vm, target, source, "__len", "len not supported for operand of type %s.");
	/*
	struct YASL_Object index = YASL_STR(YASL_String_new_copy(vm, "__len", strlen("__len")));\
	struct YASL_Object maybe_mt = vm_get_metatable_index(vm, source);\
	struct YASL_Table *mt = obj_istable(&maybe_mt) ? YASL_GETTABLE(maybe_mt) : NULL;\
	int result = vm_lookup_method_helper(vm, mt, index);\
	if (result) {\
		vm_print_err_type(vm, "%s", "");\
		vm_throw_err(vm, YASL_TYPE_ERROR);\
	}\
	vm_swaptop(vm);
	vm_INIT_CALL_offset(vm, vm->sp - 1, 1);
	vm_CALL(vm);
	 */
}

void vm_EQ(struct VM *const vm) {
	struct YASL_Object b = vm_peek(vm);
	struct YASL_Object a = vm_peek(vm, vm->sp - 1);
	if (obj_isuserdata(&a) && obj_isuserdata(&b) ||
	    obj_istable(&a) && obj_istable(&b) ||
	    obj_islist(&a) && obj_islist(&b)) {
		vm_call_binop_method_now(vm, a, b, "__eq", "== not supported for operands of types %s and %s.",
					 obj_typename(&a),
					 obj_typename(&b));
	} else {
		vm_pop(vm);
		vm_pop(vm);
		vm_pushbool(vm, isequal(&a, &b));
	}
}

static void vm_CNCT(struct VM *const vm) {
		vm_stringify_top(vm);
		struct YASL_Object top = vm_peek(vm);
		inc_ref(&top);
		struct YASL_String *b = vm_popstr(vm);
		vm_stringify_top(vm);
		struct YASL_String *a = vm_popstr(vm);

		size_t size = YASL_String_len(a) + YASL_String_len(b);
		char *ptr = (char *)malloc(size);
		memcpy(ptr, YASL_String_chars(a), YASL_String_len(a));
		memcpy(ptr + YASL_String_len(a), YASL_String_chars(b), YASL_String_len(b));
		vm_pushstr(vm, YASL_String_new_take(vm, ptr, size));
		vm_dec_ref(vm, &top);
}

#define DEFINE_COMP(name, opstr, overload_name) \
static void vm_##name(struct VM *const vm) {\
	struct YASL_Object right = vm_peek(vm);\
	struct YASL_Object left = vm_peek(vm, vm->sp -1);\
	bool c;\
	if (obj_isstr(&left) && obj_isstr(&right)) {\
		vm_pop(vm);\
		vm_pop(vm);\
		vm_pushbool(vm, name(YASL_String_cmp(obj_getstr(&left), obj_getstr(&right)), 0));\
		return;\
	}\
	if (obj_isnum(&left) && obj_isnum(&right)) {\
		vm_pop(vm);\
		vm_pop(vm);\
		COMP(vm, left, right, name);\
		return;\
	}\
	vm_call_binop_method_now(vm, left, right, overload_name, "%s not supported for operands of types %s and %s.",\
	opstr,\
	obj_typename(&left),\
	obj_typename(&right));\
}

DEFINE_COMP(GT, ">", "__gt")
DEFINE_COMP(GE, ">=", "__ge")
DEFINE_COMP(LT, "<", "__lt")
DEFINE_COMP(LE, "<=", "__le")

void vm_setformat(struct VM *const vm, const char *format) {
	/* We manually increment and decrement the refs here so that if the format
	 * str is the only remaining reference to that particular string, we do not
	 * free it.
	 */
	if (vm->format_str)
		vm->format_str->rc.refs--;
	if (format != NULL) {
		vm->format_str = YASL_String_new_copyz(vm, format);
		vm->format_str->rc.refs++;
	} else {
		vm->format_str = NULL;
	}
}

struct YASL_Object vm_getformat(struct VM *const vm) {
	return vm->format_str ? YASL_STR(vm->format_str) : YASL_UNDEF();
}

void vm_stringify_top(struct VM *const vm) {
	vm_stringify_top_format(vm, NULL);
}

void vm_stringify_top_format(struct VM *const vm, struct YASL_Object *format) {
	if (vm_isfn(vm) || vm_iscfn(vm) || vm_isclosure(vm)) {
		size_t n = (size_t)snprintf(NULL, 0, "<fn: %p>", vm_peekuserptr(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<fn: %d>", (int)vm_popint(vm));
		vm_pushstr(vm, YASL_String_new_take(vm, buffer, strlen(buffer)));
	} else if (vm_isuserptr(vm)) {
		size_t n = (size_t)snprintf(NULL, 0, "<userptr: %p>", vm_peekuserptr(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<userptr: %p>", (void *)vm_popint(vm));
		vm_pushstr(vm, YASL_String_new_take(vm, buffer, strlen(buffer)));
	} else {
		vm_duptop(vm);
		vm_lookup_method_throwing(vm, "tostr", "tostr not supported for operand of type %s.", vm_peektypename(vm));
		vm_swaptop(vm);
		int offset = 1;
		if (format) {
			offset++;
			vm_push(vm, *format);
		}
		vm_INIT_CALL_offset(vm, vm->sp - offset, 1);
		vm_CALL_now(vm);
	}

	if (!vm_isstr(vm)) {
		vm_print_err_type(vm, "Could not stringify items, got: %s", vm_peektypename(vm));
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}
}

struct Upvalue *add_upvalue(struct VM *const vm, struct YASL_Object *const location) {
	if (vm->pending == NULL) {
		return (vm->pending = upval_new(location));
	}

	struct Upvalue *prev = NULL;
	struct Upvalue *curr = vm->pending;
	while (curr) {
		if (curr->location > location) {
			prev = curr;
			curr = curr->next;
			continue;
		}
		if (curr->location == location) {
			return curr;
		}
		if (curr->location < location) {
			struct Upvalue *upval = upval_new(location);
			if (prev == NULL) {
				vm->pending = upval;
			} else {
				prev->next = upval;
			}
			upval->next = curr;
			return upval;
		}
		return (curr->next = upval_new(location));
	}
	return (prev->next = upval_new(location));
}

void vm_remove_pending_upvalue(struct VM *vm, struct Upvalue *upval) {
	// Hack until we get every possible call site to this passing VM* correctly.
	// Currently, some pass NULL for backwards compatibility.
	if (vm == NULL) return;
	struct Upvalue **p = &vm->pending;
	while (*p != NULL) {
		if (*p == upval)
			*p = upval->next;
		else
			p = &(*p)->next;
	}
}

static void vm_CCONST(struct VM *const vm) {
	yasl_int len = vm_read_int(vm);
	unsigned char *start = vm->pc;
	vm->pc += len;

	const size_t num_upvalues = NCODE(vm);
	struct Closure *closure = (struct Closure *)vm_alloc_cyclic(vm, sizeof(struct Closure) + num_upvalues*sizeof(struct Upvalue *));
	closure->f = start;
	closure->num_upvalues = num_upvalues;
	closure->rc = NEW_RC();

	for (size_t i = 0; i < num_upvalues; i++) {
		unsigned char u = NCODE(vm);
		if ((signed char)u >= 0) {
			closure->upvalues[i] = add_upvalue(vm, &vm_peek_fp(vm, u));
		} else {
			closure->upvalues[i] = vm->stack[vm->fp].value.lval->upvalues[~(signed char)u];
		}
		closure->upvalues[i]->rc.refs++;
	}

	vm_push(vm, ((struct YASL_Object){.type = Y_CLOSURE, .value = {.lval = closure}}));
}

static void vm_SLICE_list(struct VM *const vm) {
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
				  (vm_peektypename(vm, vm->sp - 1)),
				  (vm_peektypename(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	if (vm_isundef(vm, vm->sp - 1)) {
		start = 0;
	} else if (vm_isint(vm, vm->sp - 1)) {
		start = vm_peekint(vm, vm->sp -1);
		if (start < 0) start += len;

		if (start < 0) start = 0;
	} else {
		vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
				  (vm_peektypename(vm, vm->sp - 1)),
				  (vm_peektypename(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	vm_pop(vm);
	vm_pop(vm);

	if (end < start) {
		vm_print_err_value(vm, "slicing expected the end of the range to be greater than or equal to the start, got %d:%d", (int)start, (int)end);
		vm_throw_err(vm, YASL_VALUE_ERROR);
	}

	struct YASL_List *list = vm_poplist(vm);
	struct RC_UserData *new_ls = rcls_new(vm);

	for (yasl_int i = start; i <end; ++i) {
		YASL_List_push((struct YASL_List *) new_ls->data, list->items[i]);
	}
	vm_pushlist(vm, new_ls);
}

static void vm_SLICE_str(struct VM *const vm){
	const yasl_int len = YASL_String_len(vm_peekstr(vm, vm->sp - 2));
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
				  (vm_peektypename(vm, vm->sp - 1)),
				  (vm_peektypename(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	if (vm_isundef(vm, vm->sp - 1)) {
		start = 0;
	} else if (vm_isint(vm, vm->sp - 1)) {
		start = vm_peekint(vm, vm->sp -1);
		if (start < 0) start += len;
		if (start < 0) start = 0;
	} else {
		vm_print_err_type(vm,  "slicing expected range of type int:int, got type %s:%s",
				  (vm_peektypename(vm, vm->sp - 1)),
				  (vm_peektypename(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	vm_pop(vm);
	vm_pop(vm);

	if (end < start) {
		vm_print_err_value(vm, "slicing expected the end of the range to be greater than or equal to the start, got %d:%d", (int)start, (int)end);
		vm_throw_err(vm, YASL_VALUE_ERROR);
	}

	struct YASL_String *str = vm_popstr(vm);

	vm_pushstr(vm, YASL_String_new_substring(vm, str, (size_t)start, (size_t)end));
}

static void vm_SLICE(struct VM *const vm) {
	if (vm_islist(vm, vm->sp - 2)) {
		vm_SLICE_list(vm);
	} else if (vm_isstr(vm, vm->sp - 2)) {
		vm_SLICE_str(vm);
	} else {
		vm_pop(vm);
		vm_pop(vm);
		vm_print_err_type(vm,  "slicing is not defined for objects of type %s.", obj_typename(vm_pop_p(vm)));
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}
}

struct RC_UserData *obj_get_metatable(const struct VM *const vm, struct YASL_Object v) {
	switch (v.type) {
	case Y_USERDATA:
	case Y_LIST:
	case Y_TABLE:
		return YASL_GETUSERDATA(v)->mt;
	default:
		return vm->builtins_htable[v.type];
	}
}

struct YASL_Table *get_mt(const struct VM *const vm, struct YASL_Object v) {
	struct RC_UserData *ud = obj_get_metatable(vm, v);
	return ud ? (struct YASL_Table *)ud->data : NULL;
}

struct YASL_Object vm_get_metatable_index(struct VM *const vm, int source) {
	struct YASL_Object v = vm_peek_fp(vm, source);
	struct RC_UserData *mt = obj_get_metatable(vm, v);
	return mt ? YASL_TABLE(mt) : YASL_UNDEF();
}

void vm_get_metatable(struct VM *const vm) {
	struct YASL_Object v = vm_pop(vm);
	struct RC_UserData *mt = obj_get_metatable(vm, v);
	vm_push(vm, mt ? YASL_TABLE(mt) : YASL_UNDEF());
}

int vm_lookup_method_helper(struct VM *vm, struct YASL_Table *mt, struct YASL_Object index) {
	if (!mt) return YASL_VALUE_ERROR;
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_push(vm,search);
		return YASL_SUCCESS;
	}

	return YASL_VALUE_ERROR;
}

static int lookup(struct VM *vm, struct YASL_Table *mt) {
	struct YASL_Object search = YASL_Table_search_zstring_int(mt, "__get");
	if (search.type != Y_END) {
		vm_push(vm, search);
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
		return YASL_SUCCESS;
	}
	return YASL_VALUE_ERROR;
}

static void vm_GET(struct VM *const vm) {
	struct YASL_Object index = vm_peek(vm);
	struct YASL_Object v = vm_peek(vm, vm->sp - 1);

	struct YASL_Table *mt = get_mt(vm, v);
	int result = YASL_ERROR;
	if (mt) {
		result = lookup(vm, mt);
	}

	if (result) {
		if (obj_istable(&v)) {
			struct YASL_Object search = YASL_Table_search(YASL_GETTABLE(v), index);
			if (search.type != Y_END) {
				vm_pop(vm);
				vm_pop(vm);
				vm_push(vm, search);
				return;
			}
		}
		vm_print_err_value(vm, "Could not find value for index%s", "");
		vm_throw_err(vm, YASL_VALUE_ERROR);
	}
}

static void vm_SET(struct VM *const vm) {
	struct YASL_Object obj = vm_peek(vm, vm->sp - 2);

	vm_push(vm, obj);
	vm_lookup_method_throwing(vm, "__set", "object of type %s is immutable.", obj_typename(&obj));
	vm_shifttopdown(vm, 3);
	vm_INIT_CALL_offset(vm, vm->sp - 3, 0);
	vm_CALL(vm);
}

static void vm_LIT(struct VM *const vm) {
	unsigned char addr = NCODE(vm);
	vm_push(vm, vm->constants[addr]);
}

static void vm_LIT8(struct VM *const vm) {
	yasl_int addr = vm_read_int(vm);
	vm_push(vm, vm->constants[addr]);
}

static void vm_ITER_1(struct VM *const vm) {
	struct LoopFrame *frame = &vm->loopframes[vm->loopframe_num];
	switch (frame->iterable.type) {
	case Y_STR:
	case Y_LIST:
	case Y_TABLE:
	case Y_USERDATA: {
		vm_push(vm, frame->next_fn);
		vm_push(vm, frame->iterable);
		vm_push(vm, frame->curr);
		vm_INIT_CALL_offset(vm, vm->sp - 2, -1);
		vm_CALL_now(vm);
		if (vm_popbool(vm)) {
			dec_ref(&frame->curr);
			struct YASL_Object value = vm_pop(vm);
			frame->curr = vm_pop(vm);
			inc_ref(&frame->curr);
			vm_push(vm, value);
			vm_pushbool(vm, true);
		} else {
			vm_pushbool(vm, false);
		}
		return;
	}
	default:
		vm_print_err_type(vm,  "object of type %s is not iterable.\n", obj_typename(&frame->iterable));
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}
}

static bool vm_MATCH_subpattern(struct VM *const vm, struct YASL_Object *expr);
static void vm_ff_subpatterns_multiple(struct VM *const vm, const size_t n);

static void vm_ff_subpattern(struct VM *const vm) {
	switch((enum Pattern)NCODE(vm)) {
	case P_UNDEF:
	case P_TYPE_BOOL:
	case P_TYPE_INT:
	case P_TYPE_FLOAT:
	case P_TYPE_STR:
	case P_TYPE_LS:
	case P_TYPE_TABLE:
	case P_ONE:
	case P_ANY:
	case P_NOT:
		break;
	case P_BIND:
	case P_BOOL:
		(void)NCODE(vm);
		break;
	case P_LIT:
		(void)NCODE(vm);
		break;
	case P_LIT8:
		(void)vm_read_int(vm);
		break;
	case P_LS:
	case P_VLS:
	case P_TABLE:
	case P_VTABLE:
		vm_ff_subpatterns_multiple(vm, (size_t)vm_read_int(vm));
		break;
	case P_ALT:
		vm_ff_subpatterns_multiple(vm, 2);
		break;
	}
}

static void vm_ff_subpatterns_multiple(struct VM *const vm, const size_t n) {
	for (size_t i = 0; i < n; i++) {
		vm_ff_subpattern(vm);
	}
}

static bool vm_MATCH_table_elements(struct VM *const vm, size_t len, struct YASL_Table *table) {
	for (size_t i = 0; i < len; i++) {
		struct YASL_Object val;
		switch ((enum Pattern)NCODE(vm)) {
		case P_UNDEF:
			val = YASL_Table_search(table, YASL_UNDEF());
			break;
		case P_BOOL:
			val = YASL_Table_search(table, YASL_BOOL(NCODE(vm)));
			break;
		case P_LIT:
			val = YASL_Table_search(table, vm->constants[NCODE(vm)]);
			break;
		case P_LIT8:
			val = YASL_Table_search(table, vm->constants[vm_read_int(vm)]);
			break;
		default:
			break;
		}
		if (val.type == Y_END || !(vm_MATCH_subpattern(vm, &val))) {
			vm_ff_subpatterns_multiple(vm, 2 * (len - (i + 1)));
			return false;
		}
	}
	return true;
}

static bool MATCH_list(struct VM *vm, struct YASL_List *ls, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (!vm_MATCH_subpattern(vm, ls->items + i)) {
			vm_ff_subpatterns_multiple(vm, len - (i + 1));
			return false;
		}
	}
	return true;
}

static bool vm_MATCH_subpattern(struct VM *const vm, struct YASL_Object *expr) {
	unsigned char next = NCODE(vm);
	switch ((enum Pattern)next) {
	case P_UNDEF:
		return obj_isundef(expr);
	case P_TYPE_BOOL:
		return obj_isbool(expr);
	case P_TYPE_INT:
		return obj_isint(expr);
	case P_TYPE_FLOAT:
		return obj_isfloat(expr);
	case P_TYPE_STR:
		return obj_isstr(expr);
	case P_TYPE_LS:
		return obj_islist(expr);
	case P_TYPE_TABLE:
		return obj_istable(expr);
	case P_BOOL: {
		bool tmp = (bool)NCODE(vm);
		return obj_isbool(expr) && obj_getbool(expr) == tmp;
	}
	case P_LIT: {
		unsigned char tmp = NCODE(vm);
		return isequal(vm->constants + tmp, expr);
	}
	case P_LIT8: {
		yasl_int tmp = vm_read_int(vm);
		return isequal(vm->constants + tmp, expr);
	}
	case P_TABLE: {
		size_t len = (size_t) vm_read_int(vm) / 2;
		if (!obj_istable(expr)) {
			vm_ff_subpatterns_multiple(vm, len * 2);
			return false;
		}

		struct YASL_Table *table = (struct YASL_Table *) expr->value.uval->data;
		if (table->count != len) {
			vm_ff_subpatterns_multiple(vm, len * 2);
			return false;
		}

		return vm_MATCH_table_elements(vm, len, table);
	}
	case P_VTABLE: {
		size_t len = (size_t) vm_read_int(vm) / 2;
		if (!obj_istable(expr)) {
			vm_ff_subpatterns_multiple(vm, len * 2);
			return false;
		}

		struct YASL_Table *table = (struct YASL_Table *) expr->value.uval->data;
		return vm_MATCH_table_elements(vm, len, table);
	}
	case P_LS: {
		size_t len = (size_t)vm_read_int(vm);
		if (!obj_islist(expr)) {
			vm_ff_subpatterns_multiple(vm, len);
			return false;
		}

		struct YASL_List *ls = (struct YASL_List *)expr->value.uval->data;
		if (ls->count != (size_t)len) {
			vm_ff_subpatterns_multiple(vm, len);
			return false;
		}

		return MATCH_list(vm, ls, len);
	}
	case P_VLS: {
		size_t len = (size_t)vm_read_int(vm);
		if (!obj_islist(expr)) {
			vm_ff_subpatterns_multiple(vm, len);
			return false;
		}

		struct YASL_List *ls = (struct YASL_List *)expr->value.uval->data;
		if (ls->count < (size_t)len) {
			vm_ff_subpatterns_multiple(vm, len);
			return false;
		}

		return MATCH_list(vm, ls, len);
	}
	case P_BIND: {
		/* We offset by +1 here to account for the fact that the expression we are matching on is still on top
		 * of the stack. This is taken care of _after_ a successful match by a O_DEL_FP instruction.
		 */
		unsigned char offset = NCODE(vm) + 1;
		vm_dec_ref(vm, &vm_peek_fp(vm, offset));
		vm_peek_fp(vm, offset) = *expr;
		inc_ref(&vm_peek_fp(vm, offset));
		return true;
	}
	case P_ONE:
		return true;
	case P_ANY:
		return true;
	case P_NOT:
		return !vm_MATCH_subpattern(vm, expr);
	case P_ALT: {
		if (vm_MATCH_subpattern(vm, expr)) {
			vm_ff_subpattern(vm);
			return true;
		}
		return vm_MATCH_subpattern(vm, expr);
	}
	}

	return false;
}

static bool vm_MATCH_pattern(struct VM *const vm, struct YASL_List *exprs) {
	unsigned char count = NCODE(vm);
	bool tmp = true;
	for (unsigned i = 0; i < exprs->count && i < count; i++) {
		struct YASL_Object *expr = &exprs->items[i];
		if (*vm->pc == P_ANY) {
			YASL_UNUSED(NCODE(vm));
			return tmp;
		}
		tmp = tmp && vm_MATCH_subpattern(vm, expr);
	}
	if (*vm->pc == P_ANY) {
		YASL_UNUSED(NCODE(vm));
		return tmp;
	}
	return tmp && exprs->count == count;
}

static void vm_MATCH_IF(struct VM *const vm) {
	struct YASL_List *exprs = vm_peeklist(vm);
	yasl_int addr = vm_read_int(vm);
	unsigned char *start = vm->pc;
	if (!vm_MATCH_pattern(vm, exprs)) {
		vm->pc = start + addr;
	}
}

static void vm_GSTORE_8(struct VM *const vm) {
	yasl_int addr = vm_read_int(vm);

	YASL_Table_insert_fast(vm->globals, vm->constants[addr], vm_pop(vm));
}

static void vm_GLOAD_8(struct VM *const vm) {
	yasl_int addr = vm_read_int(vm);

	vm_push(vm, YASL_Table_search(vm->globals, vm->constants[addr]));

	YASL_ASSERT(vm_peek(vm).type != Y_END, "global not found");
}

static void vm_enterframe_offset(struct VM *const vm, int offset, int num_returns) {
	if (++vm->frame_num >= NUM_FRAMES) {
		vm->frame_num--;
		vm_print_err(vm, "StackOverflow.");
		vm_throw_err(vm, YASL_STACK_OVERFLOW_ERROR);
	}

	int next_fp = vm->next_fp;
	vm->next_fp = offset;
	vm->frames[vm->frame_num] = ((struct CallFrame) { vm->pc, vm->fp, next_fp, vm->loopframe_num, num_returns });
}

static void vm_fill_args(struct VM *const vm, const int num_args);

static void vm_exitloopframe(struct VM *const vm) {
	vm_dec_ref(vm, &vm->loopframes[vm->loopframe_num].iterable);
	vm_dec_ref(vm, &vm->loopframes[vm->loopframe_num].curr);
	vm_dec_ref(vm, &vm->loopframes[vm->loopframe_num].next_fn);
	vm->loopframe_num--;
}

void vm_exitframe_multi(struct VM *const vm, int len) {
	vm_rm_range(vm, vm->fp, vm->fp + len + 1);

	struct CallFrame frame = vm->frames[vm->frame_num];
	int num_returns = frame.num_returns;

	if (num_returns >= 0) {
		if (vm->sp + 1 - vm->fp < num_returns) {
			while (vm->sp + 1 - vm->fp < num_returns) {
				vm_pushundef(vm);
			}
		} else if (vm->sp + 1 - vm->fp > num_returns) {
			while (vm->sp + 1 - vm->fp > num_returns) {
				vm_pop(vm);
			}
		}
	}

	vm->pc = frame.pc;
	vm->fp = frame.prev_fp;
	vm->next_fp = frame.curr_fp;
	while (vm->loopframe_num > frame.lp) {
		vm_exitloopframe(vm);
	}

	vm->frame_num--;
}

static void vm_exitframe(struct VM *const vm) {
	vm_rm_range(vm, vm->fp, vm->sp);

	struct CallFrame frame = vm->frames[vm->frame_num];
	vm->pc = frame.pc;
	vm->fp = frame.prev_fp;
	vm->next_fp = frame.curr_fp;
	while (vm->loopframe_num > frame.lp) {
		vm_exitloopframe(vm);
	}

	vm->frame_num--;
}

void vm_INIT_CALL_offset(struct VM *const vm, int offset, int expected_returns) {
	if (!vm_isfn(vm, offset) && !vm_iscfn(vm, offset) && !vm_isclosure(vm, offset)) {
		const char *name = vm_peektypename(vm, offset);
		vm_lookup_method_throwing_source(vm, offset, "__call", "%s is not callable.", name);
		vm_rm(vm, offset);
		vm_shifttopdown(vm, vm->sp - offset);
	}

	vm_enterframe_offset(vm, offset, expected_returns);
}

static void vm_INIT_CALL(struct VM *const vm, int expected_returns) {
	vm_INIT_CALL_offset(vm, vm->sp, expected_returns);
}

/*
static void vm_dup(struct VM *const vm, int source) {
	vm_push(vm, vm_peek(vm, source));
}
*/

static void vm_duptop(struct VM *const vm) {
	vm_push(vm, vm_peek(vm));
}

static void vm_swaptop(struct VM *const vm) {
	struct YASL_Object tmp = vm_peek(vm);
	vm_peek(vm) = vm_peek(vm, vm->sp - 1);
	vm_peek(vm, vm->sp - 1) = tmp;
}

static void vm_INIT_MC(struct VM *const vm) {
	int expected_returns = (signed char)NCODE(vm);
	yasl_int addr = vm_read_int(vm);
	struct RC_UserData* table = obj_get_metatable(vm, vm_peek(vm));
	int result = vm_lookup_method_helper(vm, (struct YASL_Table *)(table ? table->data : NULL), vm->constants[addr]);
	if (result) {
		const size_t len = YASL_String_len(vm->constants[addr].value.sval);
		const char *chars = YASL_String_chars(vm->constants[addr].value.sval);
		vm_print_err_value(vm, "No method named `%.*s` for object of type %s.", (int)len, chars, obj_typename(vm_peek_p(vm)));
		vm_throw_err(vm, YASL_VALUE_ERROR);
	}
	vm_swaptop(vm);
#if YASL_REGISTER_MIGRATION == 1
	YASL_UNUSED(expected_returns);
#else
	vm_INIT_CALL_offset(vm, vm->sp - 1, expected_returns);
#endif
}

static void vm_fill_args(struct VM *const vm, const int num_args) {
	if (vm->sp - vm->fp < num_args) {
		while (vm->sp - vm->fp < num_args) {
			vm_pushundef(vm);
		}
	} else if (vm->sp - vm->fp > num_args) {
		while (vm->sp - vm->fp > num_args) {
			vm_pop(vm);
		}
	}
}

static inline void vm_CALL_native(struct VM *const vm, unsigned char *const code) {
	vm->frames[vm->frame_num].pc = vm->pc;

	int num_args = *(signed char *)code;
	if (num_args < 0) {
		int var_num_args = ~num_args;
		while (vm->sp - vm->fp < var_num_args) {
			vm_pushundef(vm);
		}
	} else {
		vm_fill_args(vm, num_args);
	}

	vm->pc =  code + 1;
}

static void vm_CALL_closure(struct VM *const vm) {
	vm_CALL_native(vm, vm_peek(vm, vm->fp).value.lval->f);
}

static void vm_CALL_fn(struct VM *const vm) {
	vm_CALL_native(vm, vm_peek(vm, vm->fp).value.fval);
}

static void vm_CALL_cfn(struct VM *const vm) {
	vm->frames[vm->frame_num].pc = vm->pc;
	struct CFunction *f = vm_peekcfn(vm, vm->fp);
	if (f->num_args < 0) {
		yasl_int diff = vm->sp - vm->fp - ~f->num_args;
		YASL_VM_DEBUG_LOG("diff: %d\n", (int)diff);
		vm_insert(vm, vm->fp + 1 + ~f->num_args, YASL_INT(diff));
	} else {
		vm_fill_args(vm, f->num_args);
	}

	int num_returns = f->value((struct YASL_State *) vm);

	vm_exitframe_multi(vm, vm->sp - num_returns - vm->fp);
}

void vm_COLLECT_REST(struct VM *vm, unsigned offset) {
	struct RC_UserData *ls = rcls_new(vm);

	for (int i = vm->fp + offset + 1; i <= vm->sp; i++) {
		YASL_List_push((struct YASL_List *) ls->data, vm_peek(vm, i));
	}

	vm->sp = vm->fp + offset;
	vm_pushlist(vm, ls);
}

void vm_COLLECT_REST_PARAMS(struct VM *const vm) {
	int offset = 0;
	if (vm_isfn(vm, vm->fp)) {
		offset = ~*(signed char *) vm_peek(vm, vm->fp).value.fval;
	} else if (vm_isclosure(vm, vm->fp)) {
		offset = ~*(signed char *) vm_peek(vm, vm->fp).value.lval->f;
	} else {
		YASL_UNREACHED();
	}

	vm_COLLECT_REST(vm, (unsigned)offset);
}

void vm_SPREAD_VARGS(struct VM *const vm) {
	const int top = vm->sp;
	YASL_ASSERT(vm_islist(vm), "top should be a list.");
	struct YASL_List *ls = vm_peeklist(vm);

	FOR_LIST(i, elmt, ls) {
		vm_push(vm, elmt);
	}

	vm_rm(vm, top);
}

void vm_CALL_offset(struct VM *const vm, int offset, int expected_returns) {
	vm_INIT_CALL_offset(vm, vm->fp + offset + 1, expected_returns);
	vm->fp = vm->next_fp;
	if (vm_isfn(vm, vm->fp)) {
		vm_CALL_fn(vm);
	} else if (vm_iscfn(vm, vm->fp)) {
		vm_CALL_cfn(vm);
	} else if (vm_isclosure(vm, vm->fp)) {
		vm_CALL_closure(vm);
	}
}

void vm_CALL(struct VM *const vm) {
	vm->fp = vm->next_fp;
	if (vm_isfn(vm, vm->fp)) {
		vm_CALL_fn(vm);
	} else if (vm_iscfn(vm, vm->fp)) {
		vm_CALL_cfn(vm);
	} else if (vm_isclosure(vm, vm->fp)) {
		vm_CALL_closure(vm);
	}
}

void vm_CALL_now(struct VM *const vm) {
	int fp = vm->fp;
	vm_CALL(vm);
	while (fp < vm->fp) {
		vm_executenext(vm);
	}
}

static void vm_RET(struct VM *const vm) {
	unsigned char len = NCODE(vm);
	vm_exitframe_multi(vm, len);
}

static struct Upvalue *vm_close_all_helper(struct YASL_Object *const end, struct Upvalue *const curr) {
	if (curr == NULL) return NULL;
	if (curr->location < end) return curr;
	inc_ref(curr->location);
	upval_close(curr);
	return (vm_close_all_helper(end, curr->next));
}

void vm_close_all(struct VM *const vm) {
	vm->pending = vm_close_all_helper(vm->stack + vm->fp, vm->pending);
}

static void vm_STRINGIFY(struct VM *const vm) {
	unsigned char bottom = NCODE(vm);
	const char *start = (const char*)vm->pc;
	while (*(vm->pc++)) ;
	struct YASL_Object fmt = strlen(start) ? YASL_STR(YASL_String_new_copyz(vm, start)) : vm_getformat(vm);
	int size = vm->sp - bottom - vm->fp;
	struct YASL_String **tmps = (struct YASL_String **)malloc(sizeof(struct YASL_String *) * size);
	int i = 0;
	while (vm->sp > vm->fp + bottom) {
		vm_stringify_top_format(vm, fmt.type == Y_UNDEF ? NULL : &fmt);
		vm_peekstr(vm)->rc.refs++;
		tmps[i++] = vm_popstr(vm);
	}
	for (int j = size - 1; j >= 0; j--) {
		vm_pushstr(vm, tmps[j]);
		vm_peekstr(vm)->rc.refs--;
	}
	free(tmps);
}

static void vm_ECHO(struct VM *const vm) {
	unsigned char top = NCODE(vm);
	if (vm->fp + top == vm->sp) {
		vm_print_out(vm, "\n");
		return;
	}
	size_t tmp = 0;
	for (int i = vm->fp + 1 + top; i <= vm->sp; i++) {
		tmp += YASL_String_len(vm_peekstr(vm, i)) + 2;
	}
	char *dest = (char *)malloc(tmp);
	tmp = 0;
	char *curr = dest;
	for (int i = vm->fp + 1 + top; i <= vm->sp; i++) {
		size_t strlen = YASL_String_len(vm_peekstr(vm, i));
		size_t copied = io_str_strip_char(curr, YASL_String_chars(vm_peekstr(vm, i)), strlen, 0);
		curr[strlen] = ',';
		curr[strlen+1] = ' ';
		curr += copied + 2;
		tmp += copied + 2;
	}
	vm_print_out(vm, "%.*s\n", (int)tmp-2, dest);
	free(dest);
	vm->sp = vm->fp + top;
}

void vm_setupconstants(struct VM *const vm) {
	vm->num_constants = ((int64_t *)vm->code)[2];
	vm->constants = (struct YASL_Object *)malloc(sizeof(struct YASL_Object) * vm->num_constants);
	unsigned char *tmp = vm->code + 3*sizeof(int64_t);
	for (int64_t i = 0; i < vm->num_constants; i++) {
		switch (*tmp++) {
		case C_STR: {
			int64_t len = *((int64_t *) tmp);
			tmp += sizeof(int64_t);
			char *str = (char *) malloc((size_t) len);
			memcpy(str, tmp, (size_t) len);
			vm->constants[i] = YASL_STR(YASL_String_new_take(vm, str, (size_t) len));
			inc_ref(vm->constants + i);
			tmp += len;
			break;
		}
		case C_INT_1: {
			vm->constants[i] = YASL_INT((signed char)*tmp++);
			break;
		}
		case C_INT_8: {
			int64_t v = *((int64_t *) tmp);
			vm->constants[i] = YASL_INT(v);
			tmp += sizeof(int64_t);
			break;
		}
		case C_FLOAT: {
			yasl_float v = *((yasl_float *) tmp);
			vm->constants[i] = YASL_FLOAT(v);
			tmp += sizeof(yasl_float);
			break;
		}
		default:
			break;
		}
	}
}

struct YASL_String *vm_lookup_interned_str(struct VM *vm, const char *chars, const size_t size) {
	struct YASL_String *string = YASL_StringSet_maybe_insert(vm->interned_strings, chars, size);
	return string;
}

struct YASL_String *vm_lookup_interned_zstr(struct VM *vm, const char *chars) {
	return vm_lookup_interned_str(vm, chars, strlen(chars));
}

static int get_source(struct VM *const vm) {
#if YASL_REGISTER_MIGRATION == 1
	const int source = NCODE(vm);
#else
	const int source = vm->sp - vm->fp - 1;
#endif
	return source;
}

void vm_executenext(struct VM *const vm) {
	unsigned char opcode = NCODE(vm);        // fetch
	signed char offset;
	struct YASL_Object a, b;
	yasl_int c;
	YASL_VM_DEBUG_LOG("----------------\n"
		  	  "line: %" PRI_SIZET "\n"
			  "opcode: %x\n"
			  "vm->sp, vm->prev_fp, vm->curr_fp: %d, %d, %d\n\n", vm_getcurrline(vm), opcode, vm->sp, vm->fp, vm->next_fp);
	switch (opcode) {
	case O_EXPORT:
		vm_close_all(vm);
		vm_throw_err(vm, YASL_MODULE_SUCCESS);
	case O_HALT:
		vm_throw_err(vm, YASL_SUCCESS);
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
		vm_int_binop(vm, &bor, "|", OP_BIN_BAR);
		break;
	case O_BXOR:
		vm_int_binop(vm, &bxor, "^", OP_BIN_CARET);
		break;
	case O_BAND:
		vm_int_binop(vm, &band, "&", OP_BIN_AMP);
		break;
	case O_BANDNOT:
		vm_int_binop(vm, &bandnot, "&^", OP_BIN_AMPCARET);
		break;
	case O_BNOT: {
		const int source = get_source(vm);
		vm_int_unop(vm, source, source, &bnot, "^", OP_UN_CARET);
		break;
	}
	case O_BSL:
		vm_int_binop(vm, &shift_left, "<<", OP_BIN_SHL);
		break;
	case O_BSR:
		vm_int_binop(vm, &shift_right, ">>", OP_BIN_SHR);
		break;
	case O_ADD:
		vm_num_binop(vm, &int_add, &float_add, "+", OP_BIN_PLUS);
		break;
	case O_MUL:
		vm_num_binop(vm, &int_mul, &float_mul, "*", OP_BIN_TIMES);
		break;
	case O_SUB:
		vm_num_binop(vm, &int_sub, &float_sub, "-", OP_BIN_MINUS);
		break;
	case O_FDIV:
		vm_fdiv(vm);   // handled differently because we always convert to float
		break;
	case O_IDIV:
		if (vm_isint(vm) && vm_peekint(vm) == 0) {
			vm_print_err_divide_by_zero(vm);
			vm_throw_err(vm, YASL_DIVIDE_BY_ZERO_ERROR);
		}
		vm_int_binop(vm, &idiv, "//", OP_BIN_IDIV);
		break;
	case O_MOD:
		// TODO: handle undefined C behaviour for negative numbers.
		if (vm_isint(vm) && vm_peekint(vm) == 0) {
			vm_print_err_divide_by_zero(vm);
			vm_throw_err(vm, YASL_DIVIDE_BY_ZERO_ERROR);
		}
		vm_int_binop(vm, &modulo, "%", OP_BIN_MOD);
		break;
	case O_EXP:
		vm_pow(vm);
		break;
	case O_NEG: {
		const int source = get_source(vm);
		vm_num_unop(vm, source, source, &int_neg, &float_neg, "-", OP_UN_MINUS);
		break;
	}
	case O_POS: {
		const int source = get_source(vm);
		vm_num_unop(vm, source, source, &int_pos, &float_pos, "+", OP_UN_PLUS);
		break;
	}
	case O_NOT:
#if YASL_REGISTER_MIGRATION == 1
		(void)NCODE(vm);
#endif
		vm_pushbool(vm, isfalsey(vm_pop_p(vm)));
		break;
	case O_LEN: {
		const int source = get_source(vm);
		vm_len_unop(vm, source, source);
		break;
	}
	case O_CNCT:
		vm_CNCT(vm);
		break;
	case O_GT:
		vm_GT(vm);
		break;
	case O_GE:
		vm_GE(vm);
		break;
	case O_LT:
		vm_LT(vm);
		break;
	case O_LE:
		vm_LE(vm);
		break;
	case O_EQ:
		vm_EQ(vm);
		break;
	case O_ID:     // TODO: clean-up
		b = vm_pop(vm);
		a = vm_pop(vm);
		vm_pushbool(vm, a.type == b.type && obj_getint(&a) == obj_getint(&b));
		break;
	case O_LIT:
		vm_LIT(vm);
		break;
	case O_LIT8:
		vm_LIT8(vm);
		break;
	case O_NEWTABLE: {
		struct RC_UserData *table = rcht_new(vm);
		struct YASL_Table *ht = (struct YASL_Table *)table->data;
		while (vm_peek(vm).type != Y_END) {
			struct YASL_Object val = vm_pop(vm);
			struct YASL_Object key = vm_pop(vm);
			if (obj_isundef(&val)) {
				continue;
			}
			if (!YASL_Table_insert(ht, key, val)) {
				rcht_del(table);
				struct YASL_Object mt = YASL_TABLE(vm->builtins_htable[Y_TABLE]);
				vm_dec_ref(vm, &mt);
				vm_print_err_type(vm, "unable to use mutable object of type %s as key.", obj_typename(&key));
				vm_throw_err(vm, YASL_TYPE_ERROR);
			}
		}

		vm_pop(vm);
		vm_push(vm, YASL_TABLE(table));
		break;
	}
	case O_NEWLIST: {
		struct RC_UserData *ls = rcls_new(vm);
		int len = 0;
		while (vm_peek(vm, vm->sp - len).type != Y_END) {
			len++;
		}
		for (int i = 0; i < len; i++) {
			YASL_List_push((struct YASL_List *) ls->data, vm_peek(vm, vm->sp - len + i + 1));
		}
		vm->sp -= len + 1;
		vm_pushlist(vm, ls);
		break;
	}
	case O_LIST_PUSH:{
		struct YASL_Object v = vm_pop(vm);
		struct YASL_List *ls = vm_peeklist(vm);
		YASL_List_push(ls, v);
		break;
	}
	case O_TABLE_SET: {
		struct YASL_Object v = vm_pop(vm);
		struct YASL_Object k = vm_pop(vm);
		struct YASL_Table *ht = vm_peektable(vm);
		YASL_Table_insert(ht, k, v);
		break;
	}
	case O_INITFOR: {
		inc_ref(vm_peek_p(vm));
		vm->loopframe_num++;
		struct YASL_Object *obj = vm_peek_p(vm);
		vm_push(vm, *obj);
		vm_push(vm, *obj);
		vm_lookup_method_throwing(
			vm, "__iter", "object of type %s is not iterable.",
			obj_typename(obj));
		vm_shifttopdown(vm, 1);
		vm_INIT_CALL_offset(vm, vm->sp - 1, 2);
		vm_CALL_now(vm);
		inc_ref(vm_peek_p(vm));
		vm->loopframes[vm->loopframe_num].curr = vm_pop(vm);
		inc_ref(vm_peek_p(vm));
		vm->loopframes[vm->loopframe_num].next_fn = vm_pop(vm);
		vm->loopframes[vm->loopframe_num].iterable = vm_pop(vm);
		break;
	}
	case O_ENDFOR:
		vm_exitloopframe(vm);
		break;
	case O_ENDCOMP:
		c = NCODE(vm);
		vm_rm(vm, vm->fp + 1 + c);
		vm_exitloopframe(vm);
		break;
	case O_ITER_1:
		vm_ITER_1(vm);
		break;
	case O_END:
		vm_pushend(vm);
		break;
	case O_SWAP:
		a = vm_peek(vm);
		vm_peek(vm) = vm_peek(vm, vm->sp - 1);
		vm_peek(vm, vm->sp - 1) = a;
		break;
	case O_DUP:
		a = vm_peek(vm);
		vm_push(vm, a);
		break;
	case O_MOVEUP_FP:
		offset = NCODE(vm);
		a = vm_peek_fp(vm, offset);
		memmove(vm->stack + vm->fp + offset + 1, vm->stack + vm->fp + offset + 2, (vm->sp - (vm->fp + offset + 1)) * sizeof(struct YASL_Object));
		vm->stack[vm->sp] = a;
		break;
	case O_MOVEDOWN_FP:
		offset = NCODE(vm);
		a = vm_peek(vm);
		memmove(vm->stack + vm->fp + offset + 2, vm->stack + vm->fp + offset + 1, (vm->sp - (vm->fp + offset + 1)) * sizeof(struct YASL_Object));\
		vm->stack[vm->fp + offset + 1] = a;
		break;
	case O_STRINGIFY:
		vm_STRINGIFY(vm);
		break;
	case O_MATCH:
		vm_MATCH_IF(vm);
		break;
	case O_BR_8:
		vm->pc += vm_read_int(vm);
		break;
	case O_BRF_8:
		c = vm_read_int(vm);
		if (isfalsey(vm_pop_p(vm))) vm->pc += c;
		break;
	case O_BRT_8:
		c = vm_read_int(vm);
		if (!isfalsey(vm_pop_p(vm))) vm->pc += c;
		break;
	case O_BRN_8:
		c = vm_read_int(vm);
		if (!obj_isundef(vm_pop_p(vm))) vm->pc += c;
		break;
	case O_GLOAD_8:
		vm_GLOAD_8(vm);
		break;
	case O_GSTORE_8:
		vm_GSTORE_8(vm);
		break;
	case O_LLOAD:
		offset = NCODE(vm);
		vm_push(vm, vm_peek_fp(vm, offset));
		break;
	case O_LSTORE:
		offset = NCODE(vm);
		vm_dec_ref(vm, &vm_peek_fp(vm, offset));
		vm_peek_fp(vm, offset) = vm_pop(vm);
		inc_ref(&vm_peek_fp(vm, offset));
		break;
	case O_ULOAD:
		offset = NCODE(vm);
		vm_push(vm, upval_get(vm_peek(vm, vm->fp).value.lval->upvalues[offset]));
		break;
	case O_USTORE:
		offset = NCODE(vm);
		upval_set(vm, vm_peek(vm, vm->fp).value.lval->upvalues[offset], vm_pop(vm));
		break;
	case O_INIT_MC:
		vm_INIT_MC(vm);
		break;
	case O_INIT_CALL:
		vm_INIT_CALL(vm, (signed char)NCODE(vm));
		break;
	case O_CALL:
#if YASL_REGISTER_MIGRATION == 1
	{
		char offset = NCODE(vm);
		char expected_returns = NCODE(vm);
		vm_INIT_CALL_offset(vm, vm->fp + offset + 1, expected_returns);
		vm_CALL(vm);
	}
#else
		vm_CALL(vm);
#endif
		break;
	case O_COLLECT_REST:
		offset = NCODE(vm);
		vm_COLLECT_REST(vm, offset);
		break;
	case O_COLLECT_REST_PARAMS:
		vm_COLLECT_REST_PARAMS(vm);
		break;
	case O_SPREAD_VARGS:
		vm_SPREAD_VARGS(vm);
		break;
	case O_CRET:
		vm_close_all(vm);
		vm_RET(vm);
		break;
	case O_RET:
		vm_RET(vm);
		break;
	case O_GET:
		vm_GET(vm);
		break;
	case O_SLICE:
		vm_SLICE(vm);
		break;
	case O_SET:
		vm_SET(vm);
		break;
	case O_POP:
		vm_pop(vm);
		break;
	case O_DEL_FP:
		c = NCODE(vm);
		vm_rm(vm, vm->fp + 1 + c);
		break;
	case O_DECSP:
		vm->sp -= NCODE(vm);
		vm_close_all_helper(vm->stack + vm->sp, vm->pending);
		break;
	case O_INCSP:
		vm->sp += NCODE(vm);
		break;
	case O_ECHO:
		vm_ECHO(vm);
		break;
	case O_ASS:
		if (isfalsey(vm_peek_p(vm))) {
			vm_stringify_top(vm);
			vm_print_err(vm, "AssertError: %.*s.", (int)YASL_String_len(vm_peekstr(vm)), YASL_String_chars(vm_peekstr(vm)));
			vm_pop(vm);
			vm_throw_err(vm, YASL_ASSERT_ERROR);
		}
		vm_pop(vm);
		break;
#ifdef YASL_DEBUG
	case O_ASSERT_STACK_HEIGHT:
		c = NCODE(vm);
		if (c != vm->sp - vm->fp) {
			fprintf(stderr, "vm->sp, vm->fp: %d, %d\n", vm->sp, vm->fp);
			fprintf(stdout, "Wrong stack height in line %zd, expected %d, got %d\n", vm_getcurrline(vm), (int)c, (vm->sp - vm->fp));
			vm_throw_err(vm, YASL_ERROR);
		}
		break;
#endif  // YASL_DEBUG
	default:
		vm_print_err(vm, "Error: Unknown Opcode: %x\n", opcode);
		vm_throw_err(vm, YASL_ERROR);
	}
}

void vm_init_buf(struct VM *vm) {
	YASL_ASSERT(vm->buf == NULL, "no longjmp buffer");
	vm->buf = (jmp_buf*)malloc(sizeof(jmp_buf));
}

void vm_deinit_buf(struct VM *vm) {
	free(vm->buf);
	vm->buf = NULL;
}

#define VM_FAILED(vm) ((vm)->status != YASL_SUCCESS && (vm)->status != YASL_MODULE_SUCCESS)

int vm_run(struct VM *const vm) {
	vm_init_buf(vm);
	if (setjmp(*vm->buf)) {
		vm_deinit_buf(vm);
		if (VM_FAILED(vm))
			printline(vm);
		return vm->status;
	}

	vm_setupconstants(vm);

	while (true) {
		vm_executenext(vm);
	}
}
