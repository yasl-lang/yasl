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
#include "VM.c"
#include "opcode.c"
#include "constant_types.c"
//#include "constant.c"
#define BUFFER_SIZE 256
#define PUSH(vm, v)  (vm->stack[++vm->sp] = v) // push value on top of the stack
#define POP(vm)      (vm->stack[vm->sp--])    // pop value from top of the stack as integer
#define NCODE(vm)    (vm->code[vm->pc++])     // get next bytecode
#define IPUSH(vm, v) (PUSH(vm, ((Constant) {INT64, v})))  //push integer v onto stack
#define IVAL(v)      (*((int64_t*)&v.value))
#define DPUSH(vm, v) (((FloatConstant*)vm->stack)[++vm->sp] = (FloatConstant) {FLOAT64, v}) // push double v onto stack
#define DPOP(vm)     (((double*)vm->stack)[vm->sp--])
#define DVAL(v)      (*((double*)&v.value))
#define BPUSH(vm, v) (PUSH(vm, ((Constant) {BOOL, v})))  //push boolean v onto stack

void run(VM* vm){
    for (;;) {
        unsigned char opcode = NCODE(vm);        // fetch
        int addr, offset, argc, rval;
        int i;
        Constant a, b, v;
        int64_t c;
        double d;
        //double c, d;
        unsigned char bytes[8];
        printf("opcode = %x\n", opcode);
        switch (opcode) {   // decode
        case HALT: return;  // stop the program
        case JMP:
        	vm->pc = NCODE(vm);
        	break;
        case ICONST_M1:
            IPUSH(vm, -1);
            break;
        case ICONST_0:
            IPUSH(vm, 0);
            break;
        case ICONST_1:
            IPUSH(vm, 1);
            break;
        case ICONST_2:
            IPUSH(vm, 2);
            break;
        case ICONST_3:
            IPUSH(vm, 3);
            break;
        case ICONST_4:
            IPUSH(vm, 4);
            break;
        case ICONST_5:
            IPUSH(vm, 5);
            break;
        case DCONST_0:
            DPUSH(vm, 0.0);
            break;
        case DCONST_1:
            DPUSH(vm, 1.0);
            break;
        case DCONST_2:
            DPUSH(vm, 2.0);
            break;
        case DCONST:
            c = 0;
            for (i = 56; i >= 0; i -= 8) {
              unsigned char next = NCODE(vm);
              c += ((int64_t)next) << i;
            }
            PUSH(vm, ((Constant) {FLOAT64, c}));
            break;
        case ICONST: // constants are BIG endian
            c = 0;
            for (i = 56; i >= 0; i -= 8) {
              unsigned char next = NCODE(vm);
              c += ((int64_t)next) << i;
            }
            PUSH(vm, ((Constant) {INT64, c}));
            break;
        case ADD:
            b = POP(vm);
            a = POP(vm);
            if (a.type == INT64 && b.type == INT64) {
                c = a.value + b.value;
                IPUSH(vm, c);
                break;
            }
            else if (a.type == FLOAT64 && b.type == INT64) {
                d = DVAL(a) + (double)b.value;
            }
            else if (a.type == INT64 && b.type == FLOAT64) {
                d = (double)a.value + DVAL(b);
            }
            else if (a.type == FLOAT64 && b.type == FLOAT64) {
                d = DVAL(a) + DVAL(b);
            }
            else {
                printf("ERROR: + not supported for operands of types %x and %x.\n", a.type, b.type);
                return;
            }
            DPUSH(vm, d);
            break;
        case MUL:
            b = POP(vm);
            a = POP(vm);
            if (a.type == INT64 && b.type == INT64) {
                c = a.value * b.value;
                IPUSH(vm, c);
                break;
            }
            else if (a.type == FLOAT64 && b.type == INT64) {
                d = DVAL(a) * (double)b.value;
            }
            else if (a.type == INT64 && b.type == FLOAT64) {
                d = (double)a.value * DVAL(b);
            }
            else if (a.type == FLOAT64 && b.type == FLOAT64) {
                d = DVAL(a) * DVAL(b);
            }
            else {
                printf("ERROR: * not supported for operands of types %x and %x.\n", a.type, b.type);
                return;
            }
            DPUSH(vm, d);
            break;
        case SUB:
            b = POP(vm);
            a = POP(vm);
            if (a.type == INT64 && b.type == INT64) {
                c = a.value - b.value;
                IPUSH(vm, c);
                break;
            }
            else if (a.type == FLOAT64 && b.type == INT64) {
                d = DVAL(a) - (double)b.value;
            }
            else if (a.type == INT64 && b.type == FLOAT64) {
                d = (double)a.value - DVAL(b);
            }
            else if (a.type == FLOAT64 && b.type == FLOAT64) {
                d = DVAL(a) - DVAL(b);
            }
            else {
                printf("ERROR: - not supported for operands of types %x and %x.\n", a.type, b.type);
                return;
            }
            DPUSH(vm, d);
            break;
        case DIV:
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
        case FALSE:
            BPUSH(vm, 0);   // represent false as 0
            break;
        case TRUE:
            BPUSH(vm, 1);   // represent true as 1
            break;
        //case I2D: TODO: implement
        //case D2I: TODO: implement
        case GLOAD:
            addr = NCODE(vm);             // get addr of var in locals
            v = vm->locals[addr];         // load value from memory of the provided addr
            PUSH(vm, v);                  // put that value on top of the stack
            break;
        case GSTORE:
            v = POP(vm);                  // get value from top of the stack
            addr = NCODE(vm);             // get addr of var in locals
            vm->locals[addr] = v;         // store value at addr received
            break;
        case POP:
            --vm->sp;      // throw away value at top of the stack
            break;
        case PRINT:
            v = POP(vm);        // pop value from top of the stack ...
            //printf("%x\n", v.type); // print value
            switch (v.type) {
            case INT64:
                printf("\tint64: %" PRId64 "\n", v.value);
                break;
            case FLOAT64:
                printf("\tfloat64: %f\n", *((double*)&v.value));
                break;
            case BOOL:
                if (v.value == 0) printf("\tbool: false\n");
                else printf("\tbool: true\n");
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

int main(void) {
    file_ptr = fopen("source.bc", "rb");
    fseek(file_ptr, 0, SEEK_END);
    file_len = ftell(file_ptr);
    rewind(file_ptr);
    buffer = (char *)malloc((file_len+1)*sizeof(char));
    fread(buffer, file_len, 1, file_ptr);
    //bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file_ptr);
	VM* vm = newVM(buffer,   // program to execute
	                   0,    // start address of main function
	                   0);   // locals to be reserved, fib doesn't require them
	run(vm);
    fclose(file_ptr);
	return 0;
};
