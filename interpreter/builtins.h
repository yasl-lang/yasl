#pragma once

struct VM;

struct YASL_Table *undef_builtins(struct VM *vm);
struct YASL_Table *float_builtins(struct VM *vm);
struct YASL_Table *int_builtins(struct VM *vm);
struct YASL_Table *bool_builtins(struct VM *vm);
struct YASL_Table *str_builtins(struct VM *vm);
struct YASL_Table *list_builtins(struct VM *vm);
struct YASL_Table *table_builtins(struct VM *vm);
