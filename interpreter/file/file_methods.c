#include "file_methods.h"

int file_close(VM* vm) {
    //puts("trying to close file");
    FILE *f = (FILE*)(POP(vm).value);
    if (fclose(f)) {
        puts("Error: unable closing file");
        return -1;
    }
    //puts("closed successfully");
    return 0;
}

int file_pclose(VM* vm) {
    //puts("trying to close file");
    FILE *f = (FILE*)(POP(vm).value);
    if (pclose(f)) {
        puts("Error: unable closing file");
        return -1;
    }
    //puts("closed successfully");
    return 0;
}

int file_write(VM* vm) {
    YASL_Object fileh = POP(vm);
    YASL_Object str = POP(vm);
    if (str.type != STR8) {
        printf("Error: file.write expected type %x as first argument, got type %x\n", STR8, str.type);
        return -1;
    }
    char *buffer = malloc(((String_t*)str.value)->length + 1);
    memcpy(buffer, ((String_t*)str.value)->str, ((String_t*)str.value)->length);
    buffer[((String_t*)str.value)->length] = '\0';
    if (fprintf((FILE*)fileh.value, "%s", ((String_t*)str.value)->str) < 0) {
        printf("Error: failed to write to file\n");
        return -1;
    }
    return 0;
}

int file_read(VM* vm) {
    FILE* f = (FILE*)(POP(vm).value);
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);
    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(f))){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return -1; // ERROR
        }
    }
    str = realloc(str, sizeof(char)*len);
    vm->stack[++vm->sp].value = (int64_t)new_sized_string8_from_mem(len, str);
    vm->stack[vm->sp].type = STR8;
    return 0;
}

int file_readline(VM* vm) {
    FILE* f = (FILE*)(POP(vm).value);
    int ch;
    size_t len = 0;
    size_t size = 10;
    char *str = realloc(NULL, sizeof(char)*size);

    if (!str) return -1; // ERROR
    while(EOF!=(ch=fgetc(f)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return -1; // ERROR
        }
    }
    str = realloc(str, sizeof(char)*len);
    vm->stack[++vm->sp].value = (int64_t)new_sized_string8_from_mem(len, str);
    vm->stack[vm->sp].type = STR8;
    return 0;
}