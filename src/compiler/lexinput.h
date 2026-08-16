#ifndef YASL_LEXINPUT_H_
#define YASL_LEXINPUT_H_

#include <stdio.h>

struct LEXINPUT;
struct LEXINPUT *lexinput_new_file(FILE *const lp);
struct LEXINPUT *lexinput_new_bb(const char *const buf, const size_t len);
int lxgetc(struct LEXINPUT *const lp);
long lxtell(struct LEXINPUT *const lp);
int lxseek(struct LEXINPUT *const lp, const long w, const int cmd);
int lxclose(struct LEXINPUT *const lp);
int lxeof(struct LEXINPUT *const lp);

#endif
