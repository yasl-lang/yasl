#include <stdlib.h>
#include <memory.h>
#include <interpreter/YASL_Object/YASL_Object.h>
#include "VM.h"
#include "interpreter/builtins/builtins.h"
#include <functions.h>
#include <interpreter/YASL_string/YASL_string.h>
#include <hashtable/hashtable.h>

static LoopStack *loopstack_new(void) {
    LoopStack *ls = malloc(sizeof(LoopStack));
    ls->indices = malloc(sizeof(int64_t)*6);
    ls->stack = malloc(sizeof(int64_t)*6);
    ls->sp = -1;
    return ls;
}

static Hash_t **builtins_htable_new(void) {
    Hash_t **ht = malloc(sizeof(Hash_t*) * NUM_TYPES);
    ht[1] = float64_builtins();
    ht[2] = int64_builtins();
    ht[3] = bool_builtins();
    ht[4] = str_builtins();
    ht[5] = list_builtins();
    ht[6] = table_builtins();
    ht[7] = file_builtins();

    return ht;
}

VM* vm_new(unsigned char *code,    // pointer to bytecode
           int pc0,             // address of instruction to be executed first -- entrypoint
           int datasize) {      // total locals size required to perform a program operations
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc0;
    vm->pc0 = vm->pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->globals = malloc(sizeof(YASL_Object) * datasize);
    vm->globals[0] = (YASL_Object) {Y_FILE, (int64_t)stdin};
    vm->globals[1] = (YASL_Object) {Y_FILE, (int64_t)stdout};
    vm->globals[2] = (YASL_Object) {Y_FILE, (int64_t)stderr};
    vm->globals[3] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_open};
    vm->globals[4] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_popen};
    vm->globals[5] = (YASL_Object) {Y_BFN,  (int64_t)&yasl_input};

    vm->stack = malloc(sizeof(YASL_Object) * STACK_SIZE);

    vm->builtins_htable = builtins_htable_new();
    vm->loopstack = loopstack_new();
    return vm;
}

