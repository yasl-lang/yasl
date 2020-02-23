#ifndef YASL_YASL_H_
#define YASL_YASL_H_

#include "yasl_conf.h"
#include "yasl_error.h"

#include <stdbool.h>
#include <stdlib.h>

struct YASL_State;
struct YASL_Object;
struct YASL_Table;

/**
 * initialises a new YASL_State for usage, or NULL on failure.
 * @param filename the name of the file used to initialize the state.
 * @return the new YASL_State
 */
struct YASL_State *YASL_newstate(const char *filename);

int YASL_resetstate(struct YASL_State *S, const char *filename);

/**
 * initialises a new YASL_State for usage, or NULL on failure.
 * @param buf buffer containing the source code used to initialize
 * the state.
 * @param len the length of the buffer.
 * @return the new YASL_State
 */
struct YASL_State *YASL_newstate_bb(const char *buf, size_t len);

/**
 * resets S to the same state it would be in if newly created using
 * YASL_newstate_bb.
 * @param S the YASL_State
 * @param buf the buffer used to initialize S
 * @param len the length of buf.
 */
int YASL_resetstate_bb(struct YASL_State *S, const char *buf, size_t len);

/**
 * deletes the given YASL_State.
 * @param S YASL_State to be deleted.
 * @return 0 on success, otherwise an error code.
 */
int YASL_delstate(struct YASL_State *S);

/**
 * compiles the source for the given YASL_State, but doesn't
 * run it.
 * @param S the YASL_State containing the YASL source code to be compiled.
 * @return 0 on success, otherwise an error code.
 */
int YASL_compile(struct YASL_State *S);

/**
 * Execute the bytecode for the given YASL_State.
 * @param S the YASL_State to use to execute the bytecode.
 * @return 0 on successful execution, else an error code.
 */
int YASL_execute(struct YASL_State *S);
int YASL_execute_REPL(struct YASL_State *S);

/**
 * Declares a global for use in the given YASL_State.
 * @param S the YASL_State in which to declare the global.
 * @param name the name of the global (null-terminated string).
 * @return 0 on success, else an error code.
 */
int YASL_declglobal(struct YASL_State *S, const char *name);

/**
 * Pops the top of the YASL stack and stores it in the given global.
 * @param S the YASL_State in which to store the global.
 * @param name the name of the global.
 * @return 0 on success, else an error code.
 */
int YASL_setglobal(struct YASL_State *S, const char *name);

int YASL_loadglobal(struct YASL_State *S, const char *name);

/**
 * Pushes an undef value onto the stack.
 * @param S the YASL_State onto which to push the undef.
 * @return 0 on success, else error code.
 */
int YASL_pushundef(struct YASL_State *S);

/**
 * Pushes a double value onto the stack.
 * @param S the YASL_State onto which to push the double.
 * @param value
 * @return
 */
int YASL_pushfloat(struct YASL_State *S, yasl_float value);

/**
 * Pushes an integer value onto the stack.
 * @param S the YASL_State onto which to push the integer.
 * @param integer to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushinteger(struct YASL_State *S, yasl_int value);

/**
 * Pushes a boolean value onto the stack.
 * @param S the YASL_State onto which to push the boolean.
 * @param value boolean to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushboolean(struct YASL_State *S, int value);

/**
 * Pushes a null-terminated string onto the stack.
 * @param S the YASL_State onto which to push the string.
 * @param value null-terminated string to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushszstring(struct YASL_State *S, const char *value);

/**
 * Pushes a null-terminated string onto the stack. This memory will not
 * be managed by YASL.
 * @param S the YASL_State onto which to push the string.
 * @param value null-terminated string to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushlitszstring(struct YASL_State *S, const char *value);

/**
 * Pushes a string of given size onto the stack.
 * @param S the YASL_State onto which to push the string.
 * @param value string to be pushed onto the stack.
 * @param size size of string to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushstring(struct YASL_State *S, const char *value, const size_t size);

/**
 * Pushes a string of given size onto the stack. This memory will not
 * be managed by YASL.
 * @param S the YASL_State onto which to push the string.
 * @param value string to be pushed onto the stack.
 * @param size size of string to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushlitstring(struct YASL_State *S, const char *value, const size_t size);

/**
 * Pushes an empty table onto the stack.
 * @param S the YASL_State onto which to push the table.
 * @return 0 on success, else error code.
 */
int YASL_pushtable(struct YASL_State *S);

/**
 * Pushes an empty list onto the stack.
 * @param S the YASL_State onto which to push the list.
 * @return 0 on success, else error code.
 */
int YASL_pushlist(struct YASL_State *S);

