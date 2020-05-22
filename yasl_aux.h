#ifndef YASL_YASL_AUX_H_
#define YASL_YASL_AUX_H_

/**
 * This file defines common wrappers around many common functions from yasl.h
 * It doesn't add any new functionality, just helps reduce boilerplate.
 */

#include "yasl.h"

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
 * Loads all standard libraries into the appropriate state and declares them all with their default names.
 * @param S The state onto which to load the libraries.
 * @return YASL_SUCESS on success, else error code.
 */
int YASLX_decllibs(struct YASL_State *S);

#endif
