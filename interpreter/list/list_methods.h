#pragma once

struct VM;

int list___get(struct VM *vm);

int list___set(struct VM *vm);

int list_push(struct VM *vm);

int list_copy(struct VM *vm);

int list_extend(struct VM *vm);

int list_pop(struct VM* vm);

int list_search(struct VM *vm);

int list_reverse(struct VM *vm);
