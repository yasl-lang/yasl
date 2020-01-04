#pragma once

#include <stdio.h>

struct LEXINPUT;
struct LEXINPUT *lexinput_new_file(FILE *const lp);
struct LEXINPUT *lexinput_new_bb(const char *const buf, const size_t len);
int lxgetc(struct LEXINPUT *const lp);
int lxtell(struct LEXINPUT *const lp);
int lxseek(struct LEXINPUT *const lp, const int w, const int cmd);
int lxclose(struct LEXINPUT *const lp);
int lxeof(struct LEXINPUT *const lp);
