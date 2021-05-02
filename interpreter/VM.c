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

static struct RC_UserData **builtins_htable_new(struct VM *const vm) {
	struct RC_UserData **ht = (struct RC_UserData **) malloc(sizeof(struct RC_UserData *) * NUM_TYPES);
	ht[Y_UNDEF] = ud_new(undef_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_UNDEF]->rc->refs++;
	ht[Y_FLOAT] = ud_new(float_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_FLOAT]->rc->refs++;
	ht[Y_INT] = ud_new(int_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_INT]->rc->refs++;
	ht[Y_BOOL] = ud_new(bool_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_BOOL]->rc->refs++;
	ht[Y_STR] = ud_new(str_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_STR]->rc->refs++;
	ht[Y_LIST] = ud_new(list_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_LIST]->rc->refs++;
	ht[Y_TABLE] = ud_new(table_builtins(vm), TABLE_NAME, NULL, rcht_del_data);
	ht[Y_TABLE]->rc->refs++;
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

#define X(E, S, ...) vm->special_strings[E] = YASL_String_new_sized(strlen(S), S);
#include "specialstrings.x"
#undef X

	vm->builtins_htable = builtins_htable_new(vm);
	vm->pending = NULL;
}

void vm_close_all(struct VM *const vm);

void vm_cleanup(struct VM *const vm) {
	// If we've exited early somehow, without closing over some upvalues, we need to do that first.
	vm_close_all(vm);

	// Exit out of all loops (in case we're exiting with an error).
	while (vm->loopframe_num >= 0) {
		dec_ref(&vm->loopframes[vm->loopframe_num].iterable);
		vm->loopframe_num--;
	}

	for (size_t i = 0; i < STACK_SIZE; i++) {
 		dec_ref(vm->stack + i);
	}
	free(vm->stack);

	for (int64_t i = 0; i < vm->num_constants; i++) {
		dec_ref(vm->constants + i);
	}
	free(vm->constants);

	for (size_t i = 0; i < vm->headers_size; i++) {
		free(vm->headers[i]);
	}
	free(vm->headers);

	YASL_Table_del(vm->globals);

	YASL_Table_del(vm->metatables);

	struct YASL_Object v;
	v = YASL_TABLE(vm->builtins_htable[Y_UNDEF]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_FLOAT]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_INT]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_BOOL]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_STR]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_LIST]);
	dec_ref(&v);
	v = YASL_TABLE(vm->builtins_htable[Y_TABLE]);
	dec_ref(&v);
	free(vm->builtins_htable);

	io_cleanup(&vm->out);
	io_cleanup(&vm->err);
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

void vm_throw_err(struct VM *const vm, int error) {
	vm->status = error;
	longjmp(vm->buf, 1);
}

void vm_push(struct VM *const vm, const struct YASL_Object val) {
	if (vm->sp + 1 >= STACK_SIZE) {
		vm_print_err(vm, "StackOverflow.");
		vm_throw_err(vm, YASL_STACK_OVERFLOW_ERROR);
	}

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

	dec_ref(vm->stack + vm->sp + 1);
	memmove(vm->stack + index + 1, vm->stack + index, (vm->sp - index + 1) * sizeof(struct YASL_Object));
	vm->stack[index] = val;
	vm->sp++;
}

void vm_rm(struct VM *const vm, int index) {
	int after = vm->sp - index;
	dec_ref(vm->stack + index);
	memmove(vm->stack + index, vm->stack + index + 1, after * sizeof(struct YASL_Object));
	vm->stack[vm->sp] = YASL_END();
	vm->sp--;
}

