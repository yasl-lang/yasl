#include "VM.h"

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "YASL_Object.h"
#include "builtins.h"
#include "YASL_string.h"
#include "hashtable.h"
#include "refcount.h"

#include "table_methods.h"
#include "list_methods.h"
#include "yasl_state.h"
#include "yasl_error.h"
#include "yasl_include.h"
#include "opcode.h"
#include "operator_names.h"

static struct Table **builtins_htable_new(void) {
    struct Table **ht = malloc(sizeof(struct Table*) * NUM_TYPES);
    ht[Y_UNDEF] = undef_builtins();
    ht[Y_FLOAT] = float_builtins();
    ht[Y_INT] = int_builtins();
    ht[Y_BOOL] = bool_builtins();
    ht[Y_STR] = str_builtins();
    ht[Y_LIST] = list_builtins();
    ht[Y_TABLE] = table_builtins();

    return ht;
}

struct VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize) {      // total params size required to perform a program operations
    struct VM* vm = malloc(sizeof(struct VM));
    vm->code = code;
    vm->pc = pc0;
    vm->pc0 = vm->pc;
    vm->fp = -1;
    vm->lp = -1;
    vm->sp = -1;
    vm->globals = calloc(sizeof(struct YASL_Object), datasize);

    vm->num_globals = datasize;

    vm->stack = calloc(sizeof(struct YASL_Object), STACK_SIZE);

    vm->builtins_htable = builtins_htable_new();
    return vm;
}

void vm_del(struct VM *vm) {
	for (size_t i = 0; i < STACK_SIZE; i++) dec_ref(&vm->stack[i]);
	for (size_t i = 0; i < vm->num_globals; i++) {
		dec_ref(&vm->globals[i]);
	}

	free(vm->globals);
	free(vm->stack);

	free(vm->code);


	table_del(vm->builtins_htable[Y_UNDEF]);
	table_del(vm->builtins_htable[Y_FLOAT]);
	table_del(vm->builtins_htable[Y_INT]);
	table_del(vm->builtins_htable[Y_BOOL]);
	table_del(vm->builtins_htable[Y_STR]);
	table_del(vm->builtins_htable[Y_LIST]);
	table_del(vm->builtins_htable[Y_TABLE]);
	free(vm->builtins_htable);

	free(vm);
}

void vm_push(struct VM *vm, struct YASL_Object val) {
    vm->sp++;
    dec_ref(vm->stack + vm->sp);
    vm->stack[vm->sp] = val;
    inc_ref(vm->stack + vm->sp);
}

struct YASL_Object vm_pop(struct VM *vm) {
    return vm->stack[vm->sp--];
}

struct YASL_Object vm_peek(struct VM *vm) {
    return vm->stack[vm->sp];
}

yasl_int vm_read_int(struct VM *vm) {
    yasl_int val;
    memcpy(&val, vm->code + vm->pc, sizeof(yasl_int));
    vm->pc += sizeof (yasl_int);
    return val;
}

yasl_float vm_read_float(struct VM *vm) {
    yasl_float val;
    memcpy(&val, vm->code + vm->pc, sizeof(yasl_float));
    vm->pc += sizeof(yasl_float);
    return val;
}

yasl_int bor(yasl_int left, yasl_int right) {
    return left | right;
}

yasl_int bxor(yasl_int left, yasl_int right) {
    return left ^ right;
}

yasl_int band(yasl_int left, yasl_int right) {
    return left & right;
}

yasl_int bandnot(yasl_int left, yasl_int right) {
    return left & ~right;
}

yasl_int shift_left(yasl_int left, yasl_int right) {
    return left << right;
}

yasl_int shift_right(yasl_int left, yasl_int right) {
    return left >> right;
}

yasl_int modulo(yasl_int left, yasl_int right) {
    return left % right;
}

yasl_int idiv(yasl_int left, yasl_int right) {
    return left / right;
}

