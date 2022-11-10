#ifndef YASL_YASL_H_
#define YASL_YASL_H_

#include "yasl_conf.h"
#include "yasl_error.h"

#include <stdbool.h>
#include <stdlib.h>

#define YASL_VERSION "v0.12.3"

#define YASL_STR_NAME "str"
#define YASL_FLOAT_NAME "float"
#define YASL_TABLE_NAME "table"

struct YASL_State;

/**
 * Typedef for YASL functions defined through the C API.
 */
typedef int (*YASL_cfn)(struct YASL_State *);

/**
 * [-0, +0]
 * compiles the source for the given YASL_State, but doesn't
 * run it.
 * @param S the YASL_State containing the YASL source code to be compiled.
 * @return 0 on success, otherwise an error code.
 */
int YASL_compile(struct YASL_State *S);

/**
 * [-0, +0]
 * Declares a global for use in the given YASL_State.
 * @param S the YASL_State in which to declare the global.
 * @param name the name of the global (null-terminated string).
 * @return YASL_SUCCESS on success, otherwise an error code.
 */
int YASL_declglobal(struct YASL_State *S, const char *name);

int YASL_decllib_collections(struct YASL_State *S);
int YASL_decllib_error(struct YASL_State *S);
int YASL_decllib_io(struct YASL_State *S);
int YASL_decllib_math(struct YASL_State *S);
int YASL_decllib_require(struct YASL_State *S);
int YASL_decllib_require_c(struct YASL_State *S);
int YASL_decllib_mt(struct YASL_State *S);

/**
 * deletes the given YASL_State.
 * @param S YASL_State to be deleted.
 * @return 0 on success, otherwise an error code.
 */
int YASL_delstate(struct YASL_State *S);

/**
 * [-0, +1]
 * Duplicates the top of the stack.
 * @param S the YASL_State.
 * @return YASL_SUCCESS on success, otherwise an error code.
 */
int YASL_duptop(struct YASL_State *S);

/**
 * [-0, +0]
 * Execute the bytecode for the given YASL_State.
 * @param S the YASL_State to use to execute the bytecode.
 * @return 0 on successful execution, else an error code.
 */
int YASL_execute(struct YASL_State *S);

/**
 * [-0, +0]
 * Execute the bytecode for the given YASL_State in REPL mode. The only difference
 * between YASL_execute_REPL and YASL_execute is that YASL_execute_REPL will
 * print the last statement passed to it if that statement is an expression.
 * @param S the YASL_State to use to execute the bytecode.
 * @return 0 on successful execution, else an error code.
 */
int YASL_execute_REPL(struct YASL_State *S);

/**
 * [-(n+1), +r]
 * Calls a function with n parameters. The function should be located below all n
 * parameters it will be called with. The left-most parameter is placed directly above
 * the function, the right-most paramter at the top of the stack.
 * @param S the YASL_State
 * @param n
 * @return r, the number of return values of the called functions
 */
int YASL_functioncall(struct YASL_State *S, int n);

/**
 * [-0, +0]
 * checks if the top of the stack is bool.
 * @param S the YASL_State.
 * @return true if the top of the stack is bool, else false.
 */
bool YASL_isbool(struct YASL_State *S);

/**
 *
 * [-0, +0]
 * checks if the top of the stack is float.
 * @param S the YASL_State.
 * @return true if the top of the stack is float, else false.
 */
bool YASL_isfloat(struct YASL_State *S);

/**
 * [-0, +0]
 * checks if the top of the stack is int.
 * @param S the YASL_State.
 * @return true if the top of the stack is int, else false.
 */
bool YASL_isint(struct YASL_State *S);

/**
 * [-0, +0]
 * checks if the top of the stack is list.
 * @param S the YASL_State.
 * @return true if the top of the stack is list, else false.
 */
bool YASL_islist(struct YASL_State *S);

/**
 * [-0, +0]
 * checks if the object at index n is bool.
 * @param S the YASL_State.
 * @return true if the object at index n is bool, else false.
 */
bool YASL_isnbool(struct YASL_State *S, unsigned n);

/**
 *
 * [-0, +0]
 * checks if the object at index n is float.
 * @param S the YASL_State.
 * @return true if the object at index n is float, else false.
 */
