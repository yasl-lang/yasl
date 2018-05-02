#pragma once

#include <stdio.h>
#include <string.h>
#include "../VM/VM.h"
#include "../../opcode.h"
#include "../YASL_Object/YASL_Object.h"

int file_close(VM* vm);
int file_pclose(VM *vm);
int file_write(VM* vm);
int file_read(VM* vm);
int file_readline(VM* vm);