int vm_int_binop(struct VM *vm, yasl_int (*op)(yasl_int, yasl_int), char *opstr, char *overload_name) {
    struct YASL_Object b = vm_pop(vm);
    struct YASL_Object a = vm_pop(vm);
    if (YASL_ISINT(a) && YASL_ISINT(b)) {
        vm_push(vm, YASL_INT(op(YASL_GETINT(a), YASL_GETINT(b))));
        return YASL_SUCCESS;
    }  else if (YASL_ISTBL(a)) {
        struct YASL_Object *key = YASL_String(str_new_sized(strlen(overload_name), overload_name));
        struct YASL_Object *result = table_search(YASL_GETTBL(a), *key);
        if (result && YASL_ISFN(*result)) {
            vm->sp += 2;

            int offset = 2;
            int addr = YASL_GETINT(*result);
            a = ((struct YASL_Object) {.type = offset, .value.ival = vm->fp});
            b = ((struct YASL_Object) { .type = offset, .value.ival = vm->pc});
            vm_push(vm, a);  // store previous frame ptr;
            vm_push(vm, b);  // store pc addr
            vm->fp = vm->sp;

            vm->sp += offset + 1; // + 2
            vm->pc = addr + 2;
        } else {
            YASL_PRINT_ERROR_TYPE("%s is not callable (type %s).\n", overload_name, YASL_TYPE_NAMES[result->type]);
        }
    } else {
        YASL_PRINT_ERROR_TYPE("%s not supported for operands of types %s and %s.\n",
               opstr,
               YASL_TYPE_NAMES[a.type],
               YASL_TYPE_NAMES[b.type]);
        return YASL_TYPE_ERROR;
    }
    return YASL_SUCCESS;
}

yasl_int int_add(yasl_int left, yasl_int right) {
    return left + right;
}

yasl_float float_add(yasl_float left, yasl_float right) {
    return left + right;
}

yasl_int int_sub(yasl_int left, yasl_int right) {
    return left - right;
}

yasl_float float_sub(yasl_float left, yasl_float right) {
    return left - right;
}

yasl_int int_mul(yasl_int left, yasl_int right) {
    return left * right;
}

yasl_float float_mul(yasl_float left, yasl_float right) {
    return left * right;
}

yasl_int int_pow(yasl_int left, yasl_int right) {
    return (yasl_int)pow(left, right);
}

