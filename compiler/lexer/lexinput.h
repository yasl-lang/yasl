#pragma once

#include "bytebuffer/bytebuffer.h"

typedef struct LEXINPUT {
  FILE *fp;
  ByteBuffer *bb;
  int pos;
  int (*getc)(struct LEXINPUT *lp);
  int (*tell)(struct LEXINPUT *lp);
  int (*seek)(struct LEXINPUT *lp, int w, int cmd);
  int (*close)(struct LEXINPUT *lp);
  int (*eof)(struct LEXINPUT *lp);
} LEXINPUT;

LEXINPUT *lexinput_new_file(FILE *lp);
int lxgetc(LEXINPUT *lp);
int lxtell(LEXINPUT *lp);
int lxseek(LEXINPUT *lp, int w, int cmd);
int lxclose(LEXINPUT *lp);
int lxeof(LEXINPUT *lp);



/*
 0: 1: 2
*/
