#include <stdlib.h>
#include <memory.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "VM.h"
#include "interpreter/builtins/builtins.h"
#include <functions.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <hashtable/hashtable.h>
#include <color.h>
#include <interpreter/refcount/refcount.h>
#include "yasl_state.h"

static struct YASL_Object *YASL_End() {
    struct YASL_Object *end = malloc(sizeof(struct YASL_Object));
    end->type = Y_END;
    return end;
}
#include "operator_names.h"

static Hash_t **builtins_htable_new(void) {
    Hash_t **ht = malloc(sizeof(Hash_t*) * NUM_TYPES);
    ht[Y_FLOAT64] = float64_builtins();
    ht[Y_INT64] = int64_builtins();
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
    //return;
    for (int i = 0; i < STACK_SIZE; i++) dec_ref(&vm->stack[i]);
    for (int i = 0; i < vm->num_globals; i++) dec_ref(&vm->globals[i]);

    free(vm->globals);                   // TODO: free these properly
    free(vm->stack);                     // TODO: free these properly

    free(vm->code);

    //ht_del_string_int(vm->builtins_htable[0]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_FLOAT64]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_INT64]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_BOOL]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_STR]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_LIST]);
    ht_del_cstring_cfn(vm->builtins_htable[Y_TABLE]);
    free(vm->builtins_htable);

    free(vm);
}

void vm_push(struct VM *vm, struct YASL_Object val) {
    vm->sp++;
    dec_ref(vm->stack + vm->sp);
    // Assert no NULL?
    vm->stack[vm->sp] = val;
    inc_ref(vm->stack + vm->sp);
}

struct YASL_Object vm_pop(struct VM *vm) {
    return vm->stack[vm->sp--];
}

struct YASL_Object vm_peek(struct VM *vm) {
    return vm->stack[vm->sp];
}

int64_t vm_read_int64_t(struct VM *vm) {
    int64_t val;
    memcpy(&val, vm->code + vm->pc, sizeof( int64_t));
    vm->pc += sizeof (int64_t);
    return val;
}

double vm_read_double(struct VM *vm) {
    double val;
    memcpy(&val, vm->code + vm->pc, sizeof(double));
    vm->pc += sizeof(double);
    return val;
}

int64_t bor(int64_t left, int64_t right) {
    return left | right;
}

int64_t bxor(int64_t left, int64_t right) {
    return left ^ right;
}

int64_t band(int64_t left, int64_t right) {
    return left & right;
}

int64_t bandnot(int64_t left, int64_t right) {
    return left & ~right;
}

int64_t shift_left(int64_t left, int64_t right) {
    return left << right;
}

int64_t shift_right(int64_t left, int64_t right) {
    return left >> right;
}

int64_t modulo(int64_t left, int64_t right) {
    return left % right;
}

int64_t idiv(int64_t left, int64_t right) {
    return left / right;
}

void vm_int_binop(struct VM *vm, int64_t (*op)(int64_t, int64_t), char *opstr, char *overload_name) {
    struct YASL_Object b = vm_pop(vm);
    struct YASL_Object a = vm_pop(vm);
    if (yasl_type_equals(a.type, Y_INT64) && yasl_type_equals(b.type, Y_INT64)) {
        vm_push(vm, YASL_INT(op(a.value.ival, b.value.ival)));
        return;
    }  else if (yasl_type_equals(a.type, Y_TABLE)) {
        struct YASL_Object *key = YASL_String(str_new_sized_from_mem(0, strlen(overload_name), overload_name));
        struct YASL_Object *result = ht_search(a.value.mval, *key);
        if (result && yasl_type_equals(result->type, Y_FN)) {
            vm->sp += 2;

            int offset = 2;
            int addr = result->value.ival;
            a = ((struct YASL_Object) {offset, vm->fp});
            b = ((struct YASL_Object) {offset, vm->pc});
            vm_push(vm, a);  // store previous frame ptr;
            vm_push(vm, b);  // store pc addr
            vm->fp = vm->sp;

            vm->sp += offset + 1; // + 2
            vm->pc = addr + 2;
        } else {
            printf("TypeError: %s is not callable (type %s).\n", overload_name, YASL_TYPE_NAMES[result->type]);
        }
    } else {
        printf("TypeError: %s not supported for operands of types %s and %s.\n",
               opstr,
               YASL_TYPE_NAMES[a.type],
               YASL_TYPE_NAMES[b.type]);
        exit(EXIT_FAILURE);
    }
}

