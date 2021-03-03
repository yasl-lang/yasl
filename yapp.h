#ifndef YASL_YAPP_H_
#define YASL_YAPP_H_

/*
 * This header defines a bunch of useful macros for pre-processor operations, used throughout the YASL implementation.
 */

// YAPP_EXPAND is mainly to deal with MSVC bullshit.
#define YAPP_EXPAND(arg) arg

// The YAPP_CHOOSE? macros are for allowing variadic function-like macros to be defined.
#define YAPP_CHOOSE2(...) YAPP_EXPAND(YAPP_XCHOOSE2(__VA_ARGS__))
#define YAPP_CHOOSE4(_0, _1, _2, _3, NAME, ...) NAME
#define YAPP_CHOOSE6(_0, _1, _2, _3, _4, _5, NAME, ...) NAME

#define YAPP_XCHOOSE2(_0, _1, NAME, ...) NAME

#endif
