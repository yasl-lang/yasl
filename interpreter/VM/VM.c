#include <stdlib.h>
#include <memory.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "VM.h"
#include "interpreter/builtins/builtins.h"
#include <functions.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <hashtable/hashtable.h>
#include <color.h>

static LoopStack *loopstack_new(void) {
    LoopStack *ls = malloc(sizeof(LoopStack));
    ls->indices = malloc(sizeof(int64_t)*6);
    ls->stack = calloc(6, sizeof(YASL_Object));
    ls->sp = -1;
    return ls;
}

static Hash_t **builtins_htable_new(void) {
    Hash_t **ht = malloc(sizeof(Hash_t*) * NUM_TYPES);
    ht[Y_FLOAT64] = float64_builtins();
    ht[Y_INT64] = int64_builtins();
    ht[Y_BOOL] = bool_builtins();
    ht[Y_STR] = str_builtins();
    ht[Y_LIST] = list_builtins();
    ht[Y_TABLE] = table_builtins();
    ht[Y_FILE] = file_builtins();

    return ht;
}

VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize) {      // total params size required to perform a program operations
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc0;
    vm->pc0 = vm->pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->globals = calloc(sizeof(YASL_Object), datasize);
    vm->num_globals = datasize;
    vm->globals[0] = (YASL_Object) {Y_FILE, (int64_t)stdin};
    vm->globals[1] = (YASL_Object) {Y_FILE, (int64_t)stdout};
    vm->globals[2] = (YASL_Object) {Y_FILE, (int64_t)stderr};
    vm->globals[3] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_open};
    vm->globals[4] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_popen};
    vm->globals[5] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_input};

    vm->stack = calloc(sizeof(YASL_Object), STACK_SIZE);

    vm->builtins_htable = builtins_htable_new();
    vm->loopstack = loopstack_new();
    return vm;
}

void vm_del(VM *vm) {
    for (int i = 0; i < STACK_SIZE; i++) dec_ref(&vm->stack[i]);
    for (int i = 0; i < vm->num_globals; i++) dec_ref(&vm->globals[i]);

    free(vm->globals);                   // TODO: free these properly
    free(vm->stack);                     // TODO: free these properly

    free(vm->code);

    //ht_del_string_int(vm->builtins_htable[0]);
    ht_del_string_int(vm->builtins_htable[Y_FLOAT64]);
    ht_del_string_int(vm->builtins_htable[Y_INT64]);
    ht_del_string_int(vm->builtins_htable[Y_BOOL]);
    ht_del_string_int(vm->builtins_htable[Y_STR]);
    ht_del_string_int(vm->builtins_htable[Y_LIST]);
    ht_del_string_int(vm->builtins_htable[Y_TABLE]);
    ht_del_string_int(vm->builtins_htable[Y_FILE]);
    free(vm->builtins_htable);

    for (int i = 0; i < 6; i++) dec_ref(&vm->loopstack->stack[i]);
    free(vm->loopstack->stack);
    free(vm->loopstack->indices);
    free(vm->loopstack);

    free(vm);
}

#define LOOPSTACK_PEEK(vm) vm->loopstack->stack[vm->loopstack->sp]
#define LOOPSTACK_INDEX(vm) vm->loopstack->indices[vm->loopstack->sp]

void vm_loopstack_push(VM *vm, YASL_Object val) {
    vm->loopstack->sp++;
    dec_ref(vm->loopstack->stack + vm->loopstack->sp);
    vm->loopstack->stack[vm->loopstack->sp] = val;
    inc_ref(vm->loopstack->stack + vm->loopstack->sp);
}

void vm_push(VM *vm, YASL_Object val) {
    vm->sp++;
    dec_ref(vm->stack + vm->sp);
    vm->stack[vm->sp] = val;
    inc_ref(vm->stack + vm->sp);
}

YASL_Object vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

YASL_Object vm_peek(VM *vm) {
    return vm->stack[vm->sp];
}