int64_t int_add(int64_t left, int64_t right) {
    return left + right;
}

double float_add(double left, double right) {
    return left + right;
}

int64_t int_sub(int64_t left, int64_t right) {
    return left - right;
}

double float_sub(double left, double right) {
    return left - right;
}

int64_t int_mul(int64_t left, int64_t right) {
    return left * right;
}

double float_mul(double left, double right) {
    return left * right;
}

int64_t int_pow(int64_t left, int64_t right) {
    return (int64_t)pow(left, right);
}

void vm_num_binop(
        struct VM *vm, int64_t (*int_op)(int64_t, int64_t),
        double (*float_op)(double, double),
        const char *const opstr,
        const char *const overload_name) {
    struct YASL_Object right = vm_pop(vm);
    struct YASL_Object left = vm_pop(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64)) {
        vm_push(vm, YASL_INT(int_op(left.value.ival, right.value.ival)));
    } else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        vm_push(vm, YASL_FLOAT(float_op(left.value.dval, right.value.dval)));
    } else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_INT64)) {
        vm_push(vm, YASL_FLOAT(float_op(left.value.dval, right.value.ival)));
    } else if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        vm_push(vm, YASL_FLOAT(float_op(left.value.ival, right.value.dval)));
    } else if (yasl_type_equals(left.type, Y_TABLE)) {
        struct YASL_Object *key = YASL_String(str_new_sized_from_mem(0, strlen(overload_name), overload_name));
        struct YASL_Object *result = ht_search(left.value.mval, *key);
        if (result && yasl_type_equals(result->type, Y_FN)) {
            vm->sp += 2;

            int offset = 2;
            int addr = result->value.ival;
            struct YASL_Object a = ((struct YASL_Object) {offset, vm->fp});
            struct YASL_Object b = ((struct YASL_Object) {offset, vm->pc});
            vm_push(vm, a);  // store previous frame ptr;
            vm_push(vm, b);  // store pc addr
            vm->fp = vm->sp;

            vm->sp += offset + 1; // + 2
            vm->pc = addr + 2;
        } else {
            printf("TypeError: %s is not callable (type %s).\n", overload_name, YASL_TYPE_NAMES[result->type]);
        }
    } else {
        printf("TypeError: %s not supported for operands of types %s and %s.\n",
               opstr,
               YASL_TYPE_NAMES[left.type],
               YASL_TYPE_NAMES[right.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_fdiv(struct VM *vm) {
    char *overload_name = OP_BIN_FDIV;
    struct YASL_Object right = vm_pop(vm);
    struct YASL_Object left = vm_pop(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64)) {
        vm_push(vm, YASL_FLOAT((double)left.value.ival / (double)right.value.ival));
    }
    else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        vm_push(vm, YASL_FLOAT(left.value.dval / right.value.dval));
    }
    else if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        vm_push(vm, YASL_FLOAT((double)left.value.ival / right.value.dval));
    }
    else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_INT64)) {
        vm_push(vm, YASL_FLOAT(left.value.dval / (double)right.value.ival));
    }  else if (yasl_type_equals(left.type, Y_TABLE)) {
        struct YASL_Object *key = YASL_String(str_new_sized_from_mem(0, strlen(overload_name), overload_name));
        struct YASL_Object *result = ht_search(left.value.mval, *key);
        if (result && yasl_type_equals(result->type, Y_FN)) {
            vm->sp += 2;

            int offset = 2;
            int addr = result->value.ival;
            struct YASL_Object a = ((struct YASL_Object) {offset, vm->fp});
            struct YASL_Object b = ((struct YASL_Object) {offset, vm->pc});
            vm_push(vm, a);  // store previous frame ptr;
            vm_push(vm, b);  // store pc addr
            vm->fp = vm->sp;

            vm->sp += offset + 1; // + 2
            vm->pc = addr + 2;
        } else {
            printf("TypeError: %s is not callable (type %s).\n", overload_name, YASL_TYPE_NAMES[result->type]);
        }
    } else {
        printf("TypeError: / not supported for operands of types %s and %s.\n",
               YASL_TYPE_NAMES[left.type],
               YASL_TYPE_NAMES[right.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_pow(struct VM *vm) {
    struct YASL_Object right = vm_pop(vm);
    struct YASL_Object left  = vm_peek(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64) && right.value.ival < 0) {
        vm_pop(vm);
        vm_push(vm, YASL_FLOAT(pow(left.value.ival, right.value.ival)));
    } else {
        vm->sp++;
        vm_num_binop(vm, &int_pow, &pow, "**", OP_BIN_POWER);
    }
}

int64_t bnot(int64_t val) {
    return ~val;
}

void vm_int_unop(struct VM *vm, int64_t (*op)(int64_t), char *opstr) {
    struct YASL_Object a = PEEK(vm);
    if (yasl_type_equals(a.type, Y_INT64)) {
        PEEK(vm).value.ival = op(a.value.ival);
        return;
    } else {
        printf("TypeError: %s not supported for operand of type %s.\n",
               opstr,
               YASL_TYPE_NAMES[a.type]);
        exit(EXIT_FAILURE);
    }
}

int64_t int_neg(int64_t expr) {
    return -expr;
}

double float_neg(double expr) {
    return -expr;
}

void vm_num_unop(struct VM *vm, int64_t (*int_op)(int64_t), double (*float_op)(double), char *opstr) {
    struct YASL_Object expr = vm_pop(vm);
    if (yasl_type_equals(expr.type, Y_INT64)) {
        vm_push(vm, YASL_INT(int_op(expr.value.ival)));
    } else if (yasl_type_equals(expr.type, Y_FLOAT64)) {
        vm_push(vm, YASL_FLOAT(float_op(expr.value.dval)));
    } else {
        printf("TypeError: %s not supported for operand of type %s.\n",
               opstr,
               YASL_TYPE_NAMES[expr.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_run(struct VM *vm){
    while (1) {
        unsigned char opcode = NCODE(vm);        // fetch
        signed char offset;
        uint64_t big_offset, size;
        int64_t addr;
        struct YASL_Object a, b, v;
        int64_t c;
        double d;
        void* ptr;
        //printf("----------------\nvm->sp, vm->fp, vm->pc, opcode: %d, %d, %d, 0x%x\n", vm->sp, vm->fp, vm->pc, opcode);
        // printf("vm->pc, opcode: %x, %x\n", vm->pc - vm->pc0, opcode);
        //printf("pc: %d\n\n", vm->pc);
        //print(vm->stack[vm->sp]);
        //puts("\n");
        /* printf("tpye is: %s\n", YASL_TYPE_NAMES[PEEK(vm).type]);
        print(PEEK(vm));
         */
        //print(vm->globals[6]);
        //printf("\n%p\n", (void*)vm->globals[6].value.sval->str);
        //printf("%p\n", (void*)vm->globals[6].value.sval);
        switch (opcode) {   // decode
            case HALT: return;  // stop the program
            case NOP: puts("Slide"); break;    // pass
            case ICONST_M1:     // TODO: make sure no changes to opcodes ruin this
            case ICONST_0:
            case ICONST_1:
            case ICONST_2:
            case ICONST_3:
            case ICONST_4:
            case ICONST_5:
                vm_push(vm, YASL_INT(opcode - 0x04));
                break;
            case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
            case DCONST_1:
            case DCONST_2:
                vm_push(vm, YASL_FLOAT(opcode - 0x0B));
                break;
            case DCONST_N:
                vm_push(vm, YASL_FLOAT(0.0 / 0.0));
                break;
            case DCONST_I:
                vm_push(vm, YASL_FLOAT(1.0 / 0.0));
                break;
            case DCONST:        // constants have native endianness
                vm_push(vm, YASL_FLOAT(vm_read_double(vm)));
                break;
            case ICONST:        // constants have native endianness
                c = vm_read_int64_t(vm);
                vm_push(vm, YASL_INT(c));
                break;
            case BCONST_F:
            case BCONST_T:
                vm_push(vm, YASL_BOOL(opcode & 0x01));
                break;
            case NCONST:
                vm_push(vm, YASL_UNDEF());
                break;
            case FCONST:
                c = vm_read_int64_t(vm);
                vm_push(vm, YASL_FN(c));
                break;
            case BOR:
                vm_int_binop(vm, &bor, "|", OP_BIN_BAR);
                break;
            case BXOR:
                vm_int_binop(vm, &bxor, "^", OP_BIN_CARET);
                break;
            case BAND:
                vm_int_binop(vm, &band, "&", OP_BIN_AMP);
                break;
            case BANDNOT:
                vm_int_binop(vm, &bandnot, "&^", OP_BIN_AMPCARET);
                break;
            case BNOT:
                vm_int_unop(vm, &bnot, "^");
                break;
            case BSL:
                vm_int_binop(vm, &shift_left, "<<", OP_BIN_SHL);
                break;
            case BSR:
                vm_int_binop(vm, &shift_right, ">>", OP_BIN_SHR);
                break;
            case ADD:
                vm_num_binop(vm, &int_add, &float_add, "+", OP_BIN_PLUS);
                break;
            case MUL:
                vm_num_binop(vm, &int_mul, &float_mul, "*", OP_BIN_TIMES);
                break;
            case SUB:
                vm_num_binop(vm, &int_sub, &float_sub, "-", OP_BIN_MINUS);
                break;
            case FDIV:
                vm_fdiv(vm);   // handled differently because we always convert to float
                break;
            case IDIV:
                if (yasl_type_equals(vm_peek(vm).type, Y_INT64) && vm_peek(vm).value.ival == 0) {
                    vm_fdiv(vm);
                    break;
                }
                vm_int_binop(vm, &idiv, "//", OP_BIN_IDIV);
                break;
            case MOD:
                // TODO: handle undefined C behaviour for negative numbers.
                vm_int_binop(vm, &modulo, "%", OP_BIN_MOD);
                break;
            case EXP:
                vm_pow(vm);
                break;
            case NEG:
                vm_num_unop(vm, &int_neg, &float_neg, "-");
                break;
            case NOT:
                vm_push(vm, YASL_BOOL(isfalsey(vm_pop(vm))));
                break;
            case LEN:
                v = vm_pop(vm);
                if (yasl_type_equals(v.type, Y_STR)) {
                    vm_push(vm, YASL_INT(yasl_string_len(v.value.sval)));
                } else if (yasl_type_equals(v.type, Y_TABLE)) {
                    vm_push(vm, YASL_INT(v.value.mval->count));
                } else if (yasl_type_equals(v.type, Y_LIST)) {
                    vm_push(vm, YASL_INT(v.value.lval->count));
                } else {
                    printf("TypeError: # not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[v.type]);
                    return;
                }
                break;
            case CNCT:
                b = vm_pop(vm);
                a = vm_pop(vm);
                if (yasl_type_equals(a.type, Y_STR) && yasl_type_equals(b.type, Y_STR)) {
                    size = yasl_string_len(a.value.sval) + yasl_string_len(b.value.sval);
                    char *ptr = malloc(size);
                    memcpy(ptr, (a.value.sval)->str + a.value.sval->start, yasl_string_len(a.value.sval));
                    memcpy(ptr + yasl_string_len(a.value.sval), (b.value.sval)->str + b.value.sval->start, yasl_string_len(b.value.sval));
                    vm_push(vm, YASL_STR(str_new_sized(size, ptr)));
                    break;
                } else if (yasl_type_equals(a.type, Y_LIST) && yasl_type_equals(b.type, Y_LIST)) {
                    size = a.value.lval->count + b.value.lval->count;
                    ptr = ls_new_sized(size);
                    vm_push(vm, YASL_LIST(ptr));
                    int i;
                    for (i = 0; i < a.value.lval->count; i++) {
                        ls_append(ptr, a.value.lval->items[i]);
                    }
                    for (i = 0; i < b.value.lval->count; i++) {
                        ls_append(ptr, b.value.lval->items[i]);
                    }
                    break;
                }
                printf("TypeError: ~ not supported for operands of types %s and %s.\n",
                       YASL_TYPE_NAMES[a.type],
                       YASL_TYPE_NAMES[b.type]);
                return;
            case GT:
                b = vm_pop(vm);
                a = vm_pop(vm);
                if (a.type == Y_STR && b.type == Y_STR) {
                    vm_push(vm, YASL_BOOL(yasl_string_cmp(a.value.sval, b.value.sval) > 0));
                    break;
                }
                if ((a.type != Y_INT64 && a.type != Y_FLOAT64) ||
                    (b.type != Y_INT64 && b.type != Y_FLOAT64)) {
                    printf("TypeError: < and > not supported for operand of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                COMP(vm, a, b, GT, ">");
                break;
            case GE:
                b = vm_pop(vm);
                a = vm_pop(vm);
                if (a.type == Y_STR && b.type == Y_STR) {
                    //printf("%d\n", yasl_string_cmp(a.value.sval, b.value.sval));
                    vm_push(vm, YASL_BOOL(yasl_string_cmp(a.value.sval, b.value.sval) >= 0));
                    break;
                }
                if ((a.type != Y_INT64 && a.type != Y_FLOAT64) ||
                    (b.type != Y_INT64 && b.type != Y_FLOAT64)) {
                    printf("TypeError: <= and >= not supported for operand of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                COMP(vm, a, b, GE, ">=");
                break;
            case EQ:
                b = vm_pop(vm);
                a = vm_pop(vm);
                v = isequal(a, b);
                vm_push(vm, v);
                break;
            case ID:
                b = vm_pop(vm);
                a = vm_pop(vm);
                vm_push(vm, YASL_BOOL(yasl_type_equals(a.type, b.type) && a.value.ival == b.value.ival));
                break;
            case NEWSTR:
                addr = vm_read_int64_t(vm);
                memcpy(&size, vm->code + addr, sizeof(int64_t));
                addr += sizeof(int64_t);

                vm_push(vm, YASL_STR(str_new_sized_from_mem(addr, addr + size, vm->code)));
                break;
            case NEWTABLE: {
                struct YASL_Object *table = YASL_Table();
                Hash_t *ht = table->value.mval;
                while(PEEK(vm).type != Y_END) {
                    struct YASL_Object value = POP(vm);
                    struct YASL_Object key = POP(vm);
                    ht_insert(ht, key, value);
                }
                vm_pop(vm);
                vm_push(vm, *table);
                free(table);
                break;
            }
            case NEWLIST: {
                List_t *ls = ls_new();
                while (vm_peek(vm).type != Y_END) {
                    ls_append(ls, vm_pop(vm));
                }
                ls_reverse(ls);
                vm_pop(vm);
                vm_push(vm, YASL_LIST(ls));
                break;
            }
            case INITFOR:
                vm_push(vm, YASL_INT(0));
                vm_push(vm, YASL_INT(vm->lp));
                vm->lp = vm->sp - 2;
                break;
            case ENDCOMP:
                a = vm_pop(vm);
                vm->lp = vm_pop(vm).value.ival;
                vm_pop(vm);
                vm_pop(vm);
                vm_push(vm, a);
                break;
            case ENDFOR:
                vm->lp = vm_pop(vm).value.ival;
                vm_pop(vm);
                vm_pop(vm);
                break;
            case ITER_1:
                //print(vm->stack[vm->lp]);
                switch (vm->stack[vm->lp].type) {
                    case Y_LIST:
                        if (vm->stack[vm->lp].value.lval->count <= vm->stack[vm->lp + 1].value.ival) {
                            vm_push(vm, YASL_BOOL(0));
                        } else {
                            vm_push(vm, vm->stack[vm->lp].value.lval->items[vm->stack[vm->lp + 1].value.ival++]);
                            //print(vm->stack[vm->sp]);
                            vm_push(vm, YASL_BOOL(1));
                        }
                        break;
                    case Y_TABLE:
                        while ((vm->stack[vm->lp].value.mval->items[vm->stack[vm->lp + 1].value.ival] == &TOMBSTONE ||
                                vm->stack[vm->lp].value.mval->items[vm->stack[vm->lp + 1].value.ival] == NULL
                        ) && vm->stack[vm->lp].value.mval->size > vm->stack[vm->lp + 1].value.ival) {
                            vm->stack[vm->lp + 1].value.ival++;
                        }
                        if (vm->stack[vm->lp].value.mval->size <= vm->stack[vm->lp + 1].value.ival) {
                            vm_push(vm, YASL_BOOL(0));
                            break;
                        }
                        vm_push(vm, *vm->stack[vm->lp].value.mval->items[vm->stack[vm->lp + 1].value.ival++]->key);
                        vm_push(vm, YASL_BOOL(1));
                        break;
                    default:
                        printf("object of type %s is not iterable.\n", YASL_TYPE_NAMES[vm->stack[vm->lp].type]);
                        exit(EXIT_FAILURE);
                }
                break;
            case ITER_2:
                exit(1);
            case END:
                vm_push(vm, YASL_END()); //(struct YASL_Object) {.type = Y_END });
                break;
            case DUP: {
                a = vm_peek(vm);
                vm_push(vm, a);
                break;
            }
            case SWAP:
                a = vm->stack[vm->sp];
                b = vm->stack[vm->sp-1];
                vm->stack[vm->sp-1] = a;
                vm->stack[vm->sp] = b;
                break;
            case GOTO:
                c = vm_read_int64_t(vm);
                vm->pc = c + (vm->pc < vm->pc0 ? 16 : vm->pc0);
                break;
            case BR_8:
                c = vm_read_int64_t(vm);
                vm->pc += c;
                break;
            case BRF_8:
                c = vm_read_int64_t(vm);
                v = vm_pop(vm);
                if (isfalsey(v)) vm->pc += c;
                break;
            case BRT_8:
                c = vm_read_int64_t(vm);
                v = vm_pop(vm);
                if (!(isfalsey(v))) vm->pc += c;
                break;
            case BRN_8:
                c = vm_read_int64_t(vm);
                v = vm_pop(vm);
                if (v.type != Y_UNDEF) vm->pc += c;
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
                vm_push(vm, vm->stack[vm->fp+offset+3]);
                break;
            case LSTORE_1:
                offset = NCODE(vm);
                dec_ref(&vm->stack[vm->fp+offset+3]);
                vm->stack[vm->fp+offset+3] = vm_pop(vm);
                inc_ref(&vm->stack[vm->fp+offset+3]);
                break;
            case INIT_CALL:
                if (vm_peek(vm).type != Y_FN && vm_peek(vm).type != Y_CFN) {
                    printf("TypeError: %s is not callable.", YASL_TYPE_NAMES[PEEK(vm).type]);
                    printf("`");
                    print(PEEK(vm));
                    printf("`\n");
                    exit(EXIT_FAILURE);
                }

                vm_push(vm, YASL_INT(vm->fp));
                vm_push(vm, YASL_INT(vm->fp));
                vm->fp = vm->sp - 2;
                break;

            case CALL:
                if (vm->stack[vm->fp].type == Y_FN) {
                    vm->stack[vm->fp+1].value.ival = vm->pc;

                    while (vm->sp - (vm->fp + 2) < vm->code[vm->stack[vm->fp].value.ival]) {
                        vm_push(vm, YASL_UNDEF());
                    }

                    while (vm->sp - (vm->fp + 2) > vm->code[vm->stack[vm->fp].value.ival]) {
                        vm_pop(vm);
                    }

                    vm->sp += vm->code[vm->stack[vm->fp].value.ival + 1];

                    vm->pc = vm->stack[vm->fp].value.ival + 2;
                } else if (vm->stack[vm->fp].type == Y_CFN) {
                    while (vm->sp - (vm->fp + 2) < vm->stack[vm->fp].value.cval->num_args) {
                        vm_push(vm, YASL_UNDEF());
                    }

                    while (vm->sp - (vm->fp + 2) > vm->stack[vm->fp].value.cval->num_args) {
                        vm_pop(vm);
                    }

                    struct YASL_State *state = malloc(sizeof(struct YASL_State));
                    state->vm = vm;
                    if (vm->stack[vm->fp].value.cval->value(state)) {
                        printf("ERROR: invalid argument type(s) to builtin function.\n");
                        return;
                    };
                    free(state);
                }
                break;
            case RET:
                // TODO: handle multiple returns
                v = vm_pop(vm);
                vm->sp = vm->fp + 2;
                vm->fp = vm_pop(vm).value.ival;
                vm->pc = vm_pop(vm).value.ival;
                vm_pop(vm);
                vm_push(vm, v);
                break;
            case GET: {
                struct YASL_State *S = malloc(sizeof(struct YASL_State));
                S->vm = vm;
                vm->sp--;
                int index = vm_peek(vm).type;
                if (yasl_type_equals(vm_peek(vm).type, Y_LIST)) {
                    vm->sp++;
                    if (!list___get(S)) {
                        free(S);
                        break;
                    }
                } else if (yasl_type_equals(vm_peek(vm).type, Y_TABLE)) {
                    vm->sp++;
                    if (!table___get(S)) {
                        free(S);
                        break;
                    }
                } else {
                    vm->sp++;
                    //vm_pop(vm);
                }

                struct YASL_Object key = vm_pop(vm);
                struct YASL_Object *result = ht_search(vm->builtins_htable[index], key);
                if (result == NULL) {
                    vm_push(vm, YASL_UNDEF());
                    // printf("%s\n", YASL_TYPE_NAMES[index]);
                    // puts("Not found.");
                    // exit(1);
                } else {
                    vm_push(vm, *result);
                }
                free(S);
                break;
            }
            case SET: {
                struct YASL_State *S = malloc(sizeof(struct YASL_State));
                S->vm = vm;
                vm->sp -= 2;
                if (yasl_type_equals(vm_peek(vm).type, Y_LIST)) {
                    vm->sp += 2;
                    list___set(S);
                } else if (yasl_type_equals(vm_peek(vm).type, Y_TABLE)) {
                    vm->sp += 2;
                    table___set(S);
                } else {
                    vm->sp += 2;
                    printf("object of type %s is immutable.", YASL_TYPE_NAMES[vm_peek(vm).type]);
                    exit(EXIT_FAILURE);
                }
                free(S);
                break;
            }
            case RCALL_8:
                offset = NCODE(vm);
                int i;
                for (i = 0; i < offset; i++) {
                    vm->stack[vm->fp - 2 - i] = vm->stack[vm->sp - i];
                }
                addr = vm_read_int64_t(vm);
                offset = NCODE(vm);
                vm->sp = vm->fp + offset;
                vm->pc = addr;
                break;
            /*case RET:
                v = POP(vm);
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
                printf("ERROR UNKNOWN OPCODE: %x\n", opcode);
                return;
        }
    }
}
