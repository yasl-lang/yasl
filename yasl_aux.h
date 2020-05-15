#ifndef YASL_YASL_AUX_H_
#define YASL_YASL_AUX_H_

/**
 * This file defines common wrappers around many common functions from yasl.h
 * It doesn't add any new functionality, just helps reduce boilerplate.
 */

#include "yasl.h"

void YASL_print_err_bad_arg_type(struct YASL_State *S,
				 const char *const fn_name,
				 int position,
				 const char *const exp,
				 const char *const act);

#endif