int64_t vm_read_int64_t(VM *vm) {
    int64_t val;
    memcpy(&val, vm->code + vm->pc, sizeof( int64_t));
    vm->pc += sizeof (int64_t);
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

void vm_int_binop(VM *vm, int64_t (*op)(int64_t, int64_t), char *opstr) {
    YASL_Object b = POP(vm);
    YASL_Object a = PEEK(vm);
    if (yasl_type_equals(a.type, Y_INT64) && yasl_type_equals(b.type, Y_INT64)) {
        vm_push(vm, YASL_Integer(op(a.value.ival, b.value.ival)));
        return;
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

void vm_num_binop(VM *vm, int64_t (*int_op)(int64_t, int64_t), double (*float_op)(double, double), char *opstr) {
    YASL_Object right = vm_pop(vm);
    YASL_Object left = vm_peek(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64)) {
        PEEK(vm).value.ival = int_op(left.value.ival, right.value.ival);
    } else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        PEEK(vm).value.dval = float_op(left.value.dval, right.value.dval);
    } else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_INT64)) {
        PEEK(vm).value.dval = float_op(left.value.dval, right.value.ival);
    } else if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        PEEK(vm).type = Y_FLOAT64;
        PEEK(vm).value.dval = float_op(left.value.ival, right.value.dval);
    } else {
        printf("TypeError: %s not supported for operands of types %s and %s.\n",
               opstr,
               YASL_TYPE_NAMES[left.type],
               YASL_TYPE_NAMES[right.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_fdiv(VM *vm) {
    YASL_Object right = vm_pop(vm);
    YASL_Object left = vm_peek(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64)) {
        vm_push(vm, YASL_Float((double)left.value.ival / (double)right.value.ival));
    }
    else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        PEEK(vm).value.dval = left.value.dval / right.value.dval;
    }
    else if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_FLOAT64)) {
        PEEK(vm).value.dval = (double)left.value.ival / right.value.dval;
        PEEK(vm).type = Y_FLOAT64;
    }
    else if (yasl_type_equals(left.type, Y_FLOAT64) && yasl_type_equals(right.type, Y_INT64)) {
        PEEK(vm).value.dval = left.value.dval / (double)right.value.ival;
    }
    else {
        printf("TypeError: / not supported for operands of types %s and %s.\n",
               YASL_TYPE_NAMES[left.type],
               YASL_TYPE_NAMES[right.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_pow(VM *vm) {
    YASL_Object right = vm_pop(vm);
    YASL_Object left  = vm_peek(vm);
    if (yasl_type_equals(left.type, Y_INT64) && yasl_type_equals(right.type, Y_INT64) && right.value.ival < 0) {
        PEEK(vm).value.dval = pow(left.value.ival, right.value.ival);
        PEEK(vm).type = Y_FLOAT64;
    } else {
        vm->sp++;
        vm_num_binop(vm, &int_pow, &pow, "**");
    }
}

int64_t bnot(int64_t val) {
    return ~val;
}

void vm_int_unop(VM *vm, int64_t (*op)(int64_t), char *opstr) {
    YASL_Object a = PEEK(vm);
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

void vm_num_unop(VM *vm, int64_t (*int_op)(int64_t), double (*float_op)(double), char *opstr) {
    YASL_Object expr = vm_peek(vm);
    if (yasl_type_equals(expr.type, Y_INT64)) {
        PEEK(vm).value.ival = int_op(expr.value.ival);
    } else if (yasl_type_equals(expr.type, Y_FLOAT64)) {
        PEEK(vm).value.dval = float_op(expr.value.dval);
    } else {
        printf("TypeError: %s not supported for operand of type %s.\n",
               opstr,
               YASL_TYPE_NAMES[expr.type]);
        exit(EXIT_FAILURE);
    }
}

void vm_run(VM *vm){
    while (1) {
        unsigned char opcode = NCODE(vm);        // fetch
        signed char offset;
        uint64_t big_offset, size;
        int64_t addr;
        YASL_Object a, b, v;
        int64_t c;
        double d;
        void* ptr;
        // printf("vm->pc, opcode: %x, %x\n", vm->pc - vm->pc0, opcode);
        //printf("pc: %d\n\n", vm->pc);
        //print(vm->stack[vm->sp]);
        //puts("\n");
        /* printf("tpye is: %s\n", YASL_TYPE_NAMES[PEEK(vm).type]);
        print(PEEK(vm));
         */
        switch (opcode) {   // decode
            case HALT: return;  // stop the program
            case NOP: break;    // pass
            case ICONST_M1:     // TODO: make sure no changes to opcodes ruin this
            case ICONST_0:
            case ICONST_1:
            case ICONST_2:
            case ICONST_3:
            case ICONST_4:
            case ICONST_5:
                vm_push(vm, YASL_Integer(opcode - 0x04));
                break;
            case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
            case DCONST_1:
            case DCONST_2:
                vm_push(vm, YASL_Float(opcode - 0x0B));
                break;
            case DCONST_N:
                vm_push(vm, YASL_Float(0.0 / 0.0));
                break;
            case DCONST_I:
                vm_push(vm, YASL_Float(1.0 / 0.0));
                break;
            case DCONST:        // constants have native endianness
                c = vm_read_int64_t(vm);
                vm_push(vm, (YASL_Object){Y_FLOAT64, c, NULL});
                break;
            case ICONST:        // constants have native endianness
                c = vm_read_int64_t(vm);
                vm_push(vm, YASL_Integer(c));
                break;
            case BCONST_F:
            case BCONST_T:
                vm_push(vm, YASL_Boolean(opcode & 0x01));
                break;
            case NCONST:
                vm_push(vm, YASL_Undef());
                break;
            case FCONST:
                c = vm_read_int64_t(vm);
                vm_push(vm, (YASL_Object) {Y_FN, c});
                break;
            case BOR:
                vm_int_binop(vm, &bor, "|");
                break;
            case BXOR:
                vm_int_binop(vm, &bxor, "^");
                break;
            case BAND:
                vm_int_binop(vm, &band, "&");
                break;
            case BNOT:
                vm_int_unop(vm, &bnot, "^");
                break;
            case BSL:
                vm_int_binop(vm, &shift_left, "<<");
                break;
            case BSR:
                vm_int_binop(vm, &shift_right, ">>");
                break;
            case ADD:
                vm_num_binop(vm, &int_add, &float_add, "+");
                break;
            case MUL:
                vm_num_binop(vm, &int_mul, &float_mul, "*");
                break;
            case SUB:
                vm_num_binop(vm, &int_sub, &float_sub, "-");
                break;
            case FDIV:
                vm_fdiv(vm);   // handled differently because we always convert to float
                break;
            case IDIV:
                if (yasl_type_equals(vm_peek(vm).type, Y_INT64) && vm_peek(vm).value.ival == 0) {
                    vm_fdiv(vm);
                    break;
                }
                vm_int_binop(vm, &idiv, "//");
                break;
            case MOD:
                // TODO: handle undefined C behaviour for negative numbers.
                vm_int_binop(vm, &modulo, "%");
                break;
            case EXP:
                vm_pow(vm);
                break;
            case NEG:
                vm_num_unop(vm, &int_neg, &float_neg, "-");
                break;
            case NOT:
                vm_push(vm, YASL_Boolean(isfalsey(vm_pop(vm))));
                break;
            case LEN:
                v = vm->stack[vm->sp];
                if (yasl_type_equals(v.type, Y_STR)) {
                    vm_push(vm, YASL_Integer(yasl_string_len(v.value.sval)));
                } else if (yasl_type_equals(v.type, Y_TABLE)) {
                    vm_push(vm, YASL_Integer(v.value.mval->count));
                } else if (yasl_type_equals(v.type, Y_LIST)) {
                    vm_push(vm, YASL_Integer(v.value.lval->count));
                } else {
                    printf("TypeError: # not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[v.type]);
                    return;
                }
                break;
            case CNCT:
                b = vm_pop(vm);
                a = vm_peek(vm);
                if (yasl_type_equals(a.type, Y_STR) && yasl_type_equals(b.type, Y_STR)) {
                    size = yasl_string_len(a.value.sval) + yasl_string_len(b.value.sval);
                    char *ptr = malloc(size);
                    memcpy(ptr, (a.value.sval)->str + a.value.sval->start, yasl_string_len(a.value.sval));
                    memcpy(ptr + yasl_string_len(a.value.sval), (b.value.sval)->str + b.value.sval->start, yasl_string_len(b.value.sval));
                    vm->stack[vm->sp].value.sval = str_new_sized(size, ptr);
                    break;
                } else if (yasl_type_equals(a.type, Y_LIST) && yasl_type_equals(b.type, Y_LIST)) {
                    size = a.value.lval->count + b.value.lval->count;
                    ptr = ls_new_sized(size);
                    vm->stack[vm->sp].value.lval = ptr;
                    // TODO: optimise this.
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
                printf(K_RED);
                print(a);
                print(b);
                printf(K_END);
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
                a = vm_peek(vm);
                vm_push(vm, isequal(a, b));
                //vm->stack[vm->sp] = (vm, isequal(a, b));
                break;
            case ID:
                b = vm_pop(vm);
                a = vm_peek(vm);
                vm->stack[vm->sp].value.ival = yasl_type_equals(a.type, b.type) && a.value.ival == b.value.ival;
                vm->stack[vm->sp].type = Y_BOOL;
                break;
            case NEWSTR:
                addr = vm_read_int64_t(vm);
                memcpy(&size, vm->code + addr, sizeof(int64_t));
                addr += sizeof(int64_t);

                vm_push(vm, YASL_String(str_new_sized_from_mem(addr, addr + size, vm->code)));
                break;
            case NEWTABLE: {
                Hash_t *ht = ht_new();
                while(PEEK(vm).type != Y_END) {
                    YASL_Object value = POP(vm);
                    YASL_Object key = POP(vm);
                    ht_insert(ht, key, value);
                }
                vm_pop(vm);
                vm_push(vm, (YASL_Object) { .type = Y_TABLE, .value.mval = ht });
                break;
            }
            case NEWLIST: {
                List_t *ls = ls_new();
                while (vm_peek(vm).type != Y_END) {
                    ls_append(ls, vm_pop(vm));
                }
                ls_reverse(ls);
                vm_pop(vm);
                vm_push(vm, YASL_List(ls));
                break;
            }
            case INITFOR:
                vm_loopstack_push(vm, POP(vm));
                LOOPSTACK_INDEX(vm) = 0;
                break;
            case ENDFOR:
                vm->loopstack->sp--;
                break;
            case ITER_1:
                switch (LOOPSTACK_PEEK(vm).type) {
                    case Y_LIST:
                        if (LOOPSTACK_PEEK(vm).value.lval->count <= LOOPSTACK_INDEX(vm)) {
                            BPUSH(vm, 0);
                        } else {
                            vm_push(vm, LOOPSTACK_PEEK(vm).value.lval->items[LOOPSTACK_INDEX(vm)++]); //.value.lval->items;
                            BPUSH(vm, 1);
                        }
                        break;
                    case Y_TABLE:
                        while ((LOOPSTACK_PEEK(vm).value.mval->items[LOOPSTACK_INDEX(vm)] == &TOMBSTONE ||
                                LOOPSTACK_PEEK(vm).value.mval->items[LOOPSTACK_INDEX(vm)] == NULL) &&
                                LOOPSTACK_PEEK(vm).value.mval->size > LOOPSTACK_INDEX(vm)) {
                            LOOPSTACK_INDEX(vm)++;
                        }
                        if (LOOPSTACK_PEEK(vm).value.mval->size <= LOOPSTACK_INDEX(vm)) {
                            BPUSH(vm, 0);
                            break;
                        }
                        vm_push(vm, *LOOPSTACK_PEEK(vm).value.mval->items[LOOPSTACK_INDEX(vm)++]->key); //.value.lval->items;
                        BPUSH(vm, 1);
                        break;
                    default:
                        printf("object of type %s is not iterable.\n", YASL_TYPE_NAMES[LOOPSTACK_PEEK(vm).type]);
                        exit(EXIT_FAILURE);
                }
                break;
            case ITER_2:
                exit(1);
            case END:
                vm_push(vm, (YASL_Object) {.type = Y_END });
                break;
            case DUP:
                vm_push(vm, vm_peek(vm));
                //vm->stack[vm->sp+1] = vm->stack[vm->sp];
                //vm->sp++;
                break;
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
                //printf(K_MAG "-%x (%x)" K_END "\n", -c, c);
                vm->pc += c;
                //printf("old pc: %x, new pc: %x\n", vm->pc - c - vm->pc0, vm->pc - vm->pc0);
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
                // vm->stack[++vm->sp] = vm->globals[addr];  // load value from memory of the provided addr
                break;
            case GSTORE_1:
                addr = vm->code[vm->pc++];
                //puts(K_BLU "object is");
                //print(vm->globals[addr]);
                //printf(K_END);
                //print(vm->globals[addr]);
                dec_ref(&vm->globals[addr]);
                vm->globals[addr] = vm->stack[vm->sp--];
                //print(vm->globals[addr]);
                inc_ref(&vm->globals[addr]);
                break;
            case LLOAD_1:
                offset = NCODE(vm);
                vm->stack[++vm->sp] = vm->stack[vm->fp-offset-2];
                break;
            case LSTORE_1:
                offset = NCODE(vm);
                vm->stack[vm->fp-offset-2] = vm->stack[vm->sp--];
                break;
            case CALL_8:
                if (yasl_type_equals(vm_peek(vm).type, Y_FN)) {
                    offset = NCODE(vm);
                    addr = POP(vm).value.ival;
                    vm_push(vm, ((YASL_Object) {offset, vm->fp}));  // store previous frame ptr;
                    vm_push(vm, ((YASL_Object) {offset, vm->pc}));  // store pc addr
                    vm->fp = vm->sp;
                    if (vm->code[addr] != offset) {
                        puts("CallError: wrong number params.");
                    }
                    offset = vm->code[addr+1];
                    vm->sp += offset + 1; // + 2
                    vm->pc = addr + 2;
                    break;
                } else if (yasl_type_equals(vm_peek(vm).type, Y_BFN)) {
                    offset = NCODE(vm);
                    addr = POP(vm).value.ival;
                    if (((int (*)(VM*))addr)(vm)) {
                        printf("ERROR: invalid argument type(s) to builtin function.\n");
                        return;
                    };
                    break;
                } else {
                    printf("TypeError: %s is not callable.", YASL_TYPE_NAMES[PEEK(vm).type]);
                    exit(EXIT_FAILURE);
                }
            case GET:
            {
                int index = PEEK(vm).type;
                if (yasl_type_equals(vm_peek(vm).type, Y_LIST)) {
                    if (!list___get(vm)) break;
                } else if (yasl_type_equals(vm_peek(vm).type, Y_TABLE)) {
                    if (!table___get(vm)) break;
                } else {
                    POP(vm);
                }

                YASL_Object key = POP(vm);
                YASL_Object *result = ht_search(vm->builtins_htable[index], key);
                if (result == NULL) {
                    printf("%s\n", YASL_TYPE_NAMES[index]);
                    puts("Not found.");
                    exit(1);
                } else {
                    vm_push(vm, *result);
                }
                break;
            }
            case SET:
                if (yasl_type_equals(vm_peek(vm).type, Y_LIST)) {
                    list___set(vm);
                } else if (yasl_type_equals(vm_peek(vm).type, Y_TABLE)) {
                    table___set(vm);
                } else {
                    printf("object of type %s is immutable.", YASL_TYPE_NAMES[PEEK(vm).type]);
                    exit(EXIT_FAILURE);
                }
                break;
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
            case RET:
                v = POP(vm);
                a = vm->stack[vm->fp];
                b = vm->stack[vm->fp-1];
                vm->pc = a.value.ival;
                vm->sp = vm->fp - a.type - 2;
                vm->fp = b.value.ival;
                vm_push(vm, v);
                break;
            case POP:
                --vm->sp;
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
