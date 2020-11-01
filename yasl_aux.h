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
 * Prints an error message
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
 * Use YASLX_print_err_bad_arg_type instead.
 */
YASL_DEPRECATE
void YASL_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act);

/**
 * Returns the top of the stack if it is an int. Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument, used only for the error message.
 * @return the top of the stack if it's an int, otherwise no return.
 */
yasl_int YASLX_checkint(struct YASL_State *S, const char *name, int pos);

/**
 * Returns the top of the stack if it is a float. Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument, used only for the error message.
 * @return the top of the stack if it's a float, otherwise no return.
 */
yasl_float YASLX_checkfloat(struct YASL_State *S, const char *name, int pos);

/**
 * Returns the top of the stack if it is a bool. Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument, used only for the error message.
 * @return the top of the stack if it's a bool, otherwise no return.
 */
bool YASLX_checkbool(struct YASL_State *S, const char *name, int pos);

/**
 * Pops the top of the stack if it is undef. Otherwise, causes a type error, along with a printed error message.
 * @param S The YASL_State.
 * @param name Name of the function in which this is called, used for error message.
 * @param pos the position of the argument, used only for the error message.
 */
void YASLX_checkundef(struct YASL_State *S, const char *name, int pos);

#endif
