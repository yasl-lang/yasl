#pragma once

#include "bytebuffer/bytebuffer.h"

struct LEXINPUT;
struct LEXINPUT *lexinput_new_file(FILE *lp);
struct LEXINPUT *lexinput_new_bb(char *buf, size_t len);
int lxgetc(struct LEXINPUT *lp);
int lxtell(struct LEXINPUT *lp);
int lxseek(struct LEXINPUT *lp, int w, int cmd);
int lxclose(struct LEXINPUT *lp);
int lxeof(struct LEXINPUT *lp);