int vm_num_binop(
        struct VM *vm, yasl_int (*int_op)(yasl_int, yasl_int),
        yasl_float (*float_op)(yasl_float, yasl_float),
        const char *const opstr,
        char *overload_name) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_pop(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_INT(int_op(YASL_GETINT(left), YASL_GETINT(right))));
	} else if (YASL_ISFLOAT(left) && YASL_ISFLOAT(right)) {
		vm_push(vm, YASL_FLOAT(float_op(YASL_GETFLOAT(left), YASL_GETFLOAT(right))));
	} else if (YASL_ISFLOAT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_FLOAT(float_op(YASL_GETFLOAT(left), YASL_GETINT(right))));
	} else if (YASL_ISINT(left) && YASL_ISFLOAT(right)) {
		vm_push(vm, YASL_FLOAT(float_op(YASL_GETINT(left), YASL_GETFLOAT(right))));
	} else if (YASL_ISTBL(left)) {
		struct YASL_Object *key = YASL_String(str_new_sized(strlen(overload_name), overload_name));
		struct YASL_Object *result = table_search(YASL_GETTBL(left), *key);
		if (result && YASL_ISFN(*result)) {
			vm->sp += 2;

			int offset = 2;
			int64_t addr = YASL_GETINT(*result);
			struct YASL_Object a = ((struct YASL_Object) {.type = offset, .value.ival = vm->fp});
			struct YASL_Object b = ((struct YASL_Object) {.type = offset, .value.ival =  vm->pc});
			vm_push(vm, a);  // store previous frame ptr;
			vm_push(vm, b);  // store pc addr
			vm->fp = vm->sp;

			vm->sp += offset + 1; // + 2
			vm->pc = addr + 2;
		} else {
			YASL_PRINT_ERROR_TYPE("%s is not callable (type %s).\n", overload_name,
			       YASL_TYPE_NAMES[result->type]);
		}
	} else {
		YASL_PRINT_ERROR_TYPE("%s not supported for operands of types %s and %s.\n",
		       opstr,
		       YASL_TYPE_NAMES[left.type],
		       YASL_TYPE_NAMES[right.type]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int vm_fdiv(struct VM *vm) {
	char *overload_name = OP_BIN_FDIV;
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_pop(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_FLOAT((yasl_float) YASL_GETINT(left) / (yasl_float) YASL_GETINT(right)));
	} else if (YASL_ISFLOAT(left) && YASL_ISFLOAT(right)) {
		vm_push(vm, YASL_FLOAT(YASL_GETFLOAT(left) / YASL_GETFLOAT(right)));
	} else if (YASL_ISINT(left) && YASL_ISFLOAT(right)) {
		vm_push(vm, YASL_FLOAT((yasl_float) YASL_GETINT(left) / YASL_GETFLOAT(right)));
	} else if (YASL_ISFLOAT(left) && YASL_ISINT(right)) {
		vm_push(vm, YASL_FLOAT(YASL_GETFLOAT(left) / (yasl_float) YASL_GETINT(right)));
	} else if (YASL_ISTBL(left)) {
		struct YASL_Object *key = YASL_String(str_new_sized(strlen(overload_name), overload_name));
		struct YASL_Object *result = table_search(YASL_GETTBL(left), *key);
		if (result && YASL_ISFN(*result)) {
			vm->sp += 2;

			int offset = 2;
			int addr = YASL_GETINT(*result);
			struct YASL_Object a = ((struct YASL_Object) {.type = offset, .value.ival =  vm->fp});
			struct YASL_Object b = ((struct YASL_Object) {.type = offset, .value.ival =  vm->pc});
			vm_push(vm, a);  // store previous frame ptr;
			vm_push(vm, b);  // store pc addr
			vm->fp = vm->sp;

			vm->sp += offset + 1; // + 2
			vm->pc = addr + 2;
		} else {
			YASL_PRINT_ERROR_TYPE("%s is not callable (type %s).\n", overload_name,
			       YASL_TYPE_NAMES[result->type]);
		}
	} else {
		YASL_PRINT_ERROR_TYPE("/ not supported for operands of types %s and %s.\n",
		       YASL_TYPE_NAMES[left.type],
		       YASL_TYPE_NAMES[right.type]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int vm_pow(struct VM *vm) {
	struct YASL_Object right = vm_pop(vm);
	struct YASL_Object left = vm_peek(vm);
	if (YASL_ISINT(left) && YASL_ISINT(right) && YASL_GETINT(right) < 0) {
		vm_pop(vm);
		vm_push(vm, YASL_FLOAT(pow(YASL_GETINT(left), YASL_GETINT(right))));
	} else {
		vm->sp++;
		int res = vm_num_binop(vm, &int_pow, &pow, "**", OP_BIN_POWER);
		if (res) return res;
	}
	return YASL_SUCCESS;
}

yasl_int bnot(yasl_int val) {
    return ~val;
}

int vm_int_unop(struct VM *vm, yasl_int (*op)(yasl_int), char *opstr) {
	struct YASL_Object a = PEEK(vm);
	if (YASL_ISINT(a)) {
		PEEK(vm).value.ival = op(YASL_GETINT(a));
		return YASL_SUCCESS;
	} else {
		YASL_PRINT_ERROR_TYPE("%s not supported for operand of type %s.\n",
		       opstr,
		       YASL_TYPE_NAMES[a.type]);
		return YASL_TYPE_ERROR;
	}
}

yasl_int int_neg(yasl_int expr) {
    return -expr;
}

yasl_float float_neg(yasl_float expr) {
    return -expr;
}

int vm_num_unop(struct VM *vm, yasl_int (*int_op)(yasl_int), yasl_float (*float_op)(yasl_float), char *opstr) {
	struct YASL_Object expr = vm_pop(vm);
	if (YASL_ISINT(expr)) {
		vm_push(vm, YASL_INT(int_op(YASL_GETINT(expr))));
	} else if (YASL_ISFLOAT(expr)) {
		vm_push(vm, YASL_FLOAT(float_op(YASL_GETFLOAT(expr))));
	} else {
		YASL_PRINT_ERROR_TYPE("%s not supported for operand of type %s.\n",
		       opstr,
		       YASL_TYPE_NAMES[expr.type]);
		return YASL_TYPE_ERROR;
	}
	return YASL_SUCCESS;
}

int vm_run(struct VM *vm) {
	while (1) {
		unsigned char opcode = NCODE(vm);        // fetch
		signed char offset;
		size_t size;
		yasl_int addr;
		struct YASL_Object a, b, v;
		yasl_int c;
		yasl_float d;
		int res;
		// printf("vm->pc, vm->sp, opcode: %x, %d, %x\n", vm->pc - vm->pc0, vm->sp, opcode);
		//printf("%d\n----------\n", vm->builtins_htable[Y_LIST]);
		//printf("----------------\nvm->sp, vm->fp, vm->pc, opcode: %d, %d, %d, 0x%x\n", vm->sp, vm->fp, vm->pc, opcode);
		// printf("vm->pc, opcode: %x, %x\n", vm->pc - vm->pc0, opcode);
		//printf("pc: %d\n\n", vm->pc);
		//print(vm->stack[vm->sp]);
		//puts("\n");
		/* printf("tpye is: %s\n", YASL_TYPE_NAMES[PEEK(vm).type]);
		print(PEEK(vm));
		 */
		//print(vm->globals[6]);
		switch (opcode) {   // decode
		case HALT:
			return YASL_SUCCESS;  // stop the program
		case NOP:
			puts("Slide");
			break;    // pass
		case ICONST_M1:     // TODO: make sure no changes to opcodes ruin this
		case ICONST_0:
		case ICONST_1:
		case ICONST_2:
		case ICONST_3:
		case ICONST_4:
		case ICONST_5:
			vm_pushint(vm, opcode - 0x04); //(vm, YASL_INT(opcode - 0x04));
			break;
		case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
		case DCONST_1:
		case DCONST_2:
			vm_pushfloat(vm, opcode - 0x0B); // (vm, YASL_FLOAT(opcode - 0x0B));
			break;
		case DCONST_N:
			vm_pushfloat(vm, 0.0 / 0.0);
			break;
		case DCONST_I:
			vm_pushfloat(vm, 1.0 / 0.0);
			break;
		case DCONST:        // constants have native endianness
			d = vm_read_float(vm);
			vm_pushfloat(vm, d);
			break;
		case ICONST:        // constants have native endianness
			c = vm_read_int(vm);
			vm_pushint(vm, c);
			break;
		case BCONST_F:
		case BCONST_T:
			vm_pushbool(vm, opcode & 0x01);
			break;
		case NCONST:
			vm_pushundef(vm); //, YASL_UNDEF());
			break;
		case FCONST:
			c = vm_read_int(vm);
			vm_pushfn(vm, c);
			break;
		case BOR:
			if ((res = vm_int_binop(vm, &bor, "|", OP_BIN_BAR))) return res;
			break;
		case BXOR:
			if ((res = vm_int_binop(vm, &bxor, "^", OP_BIN_CARET))) return res;
			break;
		case BAND:
			if ((res = vm_int_binop(vm, &band, "&", OP_BIN_AMP))) return res;
			break;
		case BANDNOT:
			if ((res = vm_int_binop(vm, &bandnot, "&^", OP_BIN_AMPCARET))) return res;
			break;
		case BNOT:
			if ((res = vm_int_unop(vm, &bnot, "^"))) return res;
			break;
		case BSL:
			if ((res = vm_int_binop(vm, &shift_left, "<<", OP_BIN_SHL))) return res;
			break;
		case BSR:
			if ((res = vm_int_binop(vm, &shift_right, ">>", OP_BIN_SHR))) return res;
			break;
		case ADD:
			if ((res = vm_num_binop(vm, &int_add, &float_add, "+", OP_BIN_PLUS))) return res;
			break;
		case MUL:
			if ((res = vm_num_binop(vm, &int_mul, &float_mul, "*", OP_BIN_TIMES))) return res;
			break;
		case SUB:
			if ((res = vm_num_binop(vm, &int_sub, &float_sub, "-", OP_BIN_MINUS))) return res;
			break;
		case FDIV:
			if ((res = vm_fdiv(vm))) return res;   // handled differently because we always convert to float
			break;
		case IDIV:
			if (YASL_ISINT(vm_peek(vm)) && YASL_GETINT(vm_peek(vm)) == 0) {
				YASL_PRINT_ERROR_DIVIDE_BY_ZERO();
				return YASL_DIVIDE_BY_ZERO_ERROR;
				break;
			}
			if ((res = vm_int_binop(vm, &idiv, "//", OP_BIN_IDIV))) return res;
			break;
		case MOD:
			// TODO: handle undefined C behaviour for negative numbers.
			if ((res = vm_int_binop(vm, &modulo, "%", OP_BIN_MOD))) return res;
			break;
		case EXP:
			if ((res = vm_pow(vm))) return res;
			break;
		case NEG:
			if ((res = vm_num_unop(vm, &int_neg, &float_neg, "-"))) return res;
			break;
		case NOT:
			c = isfalsey(vm_pop(vm));
			vm_pushbool(vm, c);
			break;
		case LEN:
			v = vm_pop(vm);
			if (YASL_ISSTR(v)) {
				vm_pushint(vm, yasl_string_len(YASL_GETSTR(v)));
			} else if (YASL_ISTBL(v)) {
				vm_pushint(vm, YASL_GETTBL(v)->count);
			} else if (YASL_ISLIST(v)) {
				vm_pushint(vm, YASL_GETLIST(v)->count);
			} else {
				YASL_PRINT_ERROR_TYPE("len not supported for operand of type %s.\n",
							      YASL_TYPE_NAMES[v.type]);
				return YASL_TYPE_ERROR;
			}
			break;
		case CNCT:
			if (!YASL_ISSTR(vm_peek(vm))) {
				YASL_Types index = vm_peek(vm).type;
				struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
				struct YASL_Object *result = table_search(vm->builtins_htable[index], key);
				str_del(YASL_GETSTR(key));
				YASL_GETCFN(*result)->value((struct YASL_State *) &vm);
			}
			b = vm_pop(vm);
			if (!YASL_ISSTR(vm_peek(vm))) {
				YASL_Types index = vm_peek(vm).type;
				struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
				struct YASL_Object *result = table_search(vm->builtins_htable[index], key);
				str_del(YASL_GETSTR(key));
				YASL_GETCFN(*result)->value((struct YASL_State *) &vm);
			}
			a = vm_pop(vm);

			size = yasl_string_len(YASL_GETSTR(a)) + yasl_string_len(YASL_GETSTR(b));
			char *ptr = malloc(size);
			memcpy(ptr, YASL_GETSTR(a)->str + YASL_GETSTR(a)->start,
			       yasl_string_len(YASL_GETSTR(a)));
			memcpy(ptr + yasl_string_len(YASL_GETSTR(a)),
			       (YASL_GETSTR(b))->str + YASL_GETSTR(b)->start,
			       yasl_string_len(YASL_GETSTR(b)));
			vm_pushstr(vm, str_new_sized_heap(0, size, ptr));
			break;
			YASL_PRINT_ERROR_TYPE("~ not supported for operands of types %s and %s.\n",
			       YASL_TYPE_NAMES[a.type],
			       YASL_TYPE_NAMES[b.type]);
			return YASL_TYPE_ERROR;
		case GT:
			b = vm_pop(vm);
			a = vm_pop(vm);
			if (YASL_ISSTR(a) && YASL_ISSTR(b)) {
				vm_pushbool(vm, yasl_string_cmp(YASL_GETSTR(a), YASL_GETSTR(b)) > 0);
				break;
			}
			if ((!YASL_ISINT(a) && !YASL_ISFLOAT(a)) ||
			    (!YASL_ISINT(b) && !YASL_ISFLOAT(b))) {
				YASL_PRINT_ERROR_TYPE("< and > not supported for operand of types %s and %s.\n",
				       YASL_TYPE_NAMES[a.type],
				       YASL_TYPE_NAMES[b.type]);
				return YASL_TYPE_ERROR;
			}
			COMP(vm, a, b, GT, ">");
			break;
		case GE:
			b = vm_pop(vm);
			a = vm_pop(vm);
			if (YASL_ISSTR(a) && YASL_ISSTR(b)) {
				vm_push(vm, YASL_BOOL(yasl_string_cmp(YASL_GETSTR(a), YASL_GETSTR(b)) >= 0));
				break;
			}
			if ((!YASL_ISINT(a) && !YASL_ISFLOAT(a)) ||
			    (!YASL_ISINT(b) && !YASL_ISFLOAT(b))) {
				YASL_PRINT_ERROR_TYPE("<= and >= not supported for operand of types %s and %s.\n",
				       YASL_TYPE_NAMES[a.type],
				       YASL_TYPE_NAMES[b.type]);
				return YASL_TYPE_ERROR;
			}
			COMP(vm, a, b, GE, ">=");
			break;
		case EQ:
			b = vm_pop(vm);
			a = vm_pop(vm);
			v = isequal(a, b);
			vm_push(vm, v);
			break;
		case ID: // TODO: clean-up
			b = vm_pop(vm);
			a = vm_pop(vm);
			vm_push(vm, YASL_BOOL(a.type == b.type && YASL_GETINT(a) == YASL_GETINT(b)));
			break;
		case NEWSTR:
			addr = vm_read_int(vm);
			memcpy(&size, vm->code + addr, sizeof(yasl_int));
			addr += sizeof(yasl_int);
			vm_pushstr(vm, str_new_sized(size, ((char *) vm->code) + addr));
			break;
		case NEWTABLE: {
			struct YASL_Object *table = YASL_Table();
			struct Table *ht = YASL_GETTBL(*table);
			while (PEEK(vm).type != Y_END) {
				struct YASL_Object value = vm_pop(vm);
				struct YASL_Object key = vm_pop(vm);
				table_insert(ht, key, value);
			}
			vm_pop(vm);
			vm_push(vm, *table);
			free(table);
			break;
		}
		case NEWLIST: {
			struct RC_UserData *ls = ls_new();
			while (vm_peek(vm).type != Y_END) {
				ls_append(ls->data, vm_pop(vm));
			}
			ls_reverse(ls->data);
			vm_pop(vm);
			vm_push(vm, YASL_LIST(ls));
			break;
		}
		case INITFOR:
			vm_pushint(vm, 0);
			vm_pushint(vm, vm->lp);
			vm->lp = vm->sp - 2;
			break;
		case ENDCOMP:
			a = vm_pop(vm);
			vm->lp = vm_popint(vm);
			vm_pop(vm);
			vm_pop(vm);
			vm_push(vm, a);
			break;
		case ENDFOR:
			vm->lp = vm_popint(vm);
			vm_pop(vm);
			vm_pop(vm);
			break;
		case ITER_1:
			switch (VM_PEEK(vm, vm->lp).type) {
			case Y_LIST:
				if (vm_peeklist(vm, vm->lp)->count <= vm_peekint(vm, vm->lp + 1)) {
					vm_pushbool(vm, 0);
				} else {
					vm_push(vm, vm_peeklist(vm, vm->lp)->items[vm_peekint(vm, vm->lp + 1)++]);
					vm_pushbool(vm, 1);
				}
				break;
			case Y_TABLE:
				while ((vm_peektable(vm, vm->lp)->items[vm_peekint(vm, vm->lp + 1)] ==
					&TOMBSTONE ||
					vm_peektable(vm, vm->lp)->items[vm_peekint(vm, vm->lp + 1)] ==
					NULL
				       ) &&
					vm_peektable(vm, vm->lp)->size >
				       (size_t) vm_peekint(vm, vm->lp + 1)) {
					vm_peekint(vm, vm->lp + 1)++;
				}
				if (vm_peektable(vm, vm->lp)->size <=
				    (size_t) vm_peekint(vm, vm->lp + 1)) {
					vm_pushbool(vm, 0);
					break;
				}
				vm_push(vm,
					*vm_peektable(vm, vm->lp)->items[vm_peekint(vm, vm->lp + 1)++]->key);
				vm_pushbool(vm, 1);
				break;
			case Y_STR:
				if (yasl_string_len(vm_peekstr(vm, vm->lp)) <= vm_peekint(vm, vm->lp + 1)) {
					vm_push(vm, YASL_BOOL(0));
				} else {
					int64_t i = vm_peekint(vm, vm->lp + 1);
					vm_push(vm, YASL_STR(str_new_substring(i, i+1, vm_peekstr(vm, vm->lp))));
					vm_peekint(vm, vm->lp + 1)++;
					vm_pushbool(vm, 1);
				}
				break;
			default:
				YASL_PRINT_ERROR_TYPE("object of type %s is not iterable.\n", YASL_TYPE_NAMES[vm->stack[vm->lp].type]);
				return YASL_TYPE_ERROR;
			}
			break;
		case ITER_2:
			puts("NOT IMPLEMENTED");
			exit(1);
		case END:
			vm_pushend(vm); //(struct YASL_Object) {.type = Y_END });
			break;
		case DUP: {
			a = vm_peek(vm);
			vm_push(vm, a);
			break;
		}
		case SWAP:
			a = vm->stack[vm->sp];
			b = vm->stack[vm->sp - 1];
			vm->stack[vm->sp - 1] = a;
			vm->stack[vm->sp] = b;
			break;
		case BR_8:
			c = vm_read_int(vm);
			vm->pc += c;
			break;
		case BRF_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (isfalsey(v)) vm->pc += c;
			break;
		case BRT_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (!(isfalsey(v))) vm->pc += c;
			break;
		case BRN_8:
			c = vm_read_int(vm);
			v = vm_pop(vm);
			if (YASL_ISUNDEF(v)) vm->pc += c;
			break;
		case GLOAD_1:
			addr = vm->code[vm->pc++];               // get addr of var in params
			vm_push(vm, vm->globals[addr]);
			break;
		case GSTORE_1:
			addr = vm->code[vm->pc++];
			dec_ref(&vm->globals[addr]);
			vm->globals[addr] = vm_pop(vm);
			inc_ref(&vm->globals[addr]);
			break;
		case LLOAD_1:
			offset = NCODE(vm);
			vm_push(vm, VM_PEEK(vm, vm->fp + offset + 3));
			break;
		case LSTORE_1:
			offset = NCODE(vm);
			dec_ref(&VM_PEEK(vm, vm->fp + offset + 3));
			VM_PEEK(vm, vm->fp + offset + 3) = vm_pop(vm);
			inc_ref(&VM_PEEK(vm, vm->fp + offset + 3));
			break;
		case INIT_CALL:
			if (!YASL_ISFN(vm_peek(vm)) && !YASL_ISCFN(vm_peek(vm))) {
				YASL_PRINT_ERROR_TYPE("%s is not callable.", YASL_TYPE_NAMES[PEEK(vm).type]);
				return YASL_TYPE_ERROR;
			}

			vm_pushint(vm, vm->fp);
			vm_pushint(vm, vm->fp);
			vm->next_fp = vm->sp - 2;
			break;
		case CALL:
			vm->fp = vm->next_fp;
			if (YASL_ISFN(vm->stack[vm->fp])) {
				vm->stack[vm->fp + 1].value.ival = vm->pc;

				while (vm->sp - (vm->fp + 2) < vm->code[vm_peekint(vm, vm->fp)]) {
					vm_pushundef(vm);
				}

				while (vm->sp - (vm->fp + 2) > vm->code[vm_peekint(vm, vm->fp)]) {
					vm_pop(vm);
				}

				vm->sp += vm->code[vm_peekint(vm, vm->fp) + 1];

				vm->pc = vm_peekint(vm, vm->fp) + 2;
			} else if (YASL_ISCFN(vm->stack[vm->fp])) {
				while (vm->sp - (vm->fp + 2) < vm_peekcfn(vm, vm->fp)->num_args) {
					vm_pushundef(vm);
				}

				while (vm->sp - (vm->fp + 2) > vm_peekcfn(vm, vm->fp)->num_args) {
					vm_pop(vm);
				}
				if (vm_peekcfn(vm, vm->fp)->value((struct YASL_State *) &vm)) {
					printf("ERROR: invalid argument type(s) to builtin function.\n");
					return YASL_TYPE_ERROR;
				};
				v = vm_pop(vm);
				vm->sp = vm->fp + 2;
				vm->fp = vm_popint(vm);
				vm_pop(vm);
				vm_pop(vm);
				vm_push(vm, v);
			} else {
				return YASL_TYPE_ERROR;
			}
			break;
		case RET:
			// TODO: handle multiple returns
			v = vm_pop(vm);
			vm->sp = vm->fp + 2;
			vm->fp = vm_popint(vm);
			vm->pc = vm_popint(vm);
			vm_pop(vm);
			vm_push(vm, v);
			break;
		case GET: {
			vm->sp--;
			int index = vm_peek(vm).type;
			if (YASL_ISLIST(vm_peek(vm))) {
				vm->sp++;
				if (!list___get((struct YASL_State *) &vm)) {
					break;
				}
			} else if (YASL_ISTBL(vm_peek(vm))) {
				vm->sp++;
				if (!table___get((struct YASL_State *) &vm)) {
					break;
				}
			} else {
				vm->sp++;
			}

			struct YASL_Object key = vm_pop(vm);
			struct YASL_Object *result = table_search(vm->builtins_htable[index], key);
			vm_pop(vm);
			if (result == NULL) {
				vm_pushundef(vm);
			} else {
				vm_push(vm, *result);
			}
			break;
		}
		case SET: {
			vm->sp -= 2;
			if (YASL_ISLIST(vm_peek(vm))) {
				vm->sp += 2;
				list___set((struct YASL_State *) &vm);
			} else if (YASL_ISTBL(vm_peek(vm))) {
				vm->sp += 2;
				table___set((struct YASL_State *) &vm);
			} else {
				vm->sp += 2;
				YASL_PRINT_ERROR_TYPE("object of type %s is immutable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
				return YASL_TYPE_ERROR;
			}
			break;
		}
		case RCALL_8:
			offset = NCODE(vm);
			int i;
			for (i = 0; i < offset; i++) {
				vm->stack[vm->fp - 2 - i] = vm->stack[vm->sp - i];
			}
			addr = vm_read_int(vm);
			offset = NCODE(vm);
			vm->sp = vm->fp + offset;
			vm->pc = addr;
			break;
			/*case RET:
			    v = vm_pop(vm);
			    a = vm->stack[vm->fp];
			    b = vm->stack[vm->fp-1];
			    vm->pc = a.value.ival;
			    vm->sp = vm->fp - a.type - 2;
			    vm->fp = b.value.ival;
			    vm_push(vm, &v);
			    break;*/
		case POP:
			vm_pop(vm);
			break;
		case PRINT:
			yasl_print(vm);
			break;
		default:
			YASL_PRINT_ERROR("ERROR UNKNOWN OPCODE: %x\n", opcode);
			return YASL_ERROR;
		}
	}
}
