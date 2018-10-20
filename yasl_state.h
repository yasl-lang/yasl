#pragma once

struct Compiler;
struct VM;

struct YASL_State {
    struct Compiler *compiler;
    struct VM *vm;
};