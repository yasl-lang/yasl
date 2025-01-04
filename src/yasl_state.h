#ifndef YASL_YASL_STATE_H_
#define YASL_YASL_STATE_H_

#include "compiler/compiler.h"
#include "interpreter/VM.h"

// VM MUST BE FIRST ITEM IN YASL_State SO THAT FUNCTIONS CAN RUN PROPERLY
struct YASL_State {
    struct VM vm;
    struct Compiler compiler;
};

#endif