void vm_rm_range(struct VM *const vm, int start, int end) {
	int after = vm->sp - end + 1;
	int len = end - start;
	for (int i = 0; i < after && i < len; i++) {
		dec_ref(vm->stack + start + i);
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

static void vm_duptop(struct VM *const vm);
static void vm_swaptop(struct VM *const vm);
static int vm_lookup_method_helper(struct VM *vm, struct YASL_Object obj, struct YASL_Table *mt, struct YASL_Object index);
static void vm_GET(struct VM *const vm);
static void vm_INIT_CALL(struct VM *const vm, int expected_returns);
void vm_INIT_CALL_offset(struct VM *const vm, int offset, int expected_returns);
void vm_CALL(struct VM *const vm);

static void vm_call_now_2(struct VM *vm, struct YASL_Object a, struct YASL_Object b) {
	vm_INIT_CALL(vm, 1);
	vm_push(vm, a);
	vm_push(vm, b);
	vm_CALL(vm);
}

#define vm_lookup_method_throwing(vm, method_name, err_str, ...) \
do {\
	struct YASL_Object index = YASL_STR(YASL_String_new_sized(strlen(method_name), method_name));\
	struct YASL_Object val = vm_peek(vm);\
	vm_get_metatable(vm);\
	struct YASL_Table *mt = vm_istable(vm) ? vm_poptable(vm) : NULL;\
	if (!mt) {\
		vm_pop(vm);\
	}\
	int result = vm_lookup_method_helper(vm, val, mt, index);\
	str_del(obj_getstr(&index));\
	if (result) {\
		vm_print_err_type(vm, err_str, __VA_ARGS__);\
		vm_throw_err(vm, YASL_TYPE_ERROR);\
	}\
} while (0)

#define vm_call_method_now_1_top(vm, method_name, ...) do {\
	vm_duptop(vm);\
	vm_lookup_method_throwing(vm, method_name, __VA_ARGS__, obj_typename(vm_peek_p(vm)));\
	vm_swaptop(vm);\
	vm_INIT_CALL_offset(vm, vm->sp - 1, 1);\
	vm_CALL(vm);\
} while (0)

#define vm_call_method_now_2(vm, left, right, method_name, ...) do {\
	inc_ref(&left);\
	inc_ref(&right);\
	vm_push(vm, left);\
	vm_lookup_method_throwing(vm, method_name, __VA_ARGS__);\
	vm_call_now_2(vm, left, right);\
	dec_ref(&left);\
	dec_ref(&right);\
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
	dec_ref(&top);
}

