#pragma once

struct Compiler;
struct VM;

struct YASL_State {
    struct VM *vm;
    struct Compiler *compiler;
};