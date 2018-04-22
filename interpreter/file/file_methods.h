#pragma once

#include <stdio.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"

int file_close(VM* vm);
int file_pclose(VM *vm);
int file_write(VM* vm);
int file_read(VM* vm);
int file_readline(VM* vm);