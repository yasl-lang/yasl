#pragma once

/*
 * Definition of all YASL error codes.
 */

enum YASL_Error {
	YASL_SUCCESS,              // Successful execution.
	YASL_MODULE_SUCCESS,       // Successfully executed as module.
	YASL_ERROR,                // Generic error.
	YASL_INIT_ERROR,           // YASL_State has not been correctly initialised.
	YASL_SYNTAX_ERROR,         // Syntax error during compilation.
	YASL_TYPE_ERROR,           // Type error (at runtime).
	YASL_DIVIDE_BY_ZERO_ERROR, // Division by zero error (at runtime).
	YASL_VALUE_ERROR,          // Invalid value (at runtime).
	YASL_TOO_MANY_VAR_ERROR,   // Too many variables in current scope
	YASL_PLATFORM_NOT_SUPP     // Platform specific code not supported for this platform.
};
