#pragma once

#include "yasl_error.h"
#include "inttypes.h"

struct YASL_State;
struct YASL_Object;

/**
 * initialises a new YASL_State for usage, or NULL on failure.
 * @return the new YASL_State
 */
struct YASL_State *YASL_newstate(char *filename);

/**
 * deletes the given YASL_State.
 * @param S YASL_State to be deleted.
 * @return 0 on success, otherwise an error code.
 */
int YASL_delstate(struct YASL_State *S);

/**
 * Execute the bytecode for the given YASL_State.
 * @param S the YASL_State to use to execute the bytecode.
 * @return 0 on successful execution, else an error code.
 */
int YASL_execute(struct YASL_State *S);

/**
 * Declares a global for use in the given YASL_State.
 * @param S the YASL_State in which to declare the global.
 * @param name the name of the global (null-terminated string).
 * @return 0 on success, else an error code.
 */
int YASL_declglobal(struct YASL_State *S, char *name);

/**
 * Pops the top of the YASL stack and stores it in the given global.
 * @param S the YASL_State in which to store the global.
 * @param name the name of the global.
 * @return 0 on success, else an error code.
 */
int YASL_setglobal(struct YASL_State *S, char *name);

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
int YASL_pushfloat(struct YASL_State *S, double value);

/**
 * Pushes an integer value onto the stack.
 * @param S the YASL_State onto which to push the integer.
 * @param integer to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushinteger(struct YASL_State *S, int64_t value);

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
int YASL_pushcstring(struct YASL_State *S, char *value);

/**
 * Pushes a string of given size onto the stack.
 * @param S the YASL_State onto which to push the string.
 * @param value string to be pushed onto the stack.
 * @param size size of string to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushstring(struct YASL_State *S, char *value, int64_t size);

/**
 * Pushes a function pointer onto the stack
 * @param S the YASL_State onto which to push the string.
 * @param value the function pointer to be pushed onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushcfunction(struct YASL_State *S, int (*value)(struct YASL_State *));

/**
 * Pushes a user-pointer onto the stack
 * @param S the YASL_State onto which to push the user-pointer.
 * @param userpointer the user-pointer to push onto the stack.
 * @return 0 on success, else error code.
 */
int YASL_pushuserpointer(struct YASL_State *S, void *userpointer);

/**
 * Pushes an arbitrary YASL_Object onto the stack.
 * @param S the YASL_State onto which to push the user-pointer.
 * @param obj the YASL_Object to push onto the stack.
 * @return 0 on succes, else error code.
 */
int YASL_pushobject(struct YASL_State *S, struct YASL_Object *obj);

/**
 * Makes a new YASL_Table
 * @return the YASL_Table
 */
struct YASL_Object *YASL_Table(void);

struct YASL_Object *YASL_Integer(int64_t);
struct YASL_Object *YASL_Undef(void);
struct YASL_Object *YASL_Float(double value);
struct YASL_Object *YASL_Boolean(int value);
struct YASL_Object *YASL_UserPointer(void *userdata);
struct YASL_Object *YASL_Function(int64_t index);
struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *));

/**
 * inserts a key-value pair into the given table
 * @param table the YASL_Table into which to insert the key-value pair
 * @param key the key of the key-value pair
 * @param value the value of the key-value pair
 * @return 0 on success, else error code
 */
int YASL_Table_set(struct YASL_Object *table, struct YASL_Object *key, struct YASL_Object *value);

/**
 * Checks if given YASL_Object is undef.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is undef, else false.
 */
//int YASL_isundef(YASL_Object *obj);

/**
 * Checks if given YASL_Object is boolean.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is boolean, else false.
 */
//int YASL_isboolean(YASL_Object *obj);

/**
 * Checks if given YASL_Object is double.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is double, else false.
 */
//int YASL_isdouble(YASL_Object *obj);

/**
 * Checks if given YASL_Object is integer.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is integer, else false.
 */
//int YASL_isinteger(YASL_Object *obj);

/**
 * Checks if given YASL_Object is string.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is string, else false.
 */
//int YASL_isstring(YASL_Object *obj);

/**
 * Checks if given YASL_Object is list.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is list, else false.
 */
//int YASL_islist(YASL_Object *obj);

/**
 * Checks if given YASL_Object is table.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is table, else false.
 */
//int YASL_istable(YASL_Object *obj);

/**
 * Checks if given YASL_Object is function.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is function, else false.
 */
//int YASL_isfunction(YASL_Object *obj);

/**
 * Checks if given YASL_Object is C function.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is a C function, else false.
 */
//int YASL_iscfunction(YASL_Object *obj);

/**
 * Checks if given YASL_Object is userdata.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is userdata, else false.
 */
//int YASL_isuserdata(YASL_Object *obj);

/**
 * Checks if given YASL_Object is userpointer.
 * @param obj the given YASL_Object.
 * @return true if the given YASL_Object is userpointer, else false.
 */
//int YASL_isuserpointer(YASL_Object *obj);

/**
 * Retrieves the boolean value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the boolean value of the given YASL_Object, or false if the YASL_Object doesn't have type bool.
 */
//int YASL_getboolean(YASL_Object *obj);

/**
 * Retrieves the double value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the double value of the given YASL_Object, or 0.0 if the YASL_Object doesn't have type double.
 */
//double YASL_getdouble(YASL_Object *obj);

/**
 * Retrieves the integer value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the integer value of the given YASL_Object, or 0 if the YASL_Object doesn't have type integer.
 */
//int64_t YASL_getinteger(YASL_Object *obj);

/**
 * Retrieves the null-terminated string value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the null-terminated string value of the given YASL_Object, or NULL if the YASL_Object doesn't have type string.
 */
//char *YASL_getcstring(YASL_Object *obj);

/**
 * Retrieves the string value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the string value of the given YASL_Object, or NULL if the YASL_Object doesn't have type string.
 */
//char *YASL_getstring(YASL_Object *obj);

/**
 * Retrieves the C function value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the C function value of the given YASL_Object, or NULL if the YASL_Object doesn't have type C function.
 */
//int (*)(struct YASL_State) *YASL_getcfunction(YASL_Object *obj);

/**
 * Retrieves the userdata value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the userdata value of the given YASL_Object, or NULL if the YASL_Object doesn't have type userdata.
 */
//void *YASL_getuserdata(YASL_Object *obj);

/**
 * Retrieves the userpointer value of the YASL_Object.
 * @param obj the given YASL_Object.
 * @return the userpointer value of the given YASL_Object, or NULL if the YASL_Object doesn't have type userpointer.
 */
//void *YASL_getuserpointer(YASL_Object *obj);