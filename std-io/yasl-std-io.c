#include "yasl-std-io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define YASL_FILE (-3)

int YASL_io_open(struct YASL_State *S) {
    struct YASL_Object *mode = YASL_popobject(S);
    char *mode_str;
    if (YASL_isundef(mode) == YASL_SUCCESS) {
        mode_str = "r";
    } else if (YASL_isstring(mode) == YASL_SUCCESS) {
        mode_str = YASL_getcstring(mode);
    } else {
        return -1;
    }

    struct YASL_Object *filename = YASL_popobject(S);
    if (YASL_isstring(filename) != YASL_SUCCESS) {
        return -1;
    }
    char *filename_str = YASL_getcstring(filename);

    size_t mode_len = strlen(mode_str);
    size_t filename_len = strlen(filename_str);

    if (mode_len > 2 || mode_len < 1 || (mode_len == 2 && mode_str[1] != '+')) {
        return -1;
    }

    FILE *f;
    if (mode_len == 1) {
        switch (mode_str[0]) {
            case 'r':
                f = fopen(filename_str, "r");
                break;
            case 'w':
                f = fopen(filename_str, "w");
                break;
            case 'a':
                f = fopen(filename_str, "a");
                break;
            default:
                // invalid mode;
                return -1;
        }
    }
    if (mode_len == 2) {
        switch (mode_str[0]) {
            case 'r':
                f = fopen(filename_str, "r+");
                break;
            case 'w':
                f = fopen(filename_str, "w+");
                break;
            case 'a':
                f = fopen(filename_str, "a+");
                break;
            default:
                // invalid mode;
                return -1;
        }
    }
    YASL_pushobject(S, f ? YASL_UserData(f, YASL_FILE) : YASL_Undef());
    return 0;
}

int YASL_io_read(struct YASL_State *S) {
    struct YASL_Object *mode = YASL_popobject(S);
    char *mode_str;

    if (YASL_isundef(mode) == YASL_SUCCESS) {
        mode_str = "a";
    } else if (YASL_isstring(mode) == YASL_SUCCESS) {
        mode_str = YASL_getcstring(mode);
    } else {
        return -1;
    }

    struct YASL_Object *file = YASL_popobject(S);
    FILE *f;


    if (YASL_isuserdata(file, YASL_FILE) == YASL_SUCCESS) {
        f = YASL_UserData_getdata(file);
    } else {
        return -1;
    }

    size_t mode_len = strlen(mode_str);

    if (mode_len != 1) {
        return -1;
    }

    switch (mode_str[0]) {
        case 'a':
            fseek(f, 0, SEEK_END);
            size_t fsize = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *string = malloc(fsize + 1);
            fread(string, fsize, 1, f);
            string[fsize] = '\0';
            YASL_pushobject(S, YASL_CString(string));
            return 0;
        default:
            return -1;
    }

}

int YASL_load_io(struct YASL_State *S) {
    struct YASL_Object *io = YASL_Table();

    struct YASL_Object *open_str = YASL_CString("open");
    struct YASL_Object *open_fn = YASL_CFunction(YASL_io_open, 2);

    YASL_Table_set(io, open_str, open_fn);

    struct YASL_Object *read_str = YASL_CString("read");
    struct YASL_Object *read_fn = YASL_CFunction(YASL_io_read, 2);

    YASL_Table_set(io, read_str, read_fn);

    YASL_declglobal(S, "io");
    YASL_pushobject(S, io);
    YASL_setglobal(S, "io");

    free(open_str);
    free(open_fn);
    free(read_str);
    free(read_fn);

    return YASL_SUCCESS;
}

