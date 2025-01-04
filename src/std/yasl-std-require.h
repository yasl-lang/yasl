#ifndef YASL_YASL_STD_REQUIRE_H_
#define YASL_YASL_STD_REQUIRE_H_

#include "yasl.h"

int YASL_decllib_require(struct YASL_State *S);
int YASL_decllib_require_c(struct YASL_State *S);
int YASL_decllib_eval(struct YASL_State *S);
int YASL_decllib_package(struct YASL_State *S);

#endif
