#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"

int file_close(struct VM* vm);
int file_pclose(struct VM *vm);
int file_write(struct VM* vm);
int file_read(struct VM* vm);
int file_readline(struct VM* vm);