/**
 * Pushes a function pointer onto the stack
 * @param S the YASL_State onto which to push the string.
 * @param value the function pointer to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *), int num_args);

/**
 * Pushes a user-pointer onto the stack
 * @param S the YASL_State onto which to push the user-pointer.
 * @param userpointer the user-pointer to push onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushuserdata(struct YASL_State *S, void *data, int tag, struct YASL_Table *mt, void (*destructor)(void *));

/**
 * Pushes a user-pointer onto the stack
 * @param S the YASL_State onto which to push the user-pointer.
 * @param userpointer the user-pointer to push onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushuserpointer(struct YASL_State *S, void *userpointer);

/**
 * Pushes an arbitrary YASL_Object onto the stack.
 * @param S the YASL_State onto which to push the YASL_Object.
 * @param obj the YASL_Object to push onto the stack.
 * @return 0 on succes, else error code.
 */
YASL_DEPRECATE
int YASL_pushobject(struct YASL_State *S, struct YASL_Object *obj);

/**
 * pops the top YASL_Object off of the stack.
 * @param S the YASL_State from which to pop the YASL_Object.
 * @return a YASL_Object* on succes, else NULL.
 */
YASL_DEPRECATE
struct YASL_Object *YASL_popobject(struct YASL_State *S);

/**
 * pops the top of the stack.
 * @param S the YASL_State the stack belongs to.
 * @return 0 on success, else error code.
 */
int YASL_pop(struct YASL_State *S);

int YASL_top_dup(struct YASL_State *S);

/**
 * Makes a new YASL_Table
 * @return the YASL_Table
 */
YASL_DEPRECATE
struct YASL_Object *YASL_Table(void);

YASL_DEPRECATE
struct YASL_Object *YASL_UserData(void *userdata, int tag, struct YASL_Table *mt, void (*destructor)(void *));
YASL_DEPRECATE
int YASL_UserData_gettag(struct YASL_Object *obj);
YASL_DEPRECATE
void *YASL_UserData_getdata(struct YASL_Object *obj);
YASL_DEPRECATE
struct YASL_Object *YASL_Function(int64_t index);
YASL_DEPRECATE
struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *), int num_args);

/**
 * inserts a key-value pair into the table. The topmost
 * items is value, then key, then table. All 3 are popped from the stack.
 * @param S the YASL_State which has the 3 items on top of the stack.
 * @return 0 on success, else error code
 */
int YASL_settable(struct YASL_State *S);

int YASL_pushlist(struct YASL_State *S);

/**
 * returns the type of the top of the stack.
 * @param S the YASL_State to which the stack belongs.
 * @return the type on top of the stack.
 */
int YASL_top_peektype(struct YASL_State *S);

/**
 * checks if the top of the stack is undef.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is undef, else false.
 */
bool YASL_top_isundef(struct YASL_State *S);

/**
 * checks if the top of the stack is bool.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is bool, else false.
 */
bool YASL_top_isboolean(struct YASL_State *S);

YASL_DEPRECATE
bool YASL_top_isdouble(struct YASL_State *S);

/**
 * checks if the top of the stack is float.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is float, else false.
 */
bool YASL_top_isfloat(struct YASL_State *S);

/**
 * checks if the top of the stack is int.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is int, else false.
 */
bool YASL_top_isinteger(struct YASL_State *S);

/**
 * checks if the top of the stack is str.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is str, else false.
 */
bool YASL_top_isstring(struct YASL_State *S);

/**
 * checks if the top of the stack is list.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is list, else false.
 */
bool YASL_top_islist(struct YASL_State *S);

/**
 * checks if the top of the stack is table.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is table, else false.
 */
bool YASL_top_istable(struct YASL_State *S);

/**
 * checks if the top of the stack is userdata.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is userdata, else false.
 */
bool YASL_top_isuserdata(struct YASL_State *S, int tag);

/**
 * checks if the top of the stack is userpointer.
 * @param S the YASL_State to which the stack belongs.
 * @return true if the top of the stack is userpointer, else false.
 */
bool YASL_top_isuserpointer(struct YASL_State *S);

bool YASL_top_peekboolean(struct YASL_State *S);
bool YASL_top_popboolean(struct YASL_State *S);

YASL_DEPRECATE
yasl_float YASL_top_peekdouble(struct YASL_State *S);
YASL_DEPRECATE
yasl_float YASL_top_popdouble(struct YASL_State *S);

yasl_float YASL_top_peekfloat(struct YASL_State *S);
yasl_float YASL_top_popfloat(struct YASL_State *S);

yasl_int YASL_top_peekinteger(struct YASL_State *S);
yasl_int YASL_top_popinteger(struct YASL_State *S);

char *YASL_top_peekcstring(struct YASL_State *S);
char *YASL_top_popcstring(struct YASL_State *S);

size_t YASL_getstringlen(struct YASL_Object *obj);

void *YASL_top_peekuserdata(struct YASL_State *S);
void *YASL_top_popuserdata(struct YASL_State *S);

void *YASL_top_peekuserpointer(struct YASL_State *S);
void *YASL_top_popuserpointer(struct YASL_State *S);

#endif
