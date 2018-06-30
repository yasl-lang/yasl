#include <interpreter/YASL_Object/YASL_Object.h>
#include <debug.h>
#include "file_methods.h"

int file_close(VM* vm) {
    //puts("trying to close file");
    FILE *f = (POP(vm).value.fval);
    if (fclose(f)) {
        YASL_DEBUG_LOG("%s\n", "error closing file.");
        BPUSH(vm, 0);
        return 0;
    }
    YASL_DEBUG_LOG("%s\n", "file closed successfully.");
    BPUSH(vm, 1);
    return 0;
}

int file_pclose(VM* vm) {
    //puts("trying to close file");
    FILE *f = (POP(vm).value.fval);
    if (pclose(f)) {
        YASL_DEBUG_LOG("%s\n", "error closing process.");
        BPUSH(vm, 0);
    }
    YASL_DEBUG_LOG("%s\n", "process closed successfully.");
    BPUSH(vm, 1);
    return 0;
}

int file_write(VM* vm) {
    YASL_Object fileh = POP(vm);
    YASL_Object str = POP(vm);
    if (str.type != STR) {
        printf("Error: file.write expected type %s as first argument, got type %s\n", YASL_TYPE_NAMES[STR], YASL_TYPE_NAMES[str.type]);
        return -1;
    }
    // TODO: don't rely on C-strings for writing
    char *buffer = malloc((str.value.sval)->length+1);
    memcpy(buffer, (str.value.sval)->str, (str.value.sval)->length);
    buffer[(str.value.sval)->length] = '\0';
    if (fprintf(fileh.value.fval, "%s", buffer) < 0) {
        YASL_DEBUG_LOG("%s\n", "error writing to file.");
        BPUSH(vm, 0);
    } else {
        YASL_DEBUG_LOG("%s\n", "file written to successfully.");
        BPUSH(vm, 1);
    }
    free(buffer);
    return 0;
}

int file_read(VM* vm) {
    FILE* f = (POP(vm).value.fval);
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);
    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(f))){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str) return -1; // ERROR
        }
    }
    str = realloc(str, sizeof(char)*len);
    vm->stack[++vm->sp].value.sval = str_new_sized_from_mem(len, str);
    vm->stack[vm->sp].type = STR;
    YASL_DEBUG_LOG("%s\n", "successfully read from file.");
    return 0;
}

int file_readline(VM* vm) {
    FILE* f = (POP(vm).value.fval);
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);

    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(f)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str) return -1; // ERROR
        }
    }
    str = realloc(str, sizeof(char)*len);
    vm->stack[++vm->sp].value.sval = str_new_sized_from_mem(len, str);
    vm->stack[vm->sp].type = STR;
    YASL_DEBUG_LOG("%s\n", "successfully readline from file.");
    return 0;
}