static void vm_int_binop(struct VM *const vm, int_binop op, const char *opstr, const char *overload_name) {
	struct YASL_Object right = vm_peek(vm);
	struct YASL_Object left = vm_peek(vm, vm->sp - 1);
	if (obj_isint(&left) && obj_isint(&right)) {
		vm_pop(vm);
		vm_pop(vm);
		vm_push(vm, YASL_INT(op(obj_getint(&left), obj_getint(&right))));
	} else {
		vm_push(vm, left);
		vm_lookup_method_throwing(vm, overload_name, "%s not supported for operands of types %s and %s.",
					  opstr,
					  obj_typename(&left),
					  obj_typename(&right));
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
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
		vm_push(vm, left);
		vm_lookup_method_throwing(vm, overload_name, "%s not supported for operands of types %s and %s.",
					  opstr,
					  obj_typename(&left),
					  obj_typename(&right));
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
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
		vm_push(vm, left);
		vm_lookup_method_throwing(vm, overload_name, "/ not supported for operands of types %s and %s.",
					  obj_typename(&left), obj_typename(&right));
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
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

static void vm_int_unop(struct VM *const vm, yasl_int (*op)(yasl_int), const char *opstr, const char *overload_name) {
	if (vm_isint(vm)) {
		vm_pushint(vm, op(vm_popint(vm)));
		return;
	} else {
		vm_call_method_now_1_top(vm, overload_name, "%s not supported for operand of type %s.", opstr);
	}
}

static void vm_num_unop(struct VM *const vm, yasl_int (*int_op)(yasl_int), yasl_float (*float_op)(yasl_float), const char *opstr, const char *overload_name) {
	if (vm_isint(vm)) {
		vm_pushint(vm, int_op(vm_popint(vm)));
	} else if (vm_isfloat(vm)) {
		vm_pushfloat(vm, float_op(vm_popfloat(vm)));
	} else {
		vm_call_method_now_1_top(vm, overload_name, "%s not supported for operand of type %s.", opstr);
	}
}

void vm_len_unop(struct VM *const vm) {
	if (vm_isstr(vm)) {
		vm_pushint(vm, (yasl_int) YASL_String_len(vm_popstr(vm)));
	} else {
		vm_call_method_now_1_top(vm, "__len", "len not supported for operand of type %s.");
	}
}

void vm_EQ(struct VM *const vm) {
	struct YASL_Object b = vm_peek(vm);
	struct YASL_Object a = vm_peek(vm, vm->sp - 1);
	if (obj_isuserdata(&a) && obj_isuserdata(&b) ||
	    obj_istable(&a) && obj_istable(&b) ||
	    obj_islist(&a) && obj_islist(&b)) {
		vm_push(vm, a);
		vm_lookup_method_throwing(vm, "__eq", "== not supported for operands of types %s and %s.",
					  obj_typename(&a), obj_typename(&b));
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
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
		vm_pushstr(vm, YASL_String_new_sized_heap(0, size, ptr));
		dec_ref(&top);
}

#define DEFINE_COMP(name, opstr, overload_name) \
static void vm_##name(struct VM *const vm) {\
	struct YASL_Object right = vm_pop(vm);\
	struct YASL_Object left = vm_pop(vm);\
	bool c;\
	if (obj_isstr(&left) && obj_isstr(&right)) {\
		vm_pushbool(vm, name(YASL_String_cmp(obj_getstr(&left), obj_getstr(&right)), 0));\
		return;\
	}\
	if (obj_isnum(&left) && obj_isnum(&right)) {\
		COMP(vm, left, right, name);\
		return;\
	}\
	vm_call_method_now_2(vm, left, right, overload_name, "%s not supported for operands of types %s and %s.",\
		opstr,\
		obj_typename(&left),\
		obj_typename(&right));\
}

DEFINE_COMP(GT, ">", "__gt")
DEFINE_COMP(GE, ">=", "__ge")
DEFINE_COMP(LT, "<", "__lt")
DEFINE_COMP(LE, "<=", "__le")

void vm_stringify_top(struct VM *const vm) {
	if (vm_isfn(vm) || vm_iscfn(vm) || vm_isclosure(vm)) {
		size_t n = (size_t)snprintf(NULL, 0, "<fn: %p>", vm_peekuserptr(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<fn: %d>", (int)vm_popint(vm));
		vm_pushstr(vm, YASL_String_new_sized_heap(0, strlen(buffer), buffer));
	} else if (vm_isuserptr(vm)) {
		size_t n = (size_t)snprintf(NULL, 0, "<userptr: %p>", vm_peekuserptr(vm)) + 1;
		char *buffer = (char *)malloc(n);
		snprintf(buffer, n, "<userptr: %p>", (void *)vm_popint(vm));
		vm_pushstr(vm, YASL_String_new_sized_heap(0, strlen(buffer), buffer));
	} else {
		vm_duptop(vm);
		vm_lookup_method_throwing(vm, "tostr", "tostr not supported for operand of type %s.", obj_typename(vm_peek_p(vm)));
		vm_swaptop(vm);
		vm_INIT_CALL_offset(vm, vm->sp - 1, 1);
		vm_CALL(vm);
	}
}

static struct Upvalue *add_upvalue(struct VM *const vm, struct YASL_Object *const location) {
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
			upval->next = curr->next;
			curr->next = upval;
			return upval;
		}
		return (curr->next = upval_new(location));
	}
	return (prev->next = upval_new(location));
}

static void vm_CCONST(struct VM *const vm) {
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
				  obj_typename(vm_peek_p(vm, vm->sp - 1)),
				  obj_typename(vm_peek_p(vm, vm->sp))
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
				  obj_typename(vm_peek_p(vm, vm->sp - 1)),
				  obj_typename(vm_peek_p(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	vm_pop(vm);
	vm_pop(vm);

	struct YASL_List *list = vm_poplist(vm);
	struct RC_UserData *new_ls = rcls_new();
	ud_setmt(new_ls, vm->builtins_htable[Y_LIST]);

	for (yasl_int i = start; i <end; ++i) {
		YASL_List_append((struct YASL_List *) new_ls->data, list->items[i]);
	}
	vm_push(vm, YASL_LIST(new_ls));
}

static void vm_SLICE_str(struct VM *const vm){
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
				  obj_typename(vm_peek_p(vm, vm->sp - 1)),
				  obj_typename(vm_peek_p(vm, vm->sp))
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
				  obj_typename(vm_peek_p(vm, vm->sp - 1)),
				  obj_typename(vm_peek_p(vm, vm->sp))
		);
		vm_throw_err(vm, YASL_TYPE_ERROR);
	}

	vm_pop(vm);
	vm_pop(vm);

	struct YASL_String *str = vm_popstr(vm);

	vm_push(vm, YASL_STR(YASL_String_new_substring((size_t)start, (size_t)end, str)));
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
	case Y_USERDATA_W:
	case Y_LIST:
	case Y_LIST_W:
	case Y_TABLE:
	case Y_TABLE_W:
		return YASL_GETUSERDATA(v)->mt;
	default:
		return vm->builtins_htable[v.type];
	}
}

struct YASL_Table *get_mt(const struct VM *const vm, struct YASL_Object v) {
	struct RC_UserData *ud = obj_get_metatable(vm, v);
	return ud ? (struct YASL_Table *)ud->data : NULL;
}

void vm_get_metatable(struct VM *const vm) {
	struct RC_UserData *mt = obj_get_metatable(vm, vm_pop(vm));
	vm_push(vm, mt ? YASL_TABLE(mt) : YASL_UNDEF());
}

static int vm_lookup_method_helper(struct VM *vm, struct YASL_Object obj, struct YASL_Table *mt, struct YASL_Object index) {
	(void) obj;
	if (!mt) return YASL_VALUE_ERROR;
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_push(vm,search);
		return YASL_SUCCESS;
	}

	return YASL_VALUE_ERROR;
}

