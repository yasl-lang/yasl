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
#include "constant_types.c"
#include "builtins.c"
#define BUFFER_SIZE 256
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define IPUSH(vm, v) (PUSH(vm, ((Constant) {INT64, v})))  //push integer v onto stack
#define IPOP(vm)     (((vm->stack)[vm->sp--]).value)      // get int from top of stack
#define IVAL(v)      (*((int64_t*)&v.value))
#define DPUSH(vm, v) (((FloatConstant*)vm->stack)[++vm->sp] = (FloatConstant) {FLOAT64, v}) // push double v onto stack
#define DVAL(v)      (*((double*)&v.value))
#define LEN(v)       (*((int64_t*)v.value))
#define NPUSH(vm)    (PUSH(vm, ((Constant) {UNDEF, 0})))   //push nil onto stack
#define FALSEY(v)    (v.type == UNDEF || (v.type == BOOL && v.value == 0))  // returns true if v is a falsey value
#define ADD(a, b)    (a + b)
#define DIV(a, b)    (a / b)
#define SUB(a, b)    (a - b)
#define MUL(a, b)    (a * b)
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
    for (;;) {
        unsigned char opcode = NCODE(vm);        // fetch
        int argc, rval;
        signed char offset;
        uint64_t big_offset, size;
        int64_t addr;
        int i;
        Constant a, b, v;
        int64_t c;
        double d;
        //printf("opcode: %x\n", opcode);
        //printf("sp, fp, pc: %d, %d, %d\n", vm->sp, vm->fp, vm->pc);
        //printf("locals: %" PRId64 ", %" PRId64 ", %" PRId64 "\n", vm->stack[vm->fp+1].value, vm->stack[vm->fp+2].value,
        //     vm->stack[vm->fp+3].value);
        switch (opcode) {   // decode
        case HALT: return;  // stop the program
        case NOP: break;    // pass
        case ICONST_M2:     // TODO: make sure no changes to opcodes ruin this
        case ICONST_M1:
        case ICONST_0:
        case ICONST_1:
        case ICONST_2:
        case ICONST_3:
        case ICONST_4:
        case ICONST_5:
        case ICONST_6:
            IPUSH(vm, (int64_t)(opcode - 0x03));
            break;
        case DCONST_M1:     // TODO: make sure no changes to opcodes ruin this
        case DCONST_0:
        case DCONST_1:
        case DCONST_2:
            DPUSH(vm, (double)(opcode - 0x0B));
            break;
        case DCONST:        // constants have native endianness
            memcpy(&d, vm->code + vm->pc, sizeof(d));
            vm->pc += sizeof(d);
            DPUSH(vm, d);
            break;
        case ICONST:        // constants have native endianness
            memcpy(&c, vm->code + vm->pc, sizeof(c));
            vm->pc += sizeof(c);
            IPUSH(vm, c);
            break;
        case ADD:
            b = POP(vm);
            a = POP(vm);
            BINOP(vm, a, b, ADD, "+");
            break;
        case MUL:
            b = POP(vm);
            a = POP(vm);
            BINOP(vm, a, b, MUL, "*");
            break;
        case SUB:
            b = POP(vm);
            a = POP(vm);
            BINOP(vm, a, b, SUB, "binary -");
            break;
        case DIV:    // handled differently because we always convert to float
            b = POP(vm);
            a = POP(vm);
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
            DPUSH(vm, d);
            break;
        case NEG:
            a = POP(vm);
            if (a.type == INT64) {
                a.value = -a.value;
                PUSH(vm, a);
                break;
            }
            else if (a.type == FLOAT64) {
                d = -DVAL(a);
                DPUSH(vm, d);
                break;
            }
            else {
                printf("ERROR: unary - not supported for operand of type %x.\n", a.type);
                return;
            }
        case NOT:
            a = POP(vm);
            if (a.type == BOOL) {
                a.value ^= 1;    // flip the last bit of a.value
                PUSH(vm, a);
                break;
            }
            else if (a.type == UNDEF) {
                NPUSH(vm);
                break;
            }
            else {
                printf("ERROR: ! not supported for operand of type %x.\n", a.type);
                return;
            }
        case LEN:
            v = POP(vm);
            IPUSH(vm, LEN(v));
            break;
        case GT:
            b = POP(vm);
            a = POP(vm);
            if (a.type == UNDEF || a.type == BOOL || a.type == STR ||
                b.type == UNDEF || b.type == BOOL || b.type == STR)   {
                printf("ERROR: < and > not supported for operand of types %x and %x.\n", a.type, b.type);
                return;
            }
            COMP(vm, a, b, GT, ">");
            break;
        case GE:
            b = POP(vm);
            a = POP(vm);
            if (a.type == UNDEF || a.type == BOOL || a.type == STR ||
                b.type == UNDEF || b.type == BOOL || b.type == STR)   {
                printf("ERROR: <= and >= not supported for operand of types %x and %x.\n", a.type, b.type);
                return;
            }
            COMP(vm, a, b, GE, ">=");
            break;
        case EQ:
            b = POP(vm);
            a = POP(vm);
            if (a.type == UNDEF || b.type == UNDEF) {
                NPUSH(vm);
                break;
            }
            switch(a.type) {
            case BOOL:
                if (b.type == BOOL) {
                    c = a.value == b.value;
                    BPUSH(vm, c);
                }
                else BPUSH(vm, 0);
                break;
            case STR:
                if (b.type == STR) {
                    if (LEN(a) != LEN(b)) {
                        BPUSH(vm, 0);
                        break;
                    } else {
                        for (i = sizeof(int64_t); i < LEN(a) + sizeof(int64_t); i++) { // TODO: fix hardcoded 8
                            if (*((char*)(v.value + i)) != *((char*)(v.value + i))) {
                                BPUSH(vm, 0);
                                break;
                            }
                        }
                        BPUSH(vm, 1);
                        break;
                    }
                }
                BPUSH(vm, 0);
                break;
            default:
                if (b.type == BOOL) {
                    BPUSH(vm, 0);
                    break;
                }
                COMP(vm, a, b, EQ, "==");
                break;
            }
            break;
        case ID:
            b = POP(vm);
            a = POP(vm);
            if (a.type == b.type && a.value == b.value) {
                BPUSH(vm, 1);
            } else {
                BPUSH(vm, 0);
            }
            break; // */
        case BCONST_F:
            BPUSH(vm, 0);   // represent false as 0
            break;
        case BCONST_T:
            BPUSH(vm, 1);   // represent true as 1
            break;
        case NCONST:
            NPUSH(vm);
            break;
        //case I2D: TODO: implement
        //case D2I: TODO: implement
        case ISNIL:
            v = POP(vm);
            if (v.type == UNDEF) BPUSH(vm, 1);
            else BPUSH(vm, 0);
            break;
        case V2S: // TODO: implement
            break;
        case DUP:
            v = vm->stack[vm->sp];
            PUSH(vm, v);
            break;
        case DUP2:
            v = vm->stack[vm->sp-1];
            PUSH(vm, v);
            break;
        case SWAP:
            a = vm->stack[vm->sp];
            b = vm->stack[vm->sp-1];
            vm->stack[vm->sp-1] = a;
            vm->stack[vm->sp] = b;
            break;
        case SWAP_X1:
            a = vm->stack[vm->sp-1];
            b = vm->stack[vm->sp-2];
            vm->stack[vm->sp-2] = a;
            vm->stack[vm->sp-1] = b;
            break;
        case BR_8:
            memcpy(&c, vm->code + vm->pc, sizeof c);
            vm->pc += sizeof c;
            vm->pc += c;
            break;
        case BRF_8:
            memcpy(&c, vm->code + vm->pc, sizeof c);
            vm->pc += sizeof c;
            v = POP(vm);
            if (FALSEY(v)) vm->pc += c;
            break;
        case BRT_8:
            memcpy(&c, vm->code + vm->pc, sizeof c);
            vm->pc += sizeof c;
            v = POP(vm);
            if (!(FALSEY(v))) vm->pc += c;
            break;
        case GLOAD_1:
            addr = NCODE(vm);             // get addr of var in locals
            v = vm->locals[addr];         // load value from memory of the provided addr
            PUSH(vm, v);                  // put that value on top of the stack
            break;
        case GSTORE_1:
            v = POP(vm);                  // get value from top of the stack
            addr = NCODE(vm);             // get addr of var in locals
            vm->locals[addr] = v;         // store value at addr received
            break;
        case LLOAD_1:
            offset = NCODE(vm);
            //printf("offset = %d\n", offset);
            //printf("%d\n", offset);
            v = vm->stack[vm->fp+offset];
            //printf("value loaded is: %" PRId64 "\n", v.value);
            PUSH(vm, v);
            break;
        case LSTORE_1:
            offset = NCODE(vm);
            //printf("offset = %d\n", offset);
            v = POP(vm);
            //printf("value to store is: %" PRId64 "\n", v.value);
            vm->stack[vm->fp+offset] = v;
            break;
        case CALL_8:
            //printf("sp, fp, pc: %d, %d, %d\n", vm->sp, vm->fp, vm->pc);
            offset = NCODE(vm);
            memcpy(&addr, vm->code + vm->pc, sizeof addr);
            vm->pc += sizeof addr;
            PUSH(vm, ((Constant) {offset, vm->fp}));  // store previous frame ptr;
            PUSH(vm, ((Constant) {offset, vm->pc}));  // store pc addr
            //printf("%" PRId64 "\n", (int64_t)offset);
            //printf("%d\n", vm->pc);
            vm->fp = vm->sp;
            offset = NCODE(vm);
            vm->sp += offset + 2;
            vm->pc = addr;
            break;
        case BCALL_8:
            memcpy(&addr, vm->code + vm->pc, sizeof addr);
            vm->pc += sizeof addr;
            if (!(builtins[addr](vm))) {
                printf("ERROR: invalid argument type(s) to builtin function.");
            };
            break;
        /* case RCALL_8:
            offset = NCODE(vm);
            memcpy(&addr, vm->code + vm->pc, sizeof addr);
            vm->pc += sizeof addr;
            int i;
            for (i = 0; i < offset; i++) {
                vm->stack[vm->fp - 2 - i] = vm->stack[vm->sp - i];
            }
        */
        case RET:
            //printf("sp, fp, pc: %d, %d, %d\n", vm->sp, vm->fp, vm->pc);
            v = POP(vm);
            a = vm->stack[vm->fp];
            b = vm->stack[vm->fp-1];
            vm->pc = a.value;
            vm->pc++;
            vm->sp = vm->fp - a.type;
            vm->sp--;
            vm->fp = b.value;
            //printf("%" PRId64 "\n", a.value);
            //printf("%d\n", a.type);
            PUSH(vm, v);
            break; // */
        case POP:
            --vm->sp;      // throw away value at top of the stack
            break;
        case MLC_8:
            i = NCODE(vm);
            memcpy(&size, vm->code + vm->pc, sizeof size);
            vm->pc += sizeof size;
            PUSH(vm, ((Constant) {i, (int64_t)malloc(size)}));
            break;
        case MLC:
            i = NCODE(vm);
            size = IPOP(vm);
            //printf("MLC: size = %" PRId64 "\n", size);
            PUSH(vm, ((Constant) {i, (int64_t)malloc(size)}));
            break;
        case MCP_8:
            memcpy(&big_offset, vm->code + vm->pc, sizeof big_offset);
            vm->pc += sizeof big_offset;
            memcpy(&size, vm->code + vm->pc, sizeof size);
            vm->pc += sizeof size;
            v = vm->stack[vm->sp];
            memcpy((char*)v.value + big_offset, vm->code + vm->pc, size);
            vm->pc += size;
            break;
        case SCP:
            v = POP(vm);
            //printf("SCP: size = %" PRId64 "\n", *((int64_t*)v.value));
            big_offset = IPOP(vm);
            a = POP(vm);
            memcpy((char*)(v.value + 8 + big_offset), (char*)(a.value+8), *((int64_t*)a.value));
            PUSH(vm, v);
            break;
        case ICP:
            v = POP(vm);
            //printf("ICP: size = %" PRId64 "\n", *((int64_t*)v.value));
            c = IPOP(vm);
            //printf("c = %" PRId64 "\n", c);
            *((int64_t*)v.value) = c;
            //printf("ICP: size = %" PRId64 "\n", *((int64_t*)v.value));
            //memcpy((int64_t*)(v.value), &c, sizeof(int64_t));
            PUSH(vm, v);
            break;
        case PRINT:
            v = POP(vm);        // pop value from top of the stack ...
            //printf("%x\n", v.type); // print value
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
            case STR:
                printf("str: ");
                for (i = sizeof(int64_t); i < *((int64_t*)v.value) + sizeof(int64_t); i++) { // TODO: fix hardcoded 8
                    printf("%c", *((char*)(v.value + i)));
                }
                printf("\n");
                break;
            default:
                printf("ERROR UNKNOWN TYPE: %x\n", v.type);
                break;
            }
            break;
        default:
            printf("ERROR UNKNOWN OPCODE: %x\n", opcode);
            return;
        }

    }
}

char *buffer;
FILE *file_ptr;
long file_len;
int64_t entry_point;

int main(void) {
    file_ptr = fopen("source.yb", "rb");
    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    rewind(file_ptr);
    buffer = (char *)malloc((file_len+1)*sizeof(char));
    fread(buffer, file_len, 1, file_ptr);
    entry_point = *((int64_t*)buffer);
    // printf("entry_point = %" PRId64 "\n", entry_point);
    //bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file_ptr);
	VM* vm = newVM(buffer,   // program to execute
	                   entry_point,    // start address of main function
	                   0);   // locals to be reserved, fib doesn't require them
	run(vm);
	delVM(vm);
    fclose(file_ptr);
	return 0;
};
