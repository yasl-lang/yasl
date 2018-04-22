#pragma once

#include <stdio.h>
#include "../VM.h"
#include "../opcode.h"
#include "../constant/constant.h"
#include "hashtable.h"
#include "../list/list.h"

int hash_keys(VM *vm);

int hash_values(VM *vm);