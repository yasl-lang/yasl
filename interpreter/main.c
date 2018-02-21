/*
 ============================================================================
 Name        : VM
 Author      : Thiabaud Engelbrecht
 Version     : 0.0
 Copyright   :
 Description : Bytecode VM. Must be compiled with GCC
 ============================================================================
 */
#include <inttypes.h>
#include <string.h>
#include "VM.c"
#include "opcode.c"
#include "constant.c"
#include "builtins.c"
#include "hashtable/hashtable.c"
#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define IPUSH(vm, v) (PUSH(vm, ((Constant) {INT64, v})))  //push integer v onto stack
#define IPOP(vm)     (((vm->stack)[vm->sp--])->value)      // get int from top of stack
#define IVAL(v)      (*((int64_t*)&v->value))
#define DPUSH(vm, v) (((FloatConstant*)vm->stack)[++vm->sp] = (FloatConstant) {FLOAT64, v}) // push double v onto stack
#define LEN_C(v)     (*((int64_t*)v->value))
#define NPUSH(vm)    (PUSH(vm, ((Constant) {UNDEF, 0})))   //push nil onto stack
#define ADD(a, b)    (a + b)
#define DIV(a, b)    (a / b)
#define SUB(a, b)    (a - b)
#define MUL(a, b)    (a * b)
#define MOD(a, b)    (a % b)
#define GT(a, b)     (a > b)
#define GE(a, b)     (a >= b)
#define EQ(a, b)     (a == b)
#define BINOP(vm, a, b, f, str)  ({\
                            if (a.type == INT64 && b.type == INT64) {\
                                c = f(a.value, b.value);\
                                IPUSH(vm, c);\
                                break;\
                            }\
                            else if (a.type == FLOAT64 && b.type == INT64) {\
                                d = f(DVAL(a), (double)b.value);\
                            }\
                            else if (a.type == INT64 && b.type == FLOAT64) {\
                                d = f((double)a.value, DVAL(b));\
                            }\
                            else if (a.type == FLOAT64 && b.type == FLOAT64) {\
                                d = f(DVAL(a), DVAL(b));\
                            }\
                            else {\
                                printf("ERROR: %s not supported for operands of types %x and %x.\n", str, a.type, b.type);\
                                return;\
                            }\
                            DPUSH(vm, d);})
#define COMP(vm, a, b, f, str)  ({\
                            if (a.type == INT64 && b.type == INT64) {\
                                c = f(a.value, b.value);\
                            }\
                            else if (a.type == FLOAT64 && b.type == INT64) {\
                                c = f(DVAL(a), (double)b.value);\
                            }\
                            else if (a.type == INT64 && b.type == FLOAT64) {\
                                c = f((double)a.value, DVAL(b));\
                            }\
                            else if (a.type == FLOAT64 && b.type == FLOAT64) {\
                                c = f(DVAL(a), DVAL(b));\
                            }\
                            else {\
                                printf("ERROR: %s not supported for operands of types %x and %x.\n", str, a.type, b.type);\
                                return;\
                            }\
                            BPUSH(vm, c);})