void vm_del(VM *vm){
    free(vm->globals);                   // TODO: free these properly
    free(vm->stack);                     // TODO: free these properly

    //ht_del_string_int(vm->builtins_htable[0]);
    ht_del_string_int(vm->builtins_htable[1]);
    ht_del_string_int(vm->builtins_htable[2]);
    ht_del_string_int(vm->builtins_htable[3]);
    ht_del_string_int(vm->builtins_htable[4]);
    ht_del_string_int(vm->builtins_htable[5]);
    ht_del_string_int(vm->builtins_htable[6]);
    ht_del_string_int(vm->builtins_htable[7]);
    free(vm->builtins_htable);

    free(vm->loopstack->stack);
    free(vm->loopstack->indices);

    free(vm);
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
        /*printf("\nopcode: %x\n", opcode);
        printf("tpye is: %s\n", YASL_TYPE_NAMES[PEEK(vm).type]);
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
                vm->stack[++vm->sp].type = Y_INT64;
                vm->stack[vm->sp].value.ival  = opcode - 0x04;
                break;
            case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
            case DCONST_1:
            case DCONST_2:
                vm->stack[++vm->sp].type = Y_FLOAT64;
                d = opcode - 0x0B;
                memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
                break;
            case DCONST:        // constants have native endianness
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                PUSH(vm, ((YASL_Object) {Y_FLOAT64, c}));
                break;
            case ICONST:        // constants have native endianness
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                PUSH(vm, ((YASL_Object) {Y_INT64, c}));
                break;
            case BCONST_F:
            case BCONST_T:
                vm->stack[++vm->sp].type = Y_BOOL;
                vm->stack[vm->sp].value.ival  = opcode & 0x01;
                break;
            case NCONST:
                vm->stack[++vm->sp].type = Y_UNDEF;
                vm->stack[vm->sp].value.ival  = 0x00;
                break;
            case FCONST:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                PUSH(vm, ((YASL_Object) {Y_FN, c}));
                break;
            case BOR:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = a.value.ival | b.value.ival;
                    break;
                } else {
                    printf("TypeError: | not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BXOR:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = a.value.ival ^ b.value.ival;
                    break;
                } else {
                    printf("TypeError: binary ~ not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BAND:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = a.value.ival & b.value.ival;
                    break;
                } else {
                    printf("TypeError: & not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BNOT:
                a = PEEK(vm);
                if (a.type == Y_INT64) {
                    PEEK(vm).value.ival = ~a.value.ival;
                    break;
                } else {
                    printf("TypeError: unary ~ not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case BSL:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = a.value.ival << b.value.ival;
                    break;
                } else {
                    printf("TypeError: << not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case BSR:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = (uint64_t)a.value.ival >> (uint64_t)b.value.ival;
                    break;
                } else {
                    printf("TypeError: >> not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case ADD:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, ADD, "+");
                break;
            case MUL:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, MUL, "*");
                break;
            case SUB:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                BINOP(vm, a, b, SUB, "binary -");
                break;
            case FDIV:    // handled differently because we always convert to float
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp--];
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    d = (double)a.value.ival / (double)b.value.ival;
                }
                else if (a.type == Y_FLOAT64 && b.type == Y_INT64) {
                    d = a.value.dval / (double)b.value.ival;
                }
                else if (a.type == Y_INT64 && b.type == Y_FLOAT64) {
                    d = (double)a.value.ival / b.value.dval;
                }
                else if (a.type == Y_FLOAT64 && b.type == Y_FLOAT64) {
                    d = a.value.dval / b.value.dval;
                }
                else {
                    printf("TypeError: / not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                vm->stack[++vm->sp].type = Y_FLOAT64;
                memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
                break;
            case IDIV:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64) {
                    PEEK(vm).value.ival = a.value.ival / b.value.ival;
                    break;
                } else {
                    printf("TypeError: // not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
            case MOD:
                // TODO: handle undefined C behaviour for negative numbers.
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp];
                if (a.type != Y_INT64 || b.type != Y_INT64) {
                    printf("TypeError: %% not supported for operands of types %s and %s.\n",
                           YASL_TYPE_NAMES[a.type],
                           YASL_TYPE_NAMES[b.type]);
                    return;
                }
                vm->stack[vm->sp].value.ival = a.value.ival % b.value.ival;
                break;
            case EXP:
                b = POP(vm);
                a = POP(vm);
                if (a.type == Y_INT64 && b.type == Y_INT64 && b.value.ival < 0) {
                    d = pow((double)a.value.ival, (double)b.value.ival);
                    DPUSH(vm, d);
                    break;
                }
                BINOP(vm, a, b, EXP, "**");
                break;
            case NEG:
                a = vm->stack[vm->sp];
                if (a.type == Y_INT64) {
                    vm->stack[vm->sp].value.ival = -a.value.ival;
                    break;
                }
                else if (a.type == Y_FLOAT64) {
                    memcpy(&d, &vm->stack[vm->sp].value, sizeof(double));
                    d = -d;
                    memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
                    break;
                }
                else {
                    printf("TypeError: unary - not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case NOT:
                a = vm->stack[vm->sp];
                if (a.type == Y_BOOL) {
                    vm->stack[vm->sp].value.ival ^= 1;    // flip the last bit
                    break;
                }
                else if (a.type == Y_UNDEF) {
                    break;
                }
                else {
                    printf("TypeError: ! not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[a.type]);
                    return;
                }
            case LEN:
                v = vm->stack[vm->sp];
                if (v.type == Y_STR) {
                    vm->stack[vm->sp].value.ival = (v.value.sval)->length;
                } else if (v.type == Y_TABLE) {
                    vm->stack[vm->sp].value.ival = (v.value.mval)->count;
                } else if (v.type == Y_LIST) {
                    vm->stack[vm->sp].value.ival = (v.value.lval)->count;
                } else {
                    printf("TypeError: # not supported for operand of type %s.\n",
                           YASL_TYPE_NAMES[v.type]);
                    return;
                }
                vm->stack[vm->sp].type = Y_INT64;
                break;
            case CNCT:
                b = vm->stack[vm->sp--];
                a = vm->stack[vm->sp];
                if (a.type == Y_STR && b.type == Y_STR) {
                    size = (a.value.sval)->length + (b.value.sval)->length;
                    ptr = str_new_sized(size);
                    vm->stack[vm->sp].value.sval = ptr;
                    (vm->stack[vm->sp].value.sval)->length = size;
                    memcpy(((String_t*)ptr)->str, (a.value.sval)->str, (a.value.sval)->length);
                    memcpy(((String_t*)ptr)->str + (a.value.sval)->length, (b.value.sval)->str, (b.value.sval)->length);
                    break;
                } else if (a.type == Y_LIST && b.type == Y_LIST) {
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
                printf("TypeError: || not supported for operands of types %s and %s.\n",
                       YASL_TYPE_NAMES[a.type],
                       YASL_TYPE_NAMES[b.type]);
                return;
            case HARD_CNCT:
                b = POP(vm);
                a = PEEK(vm);
                if (a.type != Y_STR || b.type != Y_STR) {
                    puts("||| should have coerced to strings, aborting.");
                    return;
                }
                if ((b.value.sval)->length == 0) break;
                if ((a.value.sval)->length == 0) {
                    PEEK(vm) = b;
                    break;
                }
                size = (a.value.sval)->length + (b.value.sval)->length + 1;
                ptr = str_new_sized(size);
                vm->stack[vm->sp].value.sval = ptr; //ew_sized_string8(size);
                (vm->stack[vm->sp].value.sval)->length = size;
                memcpy(((String_t*)ptr)->str, (a.value.sval)->str, (a.value.sval)->length);
                (PEEK(vm).value.sval)->str[(a.value.sval)->length] = ' ';
                memcpy(((String_t*)ptr)->str + (a.value.sval)->length + 1, (b.value.sval)->str, (b.value.sval)->length);
                break;
            case GT:
                b = POP(vm);
                a = POP(vm);
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
                b = POP(vm);
                a = POP(vm);
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
                b = POP(vm);
                a = PEEK(vm);
                vm->stack[vm->sp] = (vm, isequal(a, b));
                break;
            case ID:
                b = POP(vm);
                a = PEEK(vm);
                vm->stack[vm->sp].value.ival = a.type == b.type && a.value.ival == b.value.ival;
                vm->stack[vm->sp].type = Y_BOOL;
                break;
            case NEWSTR:
                vm->stack[++vm->sp].type = Y_STR;

                memcpy(&addr, vm->code+vm->pc, sizeof(int64_t));
                vm->pc += sizeof(int64_t);

                memcpy(&size, vm->code + addr, sizeof(int64_t));

                vm->stack[vm->sp].value.sval  = str_new_sized(size);

                vm->stack[vm->sp].value.sval->length = size;

                memcpy(vm->stack[vm->sp].value.sval->str, vm->code+addr+sizeof(int64_t), size);

                break;
            case NEWTABLE: {
                Hash_t *ht = ht_new();
                while(PEEK(vm).type != Y_END) {
                    YASL_Object key = POP(vm);
                    YASL_Object value = POP(vm);
                    ht_insert(ht, key, value);
                }
                vm->stack[++vm->sp].type = Y_TABLE;
                vm->stack[vm->sp].value.mval  = ht;
                break;
            }
            case NEWLIST: {
                List_t *ls = ls_new();
                while (PEEK(vm).type != Y_END) {
                    ls_append(ls, POP(vm));
                }
                POP(vm);
                vm->stack[++vm->sp].type = Y_LIST;
                vm->stack[vm->sp].value.lval = ls;
                break;
            }
            case INITFOR:
                vm->loopstack->stack[++vm->loopstack->sp] = POP(vm);
                vm->loopstack->indices[vm->loopstack->sp] = 0;
                break;
            case ENDFOR:
                vm->loopstack->sp--;
                break;
            case ITER_1:
                // NOTE: only supports lists currently
                addr = NCODE(vm);
                switch (vm->loopstack->stack[vm->loopstack->sp].type) {
                    case Y_LIST:
                        if (vm->loopstack->stack[vm->loopstack->sp].value.lval->count <= vm->loopstack->indices[vm->loopstack->sp]) {
                            BPUSH(vm, 0);
                        } else {
                            vm->globals[addr] = vm->loopstack->stack[vm->loopstack->sp].value.lval->items[vm->loopstack->indices[vm->loopstack->sp]++]; //.value.lval->items;
                            BPUSH(vm, 1);
                        }
                        break;
                    case Y_TABLE:
                        while ((vm->loopstack->stack[vm->loopstack->sp].value.mval->items[vm->loopstack->indices[vm->loopstack->sp]] == &TOMBSTONE ||
                                vm->loopstack->stack[vm->loopstack->sp].value.mval->items[vm->loopstack->indices[vm->loopstack->sp]] == NULL) &&
                                vm->loopstack->stack[vm->loopstack->sp].value.mval->size > vm->loopstack->indices[vm->loopstack->sp]) {
                            vm->loopstack->indices[vm->loopstack->sp]++;
                        }
                        if (vm->loopstack->stack[vm->loopstack->sp].value.mval->size <= vm->loopstack->indices[vm->loopstack->sp]) {
                            BPUSH(vm, 0);
                            break;
                        }
                        vm->globals[addr] = *vm->loopstack->stack[vm->loopstack->sp].value.mval->items[vm->loopstack->indices[vm->loopstack->sp]++]->key; //.value.lval->items;
                        BPUSH(vm, 1);
                        break;
                    default:
                        printf("object of type %s is not iterable.\n", YASL_TYPE_NAMES[vm->loopstack->stack[vm->loopstack->sp].type]);
                        exit(EXIT_FAILURE);
                }
                break;
            case ITER_2:
                exit(1);
            case END:
                vm->stack[++vm->sp].type = Y_END;
                break;
            case DUP:
                vm->stack[vm->sp+1] = vm->stack[vm->sp];
                vm->sp++;
                break;
            case SWAP:
                a = vm->stack[vm->sp];
                b = vm->stack[vm->sp-1];
                vm->stack[vm->sp-1] = a;
                vm->stack[vm->sp] = b;
                break;
            case GOTO:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc = c + (vm->pc < vm->pc0 ? 16 : vm->pc0);
                break;
            case BR_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                vm->pc += c;
                break;
            case BRF_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                v = vm->stack[vm->sp--];
                if (FALSEY(v)) vm->pc += c;
                break;
            case BRT_8:
                memcpy(&c, vm->code + vm->pc, sizeof c);
                vm->pc += sizeof c;
                v = vm->stack[vm->sp--];
                if (!(FALSEY(v))) vm->pc += c;
                break;
            case BRN_8:
                memcpy(&c, vm->code + vm->pc, sizeof(c));
                vm->pc += sizeof(c);
                v = vm->stack[vm->sp--];
                if (v.type != Y_UNDEF) vm->pc += c;
                break;
            case GLOAD_1:
                addr = vm->code[vm->pc++];               // get addr of var in locals
                vm->stack[++vm->sp] = vm->globals[addr];  // load value from memory of the provided addr
                break;
            case GSTORE_1:
                addr = vm->code[vm->pc++];
                vm->globals[addr] = vm->stack[vm->sp--];
                break;
            case LLOAD_1:
                offset = NCODE(vm);
                vm->stack[++vm->sp] = vm->stack[vm->fp+offset];
                break;
            case LSTORE_1:
                offset = NCODE(vm);
                vm->stack[vm->fp+offset] = vm->stack[vm->sp--];
                break;
            case CALL_8:
                if (PEEK(vm).type == Y_FN) {
                    offset = NCODE(vm);
                    addr = POP(vm).value.ival;
                    PUSH(vm, ((YASL_Object) {offset, vm->fp}));  // store previous frame ptr;
                    PUSH(vm, ((YASL_Object) {offset, vm->pc}));  // store pc addr
                    vm->fp = vm->sp;
                    if (vm->code[addr] != offset) {
                        puts("CallError: wrong number params.");
                    }
                    offset = vm->code[addr+1];
                    vm->sp += offset + 1; // + 2
                    vm->pc = addr + 2;
                    break;
                } else if (PEEK(vm).type == Y_BFN) {
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
                if (PEEK(vm).type == Y_LIST) {
                    if (!list___get(vm)) break;
                } else if (PEEK(vm).type == Y_TABLE) {
                    if (!table___get(vm)) break;
                } else {
                    POP(vm);
                }
                YASL_Object key = POP(vm);
                YASL_Object *result = ht_search(vm->builtins_htable[index], key);
                if (result == NULL) {
                    printf("%s\n", YASL_TYPE_NAMES[index]);
                    puts("Not foundsdadsadsasds");
                    exit(1);
                } else {
                    PUSH(vm, *result);
                }
                break;
            }
            case SET:
                if (PEEK(vm).type == Y_LIST) {
                    list___set(vm);
                } else if (PEEK(vm).type == Y_TABLE) {
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
                memcpy(&addr, vm->code + vm->pc, sizeof addr);
                vm->pc += sizeof addr;
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
                PUSH(vm, v);
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