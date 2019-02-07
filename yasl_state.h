#pragma once

#include "compiler.h"
#include "VM.h"

// VM MUST BE FIRST ITEM IN YASL_State SO THAT FUNCTIONS CAN RUN PROPERLY
struct YASL_State {
    struct VM vm;
    struct Compiler compiler;
};