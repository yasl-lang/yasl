#pragma once

/*
 * Definition of all YASL error codes.
 */

enum YASL_Error {
    YASL_SUCCESS,       // Successful execution.
    YASL_ERROR,         // Generic error.
    YASL_INIT_ERROR,    // YASL_State has not been correctly initialised.
    YASL_SYNTAX_ERROR,  // Syntax error during compilation.
};