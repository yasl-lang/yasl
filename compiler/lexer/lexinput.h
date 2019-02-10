#pragma once

#include "bytebuffer/bytebuffer.h"

typedef struct LEXINPUT LEXINPUT;

LEXINPUT *lexinput_new_file(FILE *lp);
LEXINPUT *lexinput_new_bb(char *buf, int len);
int lxgetc(LEXINPUT *lp);
int lxtell(LEXINPUT *lp);
int lxseek(LEXINPUT *lp, int w, int cmd);
int lxclose(LEXINPUT *lp);
int lxeof(LEXINPUT *lp);



/*
 0: 1: 2
*/