bool YASL_isnfloat(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is int.
 * @param S the YASL_State.
 * @return true if the object at index n is int, else false.
 */
bool YASL_isnint(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is list.
 * @param S the YASL_State.
 * @return true if the object at index n is list, else false.
 */
bool YASL_isnlist(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is str.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the object at index n is str, else false.
 */
bool YASL_isnstr(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is table.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the object at index n is table, else false.
 */
bool YASL_isntable(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * Checks if the object at index n is undef.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the object at index n is undef, else false.
 */
bool YASL_isnundef(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is userdata.
 * @param S the YASL_State.
 * @return true if the object at index n is userdata, else false.
 */
bool YASL_isnuserdata(struct YASL_State *S, const char *tag, unsigned n);

/**
 * [-0, +0]
 * checks if the object at index n is userpointer.
 * @param S the YASL_State.
 * @return true if the object at index n is userpointer, else false.
 */
bool YASL_isnuserptr(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * checks if the top of the stack is str.
 * @param S the YASL_State.
 * @return true if the top of the stack is str, else false.
 */
bool YASL_isstr(struct YASL_State *S);

/**
 * [-0, +0]
 * checks if the top of the stack is table.
 * @param S the YASL_State.
 * @return true if the top of the stack is table, else false.
 */
bool YASL_istable(struct YASL_State *S);

/**
 * [-0, +0]
 * Checks if the top of the stack is undef.
 * @param S the YASL_State.
 * @return true if the top of the stack is undef, else false.
 */
bool YASL_isundef(struct YASL_State *S);

/**
 * [-0, +0]
 * checks if the top of the stack is userdata.
 * @param S the YASL_State.
 * @return true if the top of the stack is userdata, else false.
 */
bool YASL_isuserdata(struct YASL_State *S, const char *tag);

/**
 * [-0, +0]
 * checks if the top of the stack is userpointer.
 * @param S the YASL_State.
 * @return true if the top of the stack is userpointer, else false.
 */
bool YASL_isuserptr(struct YASL_State *S);

/**
 * [+1, -1]
 * Pops the top of the stack, the evalutes `len x`, where `x` is the popped value. The result is pushed on top
 * of the stack.
 * @param S the YASL_State.
 */
void YASL_len(struct YASL_State *S);

/**
 * [+1, -0]
 * Indexes the list on top of the stack and pushes the result on top of the stack.
 * @param S the YASL_State.
 * @param n the index to use on the list.
 * @return YASL_SUCCESS on
 */
int YASL_listget(struct YASL_State *S, yasl_int n);

/**
 * [-1, +0]
 * Pops the top of the stack and appends it to a list (which should be located directly below the top of the stack).
 * @param S the YASL_State.
 * @return YASL_SUCCESS on sucess, else an error code.
 */
int YASL_listpush(struct YASL_State *S);

/**
 * [-0, +1]
 * Loads a global and puts it on top of the stack.
 * @param S the YASL_State.
 * @param name nul-terminated name of the global.
 * @return YASL_SUCCESS on success, otherwise an error code.
 */
int YASL_loadglobal(struct YASL_State *S, const char *name);

/**
 * [-0, +1]
 * Loads a metatable by name.
 * @param S the YASL_State.
 * @param name name of the metatable.
 * @return YASL_SUCCESS on success, else an error code.
 */
int YASL_loadmt(struct YASL_State *S, const char *name);

void YASL_loadprintout(struct YASL_State *S);

void YASL_loadprinterr(struct YASL_State *S);

/**
 * Initialises a new YASL_State for usage.
 * @param filename the name of the file used to initialize the state.
 * @return the new YASL_State, or NULL on failure.
 */
struct YASL_State *YASL_newstate(const char *filename);

/**
 * initialises a new YASL_State for usage, or NULL on failure.
 * @param buf buffer containing the source code used to initialize
 * the state.
 * @param len the length of the buffer.
 * @return the new YASL_State
 */
struct YASL_State *YASL_newstate_bb(const char *buf, size_t len);

/**
 * [-0, +0]
 * Returns the bool value of the top of the stack, if it is a boolean.
 * Otherwise returns false. Does not modify the stack.
 * @param S the YASL_State.
 * @return
 */
bool YASL_peekbool(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the int value of the top of the stack, if the top of the stack is an int.
 * Otherwise returns 0. Does not modify the stack.
 * @param S the YASL_State.
 * @return the value of the int on top of the stack, or 0 if it's not an int.
 */
char *YASL_peekcstr(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the float value of the top of the stack, if the top of the stack is a float.
 * Otherwise returns 0.0. Does not modify the stack.
 * @param S the YASL_State.
 * @return
 */
yasl_float YASL_peekfloat(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the int value of the top of the stack, if the top of the stack is an int.
 * Otherwise returns 0. Does not modify the stack.
 * @param S the YASL_State.
 * @return the value of the int on top of the stack, or 0 if it's not an int.
 */
yasl_int YASL_peekint(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the bool value at index n, if it is a boolean.
 * Otherwise returns false. Does not modify the stack.
 * @param S
 * @return
 */
bool YASL_peeknbool(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * Returns the float value at index n, if it is a float.
 * Otherwise returns 0.0. Does not modify the stack.
 * @param S
 * @return
 */
yasl_float YASL_peeknfloat(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * Returns the int value at index n, if it is an int.
 * Otherwise returns 0. Does not modify the stack.
 * @param S the YASL_State.
 * @return the value of the int at index n, or 0 if it's not an int.
 */
yasl_int YASL_peeknint(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * Returns the userdata value at index n, if it is a userdata.
 * Otherwise returns NULL. Does not modify the stack.
 * @param S the YASL_State.
 * @return the userdata value at index n, or NULL if it's not a userdata.
 */
void *YASL_peeknuserdata(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * returns the type of index n as a string.
 * @param S the YASL_State.
 * @return the string representation of the type of index n.
 */
const char *YASL_peekntypename(struct YASL_State *S, unsigned n);

/**
 * [-0, +0]
 * returns the type of the top of the stack.
 * @param S the YASL_State.
 * @return the type on top of the stack.
 */
int YASL_peektype(struct YASL_State *S);

/**
 * [-0, +0]
 * returns the type of the top of the stack as a string.
 * @param S the YASL_State to which the stack belongs.
 * @return the string representation of the type on top of the stack.
 */
const char *YASL_peektypename(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the userdata value of the top of the stack, if the top of the stack is a userdata.
 * Otherwise returns NULL. Does not modify the stack.
 * @param S the YASL_State.
 * @return the userdata value on top of the stack, or NULL if it's not a userdata.
 */
void *YASL_peekuserdata(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the userptr value of the top of the stack, if the top of the stack is a userptr.
 * Otherwise returns NULL. Does not modify the stack.
 * @param S the YASL_State.
 * @return the value of the userptr on top of the stack, or NULL if it's not a userptr.
 */
void *YASL_peekuserptr(struct YASL_State *S);

yasl_int YASL_peekvargscount(struct YASL_State *S);

/**
 * [-1, +0]
 * Removes the top of the stack.
 * @param S the YASL_State the stack belongs to.
 */
void YASL_pop(struct YASL_State *S);

/**
 * [-1, +0]
 * Returns the bool value of the top of the stack, if the top of the stack is a boolean.
 * Otherwise returns false. Removes the top element of the stack.
 * @param S the YASL_State.
 * @return the bool value of the top of the stack.
 */
bool YASL_popbool(struct YASL_State *S);

/**
 * [-1, +0]
 * Returns the str value (nul-terminated) of the top of the stack, if the top of the stack is a str.
 * Otherwise returns NULL.
 * @param S the YASL_State.
 * @return the value of the str on top of the stack, or NULL if it's not a str.
 */
char *YASL_popcstr(struct YASL_State *S);

/**
 * [-1, +0]
 * Returns the float value of the top of the stack, if the top of the stack is a float.
 * Otherwise returns 0.0. Removes the top of the stack.
 * @param S the YASL_State.
 * @return the float value on top of the stack.
 */
yasl_float YASL_popfloat(struct YASL_State *S);

/**
 * [-1, +0]
 * Returns the int value of the top of the stack, if the top of the stack is an int.
 * Otherwise returns 0. Removes the top of the stack.
 * @param S the YASL_State.
 * @return the int value of the top of the stack.
 */
yasl_int YASL_popint(struct YASL_State *S);

void *YASL_popuserdata(struct YASL_State *S);

void *YASL_popuserptr(struct YASL_State *S);

/**
 * [-0, +0]
 * Prints a runtime error.
 * @param S the YASL_State in which the error occurred.
 * @param fmt a format string, taking the same parameters as printf.
 * @param ... var args for the above format strings.
 */
YASL_FORMAT_CHECK void YASL_print_err(struct YASL_State *S, const char *const fmt, ...);

/**
 * [-0, +1]
 * Pushes a boolean value onto the stack.
 * @param S the YASL_State onto which to push the boolean.
 * @param value boolean to be pushed onto the stack.
 */
void YASL_pushbool(struct YASL_State *S, bool value);

/**
 * [-0, +1]
 * Pushes a function pointer onto the stack
 * @param S the YASL_State onto which to push the string.
 * @param value the function pointer to be pushed onto the stack.
 */
void YASL_pushcfunction(struct YASL_State *S, YASL_cfn value, int num_args);

/**
 * [-0, +1]
 * Pushes a double value onto the stack.
 * @param S the YASL_State onto which to push the double.
 * @param value the float to push onto the stack.
 */
void YASL_pushfloat(struct YASL_State *S, yasl_float value);

/**
 * [-0, +1]
 * Pushes an integer value onto the stack.
 * @param S the YASL_State onto which to push the integer.
 * @param integer to be pushed onto the stack.
 */
void YASL_pushint(struct YASL_State *S, yasl_int value);

/**
 * [-0, +1]
 * Pushes a string with length len onto the stack. YASL makes a copy of the
 * given string, and manages the memory for it. The string may have embedded
 * 0s.
 * @param S the YASL_State.
 * @param value the string to be pushed onto the stack.
 * @param len the length of value.
 */
void YASL_pushlstr(struct YASL_State *S, const char *value, size_t len);

/**
 * [-0, +1]
 * Pushes an empty list onto the stack.
 * @param S the YASL_State onto which to push the list.
 */
void YASL_pushlist(struct YASL_State *S);

/**
 * [-0, +1]
 * Pushes a nul-terminated string onto the stack. This memory will not
 * be managed by YASL, and must outlive S.
 * @param S the YASL_State.
 * @param value nul-terminated string to be pushed onto the stack.
 */
void YASL_pushlit(struct YASL_State *S, const char *value);

/**
 * [-0, +1]
 * Pushes an empty table onto the stack.
 * @param S the YASL_State onto which to push the table.
 */
void YASL_pushtable(struct YASL_State *S);

/**
 * [-0, +1]
 * Pushes an undef value onto the stack.
 * @param S the YASL_State onto which to push the undef.
 */
void YASL_pushundef(struct YASL_State *S);

/**
 * [-0, +1]
 * Pushes a user-pointer onto the stack
 * @param S the YASL_State onto which to push the user-pointer.
 * @param userpointer the user-pointer to push onto the stack.
 */
void YASL_pushuserdata(struct YASL_State *S, void *data, const char *tag, void (*destructor)(struct YASL_State *, void *));

/**
 * [-0, +1]
 * Pushes a user-pointer onto the stack
 * @param S the YASL_State onto which to push the user-pointer.
 * @param userpointer the user-pointer to push onto the stack.
 */
void YASL_pushuserptr(struct YASL_State *S, void *userpointer);

/**
 * Pushes a nul-terminated string onto the stack. YASL makes a copy of
 * the given string, and manages the memory for it. The string may not
 * have embedded 0s; it is assumed to end at the first 0.
 * @param S the YASL_State.
 * @param value the string to be pushed onto the stack (nul-terminated).
 */
void YASL_pushzstr(struct YASL_State *S, const char *value);

/**
 * [-0, +0]
 * Registers a metatable with name `name`. After this returns, the
 * metatable can be referred to by `name` in other functions dealing
 * with metatables, e.g. `YASL_setmt` or `YASL_loadmt`.
 * @param S the YASL_State.
 * @param name the name of the metatable.
 * @return YASL_SUCCESS.
 */
int YASL_registermt(struct YASL_State *S, const char *name);

/**
 * resets S to the same state it would be in if newly created using
 * YASL_newstate.
 * @param S the YASL_State
 * @param filename the name of the file used to initialize S
 * @return YASL_SUCCESS if successful, otherwise an error.
 */
int YASL_resetstate(struct YASL_State *S, const char *filename);

/**
 * resets S to the same state it would be in if newly created using
 * YASL_newstate_bb.
 * @param S the YASL_State.
 * @param buf the buffer used to initialize S.
 * @param len the length of buf.
 * @return YASL_SUCCESS on success, else an error code.
 */
int YASL_resetstate_bb(struct YASL_State *S, const char *buf, size_t len);

/**
 * [-1, +0]
 * Pops the top of the YASL stack and stores it in the given global.
 * @param S the YASL_State in which to store the global.
 * @param name the name of the global.
 * @return YASL_SUCCESS on success, otherwise an error code.
 */
int YASL_setglobal(struct YASL_State *S, const char *name);

int YASL_setmt(struct YASL_State *S);

void YASL_setprintout_tostr(struct YASL_State *S);

void YASL_setprinterr_tostr(struct YASL_State *S);

void YASL_stringifytop(struct YASL_State *S);

/**
 * [-1, +2]
 * Iterates over a table. The topmost item of the stack should be the previous index in
 * the table, followed by the table itself. The index is popped, and then if there are
 * more elements in the table, the next index and value are pushed. No values are pushed
 * if we are already at the end of the table.
 * @param S the YASL_State.
 * @return true if there was a next element, otherwise false.
 */
bool YASL_tablenext(struct YASL_State *S);

/**
 * [-2, +0]
 * inserts a key-value pair into the table. The topmost
 * items is value, then key, then table. The key and value are popped from the stack.
 * @param S the YASL_State which has the 3 items on top of the stack.
 * @return 0 on success, else error code
 */
int YASL_tableset(struct YASL_State *S);

/**
 * [-0, +0]
 * Causes a fatal error.
 * @param S the YASL_State in which the error occured.
 * @param error the error code.
 */
YASL_NORETURN void YASL_throw_err(struct YASL_State *S, int error);

#endif