static int lookup(struct VM *vm, struct YASL_Object obj, struct YASL_Table *mt, struct YASL_Object index) {
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_push(vm, search);
		return YASL_SUCCESS;
	}

	search = YASL_Table_search_string_int(mt, "__get", strlen("__get"));
	if (search.type != Y_END) {
		vm_push(vm, search);
		vm_call_now_2(vm, obj, index);
		return YASL_SUCCESS;
	}
	return YASL_VALUE_ERROR;
}

static int lookup2(struct VM *vm, struct YASL_Table *mt) {
	struct YASL_Object index = vm_peek(vm);
	struct YASL_Object search = YASL_Table_search(mt, index);
	if (search.type != Y_END) {
		vm_pop(vm);
		vm_pop(vm);
		vm_push(vm, search);
		return YASL_SUCCESS;
	}

	search = YASL_Table_search_string_int(mt, "__get", strlen("__get"));
	if (search.type != Y_END) {
		vm_push(vm, search);
		vm_shifttopdown(vm, 2);
		vm_INIT_CALL_offset(vm, vm->sp - 2, 1);
		vm_CALL(vm);
		//vm_call_now_2(vm, obj, index);
		return YASL_SUCCESS;
	}
	return YASL_VALUE_ERROR;
}

static void vm_GET_helper(struct VM *const vm, struct YASL_Object index) {
	struct YASL_Object v = vm_pop(vm);

	struct YASL_Table *mt = get_mt(vm, v);
	int result = YASL_ERROR;
	if (mt) {
		result = lookup(vm, v, mt, index);
	}

	if (result) {
		if (obj_istable(&v)) {
			struct YASL_Object search = YASL_Table_search(YASL_GETTABLE(v), index);
			if (search.type != Y_END) {
				vm_push(vm, search);
				return;
			}
		}
		vm_print_err_value(vm, "Could not find value for index%s", "");
		vm_throw_err(vm, YASL_VALUE_ERROR);
	}
}