void run(VM* vm){
    while (1) {
        unsigned char opcode = NCODE(vm);        // fetch
        int argc, rval;
        signed char offset;
        uint64_t big_offset, size;
        int64_t addr;
        int i;
        Constant a, b, v;
        int64_t c;
        double d;
        void* ptr;
        //printf("\nopcode: %x\n", opcode);
        //printf("pc: %d\n", vm->pc);
        //printf("vm->sp: %d, vm->pc: %d\n", vm->sp, vm->pc);
        /*printf("stack[0, 1, 2, 3] are %d:%x, %d:%x, %d:%x, %d:%x\n",
               (int)vm->stack[vm->sp].value,     (int)vm->stack[vm->sp].type,
               (int)vm->stack[vm->sp - 1].value, (int)vm->stack[vm->sp - 1].type,
               (int)vm->stack[vm->sp - 2].value, (int)vm->stack[vm->sp - 2].type,
               (int)vm->stack[vm->sp - 3].value, (int)vm->stack[vm->sp - 3].type); //
        printf("globals[0, 1, 2, 3] are %d:%x, %d:%x, %d:%x, %d:%x\n",
               (int)vm->globals[0].value, (int)vm->globals[0].type,
               (int)vm->globals[1].value, (int)vm->globals[1].type,
               (int)vm->globals[2].value, (int)vm->globals[2].type,
               (int)vm->globals[3].value, (int)vm->globals[3].type); //*/
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
            vm->stack[++vm->sp].type = INT64;
            vm->stack[vm->sp].value  = opcode - 0x04;
            break;
        case DCONST_0:    // TODO: make sure no changes to opcodes ruin this
        case DCONST_1:
        case DCONST_2:
            vm->stack[++vm->sp].type = FLOAT64;
            d = opcode - 0x0B;
            memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
            break;
        case DCONST:        // constants have native endianness
            memcpy(&c, vm->code + vm->pc, sizeof c);
            vm->pc += sizeof c;
            PUSH(vm, ((Constant) {FLOAT64, c}));
            break;
        case ICONST:        // constants have native endianness
            memcpy(&c, vm->code + vm->pc, sizeof c);
            vm->pc += sizeof c;
            PUSH(vm, ((Constant) {INT64, c}));
            break;
        case BCONST_F:
        case BCONST_T:
            vm->stack[++vm->sp].type = BOOL;
            vm->stack[vm->sp].value  = opcode & 0x01;
            break;
        case NCONST:
            vm->stack[++vm->sp].type = UNDEF;
            vm->stack[vm->sp].value  = 0x00;
            break;
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
        case DIV:    // handled differently because we always convert to float
            b = vm->stack[vm->sp--];
            a = vm->stack[vm->sp--];
            if (a.type == INT64 && b.type == INT64) {
                d = (double)a.value / (double)b.value;
            }
            else if (a.type == FLOAT64 && b.type == INT64) {
                d = DVAL(a) / (double)b.value;
            }
            else if (a.type == INT64 && b.type == FLOAT64) {
                d = (double)a.value / DVAL(b);
            }
            else if (a.type == FLOAT64 && b.type == FLOAT64) {
                d = DVAL(a) / DVAL(b);
            }
            else {
                printf("ERROR: / not supported for operands of types %x and %x.\n", a.type, b.type);
                return;
            }
            vm->stack[++vm->sp].type = FLOAT64;
            memcpy(&vm->stack[vm->sp].value, &d, sizeof(d));
            break;
        case MOD:
            b = vm->stack[vm->sp--];
            a = vm->stack[vm->sp];
            if (a.type != INT64 || b.type != INT64) {
               printf("ERROR: %% not supported for operands of types %x and %x.\n", a.type, b.type);
               return;
            }
            vm->stack[vm->sp].value = a.value % b.value;
            break;
        case NEG:
            a = vm->stack[vm->sp];
            if (a.type == INT64) {
                vm->stack[vm->sp].value = -a.value;
                break;
            }
            else if (a.type == FLOAT64) {
                memcpy(&d, &vm->stack[vm->sp].value, sizeof(double));
                d = -d;
                memcpy(&vm->stack[vm->sp].value, &d, sizeof(double));
                break;
            }
            else {
                printf("ERROR: unary - not supported for operand of type %x.\n", a.type);
                return;
            }
        case NOT:
            a = vm->stack[vm->sp];
            if (a.type == BOOL) {
                vm->stack[vm->sp].value ^= 1;    // flip the last bit
                break;
            }
            else if (a.type == UNDEF) {
                break;
            }
            else {
                printf("ERROR: ! not supported for operand of type %x.\n", a.type);
                return;
            }
        case LEN:
            v = vm->stack[vm->sp];
            if (v.type == STR8) {
                vm->stack[vm->sp].value = *(int64_t*)v.value;
            } else if (v.type == HASH) {
                vm->stack[vm->sp].value = ((Hash_t*)v.value)->count;
            } else if (v.type == LIST) {
                vm->stack[vm->sp].value = ((List_t*)v.value)->count;
            } else {
                printf("ERROR: # not supported for operand of type %x.\n", v.type);
                return;
            }
            vm->stack[vm->sp].type = INT64;
            break;
        case CONCAT:
            b = vm->stack[vm->sp--];
            a = vm->stack[vm->sp];
            if (a.type == STR8 && b.type == STR8) {
                size = *(int64_t*)a.value + *(int64_t*)b.value;
                ptr = malloc(size);
                vm->stack[vm->sp].value = (int64_t)ptr;
                *(int64_t*)vm->stack[vm->sp].value = size;
                memcpy((char*)(ptr + 8), (char*)(a.value + 8), *(int64_t*)a.value);
                memcpy((char*)(ptr + 8 + *(int64_t*)a.value), (char*)(b.value + 8), *(int64_t*)b.value);
                break;
            }
            printf("ERROR: %% not supported for operands of types %x and %x.\n", a.type, b.type);
            return;
        case GT:
            b = POP(vm);
            a = POP(vm);
            if ((a.type != INT64 && a.type != FLOAT64) ||
                (b.type != INT64 && b.type != FLOAT64)) {
                printf("ERROR: < and > not supported for operand of types %x and %x.\n", a.type, b.type);
                return;
            }
            COMP(vm, a, b, GT, ">");
            break;
        case GE:
            b = POP(vm);
            a = POP(vm);
            if ((a.type != INT64 && a.type != FLOAT64) ||
                (b.type != INT64 && b.type != FLOAT64)) {
                printf("ERROR: <= and >= not supported for operand of types %x and %x.\n", a.type, b.type);
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
            vm->stack[vm->sp].value = a.type == b.type && a.value == b.value;
            vm->stack[vm->sp].type = BOOL;
            break;
        case NEWHASH:
            vm->stack[++vm->sp].type = HASH;
            vm->stack[vm->sp].value  = (int64_t)new_hash();
            break;
        case NEWLIST:
            vm->stack[++vm->sp].type = LIST;
            vm->stack[vm->sp].value  = (int64_t)new_list();
            break;
        /*
        case V2S:   // TODO: implement
            break;
        */
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
            if (v.type != UNDEF) vm->pc += c;
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
            offset = NCODE(vm);
            memcpy(&addr, vm->code + vm->pc, sizeof addr);
            vm->pc += sizeof addr;
            PUSH(vm, ((Constant) {offset, vm->fp}));  // store previous frame ptr;
            PUSH(vm, ((Constant) {offset, vm->pc}));  // store pc addr
            //printf("%" PRId64 "\n", (int64_t)offset);
            //printf("%d\n", vm->pc);
            vm->fp = vm->sp;
            offset = NCODE(vm);
            vm->sp += offset;// + 2;
            vm->pc = addr;
            break;
        case BCALL_8:
            memcpy(&addr, vm->code + vm->pc, sizeof(addr));
            vm->pc += sizeof(addr);
            if (builtins[addr](vm)) {
                printf("ERROR: invalid argument type(s) to builtin function.\n");
                return;
            };
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
            vm->pc = a.value;
            vm->pc++;
            vm->sp = vm->fp - a.type;
            vm->sp--;
            vm->fp = b.value;
            PUSH(vm, v);
            break;
        case POP:
            --vm->sp;      // throw away value at top of the stack
            break;
        case MLC_8:
            vm->stack[++vm->sp].type = vm->code[vm->pc++];
            memcpy(&size, vm->code + vm->pc, sizeof(size));
            vm->pc += sizeof(size);
            ptr = malloc(size);
            vm->stack[vm->sp].value = (int64_t)ptr;
            break;
        case MCP_8:
            memcpy(&big_offset, vm->code + vm->pc, sizeof(big_offset));
            vm->pc += sizeof(big_offset);
            memcpy(&size, vm->code + vm->pc, sizeof(size));
            vm->pc += sizeof(size);
            memcpy((char*)vm->stack[vm->sp].value + big_offset, vm->code + vm->pc, size);
            vm->pc += size;
            break;
        /*case PRINT:
            v = vm->stack[vm->sp--];    // pop value from top of the stack ...
            switch (v.type) {
            case INT64:
                printf("int64: %" PRId64 "\n", v.value);
                break;
            case FLOAT64:
                printf("float64: %f\n", *((double*)&v.value));
                break;
            case BOOL:
                if (v.value == 0) printf("bool: false\n");
                else printf("bool: true\n");
                break;
            case UNDEF:
                printf("undef: undef\n");
                break;
            case STR8:
                printf("str: ");
                for (i = sizeof(int64_t); i < *((int64_t*)v.value) + sizeof(int64_t); i++) { // TODO: fix hardcoded 8
                    printf("%c", *((char*)(v.value + i)));
                }
                printf("\n");
                break;
            case HASH:
                printf("hash: <%" PRIx64 ">\n", v.value);
                break;
            case LIST:
                printf("list: <%" PRIx64 ">\n", v.value);
                break;
            default:
                printf("ERROR UNKNOWN TYPE: %x\n", v.type);
                break;
            }
            break; //*/
        default:
            printf("ERROR UNKNOWN OPCODE: %x\n", opcode);
            return;
        }

    }
}

char *buffer;
FILE *file_ptr;
long file_len;
int64_t entry_point, num_globals;

int main(void) {
    file_ptr = fopen("source.yb", "rb");
    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    rewind(file_ptr);
    buffer = (char *)malloc((file_len+1)*sizeof(char));
    fread(buffer, file_len, 1, file_ptr);
    entry_point = *((int64_t*)buffer);
    num_globals = *((int64_t*)buffer+1);
    //printf("num_globals = %" PRId64 "\n", num_globals);
    // printf("entry_point = %" PRId64 "\n", entry_point);
    //bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file_ptr);
	VM* vm = newVM(buffer,   // program to execute
	                   entry_point,    // start address of main function
	                   256);   // locals to be reserved, should be num_globals
	run(vm);
	delVM(vm);
    fclose(file_ptr);
	return 0;
};
