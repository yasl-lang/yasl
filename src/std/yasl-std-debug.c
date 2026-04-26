#include "yasl-std-debug.h"

#include "yasl_aux.h"
#include "yasl_state.h"

#include "VM.h"

int YASL_debug_backtrace(struct YASL_State *S) {
	vm_debug_echobacktrace((struct VM *) S);
	return 0;
}

int YASL_debug_getglobal(struct YASL_State *S) {
	return vm_debug_getglobal((struct VM *)S);
}

int YASL_debug_setglobal(struct YASL_State *S) {
	return vm_debug_setglobal((struct VM *)S);
}

int YASL_decllib_debug(struct YASL_State *S) {
	YASL_pushtable(S);
	YASLX_initglobal(S, "debug");

	YASL_loadglobal(S, "debug");

	struct YASLX_function functions[] = {
		/* { "break", &YASL_debug_break, 0 }, */
		{ "echobacktrace", &YASL_debug_backtrace, 0 },
		{ "getglobal", &YASL_debug_getglobal, 1 },
		/*
		{ "getlocal", &YASL_debug_getlocal, 1 },
		{ "getupval", &YASL_debug_getupval, 1 },
		 */
		{ "setglobal", &YASL_debug_setglobal, 2 },
		/*
		{ "setlocal", &YASL_debug_setlocal, 2 },
		{ "setupval", &YASL_debug_setupval, 2 },
		 */
		{ NULL,	NULL, 0 }
	};

	YASLX_tablesetfunctions(S, functions);
	YASL_pop(S);

	return YASL_SUCCESS;
}