static void vm_GET_helper2(struct VM *const vm) {
	struct YASL_Object index = vm_peek(vm);
	struct YASL_Object v = vm_peek(vm, vm->sp - 1);

	struct YASL_Table *mt = get_mt(vm, v);
	int result = YASL_ERROR;
	if (mt) {
		result = lookup2(vm, mt);
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

static void vm_GET(struct VM *const vm) {
	//struct YASL_Object index = vm_peek(vm);
	//struct YASL_Object v = vm_peek(vm, vm->sp - 1);
	//inc_ref(&index);
	//inc_ref(&v);
	vm_GET_helper2(vm);
	//dec_ref(&index);
	//dec_ref(&v);
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
	case Y_LIST: {
		struct YASL_List *list = YASL_GETLIST(frame->iterable);
		if (list->count <= (size_t) frame->iter) {
			vm_pushbool(vm, false);
		} else {
			vm_push(vm, list->items[frame->iter++]);
			vm_pushbool(vm, true);
		}
		return;
	}
	case Y_TABLE: {
		struct YASL_Table *table = YASL_GETTABLE(frame->iterable);
		while (table->size > (size_t) frame->iter &&
		       (table->items[frame->iter].key.type == Y_END || table->items[frame->iter].key.type == Y_UNDEF)) {
			frame->iter++;
		}
		if (table->size <= (size_t) frame->iter) {
			vm_pushbool(vm, false);
			return;
		}
		vm_push(vm, table->items[frame->iter++].key);
		vm_pushbool(vm, true);
		return;
	}
	case Y_STR: {
		struct YASL_String *str = obj_getstr(&frame->iterable);
		if (YASL_String_len(str) <= (size_t) frame->iter) {
			vm_pushbool(vm, false);
		} else {
			size_t i = (size_t) frame->iter;
			vm_pushstr(vm, YASL_String_new_substring(i, i + 1, str));
			frame->iter++;
			vm_pushbool(vm, true);
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
	case P_ANY:
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
		/* We offset by +2 instead of +1 here to account for the fact that the expression we are matching on is
		   still on top of the stack. This is taken care of _after_ a successful match by a O_DEL_FP instruction.
		  */
		unsigned char offset = NCODE(vm);
		dec_ref(&vm_peek(vm, vm->fp + offset + 2));
		vm_peek(vm, vm->fp + offset + 2) = *expr;
		inc_ref(&vm_peek(vm, vm->fp + offset + 2));
		return true;
	}
	case P_ANY:
		return true;
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

static bool vm_MATCH_pattern(struct VM *const vm, struct YASL_Object *expr) {
	return vm_MATCH_subpattern(vm, expr);
}

static void vm_MATCH_IF(struct VM *const vm) {
	struct YASL_Object expr = vm_peek(vm);
	yasl_int addr = vm_read_int(vm);
	unsigned char *start = vm->pc;
	if (!vm_MATCH_pattern(vm, &expr)) {
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
		vm->status = YASL_STACK_OVERFLOW_ERROR;
		vm_print_err(vm, "StackOverflow.");
		longjmp(vm->buf, 1);
	}

	int next_fp = vm->next_fp;
	vm->next_fp = offset;
	vm->frames[vm->frame_num] = ((struct CallFrame) { vm->pc, vm->fp, next_fp, vm->loopframe_num, num_returns });
}

static void vm_fill_args(struct VM *const vm, const int num_args);

static void vm_exitframe_multi(struct VM *const vm, int len) {
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
		dec_ref(&vm->loopframes[vm->loopframe_num--].iterable);
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
		dec_ref(&vm->loopframes[vm->loopframe_num--].iterable);
	}

	vm->frame_num--;
}

void vm_INIT_CALL_offset(struct VM *const vm, int offset, int expected_returns) {
	if (!vm_isfn(vm, offset) && !vm_iscfn(vm, offset) && !vm_isclosure(vm, offset)) {
		const char *name = obj_typename(vm_peek_p(vm, offset));
		vm_lookup_method_throwing(vm, "__call", "%s is not callable.", name);
	}

	vm_enterframe_offset(vm, offset, expected_returns);
}

static void vm_INIT_CALL(struct VM *const vm, int expected_returns) {
	vm_INIT_CALL_offset(vm, vm->sp, expected_returns);
}

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
	vm_duptop(vm);
	yasl_int addr = vm_read_int(vm);
	vm_GET_helper(vm, vm->constants[addr]);
	vm_swaptop(vm);
	vm_INIT_CALL_offset(vm, vm->sp - 1, expected_returns);
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

	vm_fill_args(vm, *code);

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

static void vm_ECHO(struct VM *const vm) {
	unsigned char top = NCODE(vm);
	YASL_UNUSED(top);
	YASL_ASSERT(top == vm->sp - vm->fp - 1, "wrong value for top of stack.");
	vm_stringify_top(vm);
	struct YASL_String *v = vm_popstr(vm);
	size_t strlen = (int)YASL_String_len(v);
	bool alloc = false;
	char *dest;
	if (strlen > SCRATCH_SIZE) {
		dest = (char *) malloc(strlen);
		alloc = true;
	} else {
		dest = (char *) &vm->scratch;
	}
	size_t copied = io_str_strip_char(dest, YASL_String_chars(v), strlen, 0);
	vm_print_out(vm, "%.*s\n", (int)copied, dest);
	if (alloc) {
		free(dest);
	}
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
			vm->constants[i] = YASL_STR(YASL_String_new_sized_heap(0, (size_t) len, str));
			inc_ref(vm->constants + i);
			tmp += len;
			break
			;
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

void vm_executenext(struct VM *const vm) {
	unsigned char opcode = NCODE(vm);        // fetch
	signed char offset;
	struct YASL_Object a, b;
	yasl_int c;
	YASL_VM_DEBUG_LOG("----------------"
			  "opcode: %x\n"
			  "vm->sp, vm->prev_fp, vm->curr_fp: %d, %d, %d\n\n", opcode, vm->sp, vm->fp, vm->next_fp);
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
	case O_BNOT:
		vm_int_unop(vm, &bnot, "^", OP_UN_CARET);
		break;
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
	case O_NEG:
		vm_num_unop(vm, &int_neg, &float_neg, "-", OP_UN_MINUS);
		break;
	case O_POS:
		vm_num_unop(vm, &int_pos, &float_pos, "+", OP_UN_PLUS);
		break;
	case O_NOT:
		vm_pushbool(vm, isfalsey(vm_pop_p(vm)));
		break;
	case O_LEN:
		vm_len_unop(vm);
		break;
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
		struct RC_UserData *table = rcht_new();
		struct YASL_Table *ht = (struct YASL_Table *)table->data;
		while (vm_peek(vm).type != Y_END) {
			struct YASL_Object val = vm_pop(vm);
			struct YASL_Object key = vm_pop(vm);
			if (obj_isundef(&val)) {
				continue;
			}
			if (!YASL_Table_insert(ht, key, val)) {
				rcht_del(table);
				vm_print_err_type(vm, "unable to use mutable object of type %s as key.", obj_typename(&key));
				vm_throw_err(vm, YASL_TYPE_ERROR);
			}
		}
		ud_setmt(table, vm->builtins_htable[Y_TABLE]);

		vm_pop(vm);
		vm_push(vm, YASL_TABLE(table));
		break;
	}
	case O_NEWLIST: {
		struct RC_UserData *ls = rcls_new();
		ud_setmt(ls, vm->builtins_htable[Y_LIST]);
		int len = 0;
		while (vm_peek(vm, vm->sp - len).type != Y_END) {
			len++;
		}
		for (int i = 0; i < len; i++) {
			YASL_List_append((struct YASL_List *) ls->data, vm_peek(vm, vm->sp - len + i + 1));
		}
		vm->sp -= len + 1;
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
		a = vm_pop(vm);
		vm_pop(vm);
		vm_push(vm, a);
		dec_ref(&vm->loopframes[vm->loopframe_num].iterable);
		vm->loopframe_num--;
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
		a = vm_peek(vm, vm->fp + offset + 1);
		memmove(vm->stack + vm->fp + offset + 1, vm->stack + vm->fp + offset + 2, (vm->sp - (vm->fp + offset + 1)) * sizeof(struct YASL_Object));
		vm->stack[vm->sp] = a;
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
		vm_INIT_MC(vm);
		break;
	case O_INIT_CALL:
		vm_INIT_CALL(vm, (signed char)NCODE(vm));
		break;
	case O_CALL:
		vm_CALL(vm);
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
	default:
		vm_print_err(vm, "Error: Unknown Opcode: %x\n", opcode);
		vm_throw_err(vm, YASL_ERROR);
	}
}

int vm_run(struct VM *const vm) {
	if (setjmp(vm->buf)) {
		return vm->status;
	}

	vm_setupconstants(vm);

	while (true) {
		vm_executenext(vm);
	}
}
