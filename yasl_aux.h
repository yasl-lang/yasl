#ifndef YASL_YASL_AUX_H_
#define YASL_YASL_AUX_H_

/**
 * This file defines common wrappers around many common functions from yasl.h
 * It doesn't add any new functionality, just helps reduce boilerplate.
 */

#include "yasl.h"

/**
 * Loads all standard libraries into the appropriate state and declares them all with their default names.
 * @param S The state onto which to load the libraries.
 * @return YASL_SUCESS on success, else error code.
 */
int YASLX_decllibs(struct YASL_State *S);

/**
 * [-1, +0]
 * Declares a global and initializes it with the top value from the stack.
 * @param S the YASL_State.
 * @param name name of the global (nul-terminated).
 */
void YASLX_initglobal(struct YASL_State *S, const char *name);

/**
 * [-0, +0]
 * Prints an error message for bad value.
 * @param S The YASL State.
 * @param fmt a format string, taking the same parameters as printf.
 * @param ... var args for the above format string.
 */
#define YASLX_print_value_err(S, ...) YASL_print_err(S, MSG_VALUE_ERROR __VA_ARGS__)

/**
 * [-0, +0]
 * Prints an error message for bad argument type.
 * @param S The YASL State.
 * @param fn_name name of the function in which the error occured.
 * @param position which arg had the wrong type.
 * @param exp expected type of the arg.
 * @param act actual type of the arg.
 */
void YASLX_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act);

/**
 * [-0, +0]
 * Prints an error message
 * @param S The YASL State.
 * @param fn_name name of the function in which the error occured.
 * @param position which arg had the wrong type.
 * @param exp expected type of the arg.
 */
void YASLX_print_err_bad_arg_type_n(struct YASL_State *S,
				const char *const fn_name,
				unsigned position,
				const char *exp);

/**
 * [-0, +0]
 * Causes a fatal value error.
 * @param S the YASL_State in which the error occured.
 */
YASL_NORETURN void YASLX_throw_value_err(struct YASL_State *S);

/**
 * [-0, +0]
 * Causes a fatal value error.
 * @param S the YASL_State in which the error occured.
 */
YASL_NORETURN void YASLX_throw_type_err(struct YASL_State *S);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is an int. Otherwise, causes a type error,
 * along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument.
 * @return the nth position of the stack if it's an int, otherwise no return.
 */
yasl_int YASLX_checknint(struct YASL_State *S, const char *name, unsigned n);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is a float. Otherwise, causes a type error, along with a printed
 * error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument.
 * @return the nth position of the stack if it's a float, otherwise no return.
 */
yasl_float YASLX_checknfloat(struct YASL_State *S, const char *name, unsigned n);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is a bool. Otherwise, causes a type error, along with a printed
 * error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument.
 * @return the nth position of the stack if it's a bool, otherwise no return.
 */
bool YASLX_checknbool(struct YASL_State *S, const char *name, unsigned n);

/**
 * [-0, +0]
 * Checks that the nth position of the stack if it is undef. Otherwise, causes a type error, along with a printed
 * error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument.
 */
void YASLX_checknundef(struct YASL_State *S, const char *name, unsigned n);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is a str. Otherwise, causes a type error, along with a printed
 * error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument.
 * @return the nth position of the stack if it's a str, otherwise no return.
 */
const char *YASLX_checknstr(struct YASL_State *S, const char *name, unsigned n, size_t *len);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is a userdata with a matching tag. Otherwise, causes a type error,
 * along with a printed error message.
 * @param S The YASL_State.
 * @param tag The tag of the userdata.
 * @param name Name of the function in which this is called, used for error message.
 * @param n the position of the argument, used only for the error message.
 * @return the nth position of the stack if it's a userdata, otherwise no return.
 */
void *YASLX_checknuserdata(struct YASL_State *S, const char *tag, const char *name, unsigned n);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is an int, or default_val if the nth position is undef.
 * Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param n the position of the argument.
 * @param default_val the value to return if the nth position of the stack is undef.
 * @return the nth position of the stack or a default value if it's an int or undef, otherwise no return.
 */
yasl_int YASLX_checknoptint(struct YASL_State *S, const char *name, unsigned n, yasl_int default_val);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is a float, or default_val if the nth position is undef.
 * Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param n the position of the argument.
 * @param default_val the value to return if the nth position of the stack is undef.
 * @return the nth position of the stack or a default value if it's an float or undef, otherwise no return.
 */
yasl_float YASLX_checknoptfloat(struct YASL_State *S, const char *name, unsigned n, yasl_float default_val);

/**
 * [-0, +0]
 * Returns the nth position of the stack if it is an bool, or default_val if the nth position is undef.
 * Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param n the position of the argument.
 * @param default_val the value to return if the nth position of the stack is undef.
 * @return the nth position of the stack or a default value if it's an bool or undef, otherwise no return.
 */
bool YASLX_checknoptbool(struct YASL_State *S, const char *name, unsigned n, bool default_val);

struct YASLX_function {
	const char *name;
	YASL_cfn fn;
	int args;
};

/**
 * Inserts all functions in the array into a table on top of the stack.
 * @param S The YASL_State
 * @param functions array of function names, function pointers, and number of args.
 */
void YASLX_tablesetfunctions(struct YASL_State *S, const struct YASLX_function functions[]);

#endif
