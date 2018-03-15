#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "../VM.c"
#include "../opcode.c"
#include "../constant/constant.c"
#include "../hashtable/hashtable.c"
#include "../list/list.c"
#include "../string8/string8.c"

typedef int (*Handler)(VM*);