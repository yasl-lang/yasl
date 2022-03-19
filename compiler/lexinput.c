#include "lexinput.h"

#include "data-structures/YASL_ByteBuffer.h"

#include <stdbool.h>

#undef getc

struct LEXINPUT {
  FILE *fp;
  YASL_ByteBuffer *bb;
  size_t pos;
  bool iseof;
  int (*getc)(struct LEXINPUT *const lp);
  int (*tell)(struct LEXINPUT *const lp);
  int (*seek)(struct LEXINPUT *const lp, int w, int cmd);
  int (*close)(struct LEXINPUT *const lp);
  int (*eof)(struct LEXINPUT *const lp);
};


int lxgetc(struct LEXINPUT *const lp) {
	int ch = lp->getc(lp);
	return ch;
}

int lxtell(struct LEXINPUT *const lp) {
	int d = lp->tell(lp);
	return d;
}

int lxseek(struct LEXINPUT *const lp, const int w, const int cmd) {
	int r = lp->seek(lp, w, cmd);
	return r;
}

int lxclose(struct LEXINPUT *const lp) {
	return lp->close(lp);
}

int lxeof(struct LEXINPUT *const lp) {
	return lp->eof(lp);
}

static  int lexinput_file_getc(struct LEXINPUT *const lp) {
	return fgetc(lp->fp);
}

static  int lexinput_file_tell(struct LEXINPUT *const lp) {
	return ftell(lp->fp);
}

static  int lexinput_file_seek(struct LEXINPUT *const lp, int w, int cmd) {
	return fseek(lp->fp, w, cmd);
}

static  int lexinput_file_eof(struct LEXINPUT *const lp) {
	return feof(lp->fp);
}

static  int lexinput_file_close(struct LEXINPUT *const lp) {
	fclose(lp->fp);
	lp->fp = 0;
	free(lp);
	return 0;
}

struct LEXINPUT *lexinput_new_file(FILE *const fp) {
	struct LEXINPUT *lp = (struct LEXINPUT *)malloc(sizeof(struct LEXINPUT));
	lp->fp = fp;
	lp->getc = lexinput_file_getc;
	lp->tell = lexinput_file_tell;
	lp->seek = lexinput_file_seek;
	lp->close = lexinput_file_close;
	lp->eof = lexinput_file_eof;
	return lp;
}

#include "data-structures/YASL_ByteBuffer.h"

static int lexinput_bb_eof(struct LEXINPUT *const lp);
static  int lexinput_bb_getc(struct LEXINPUT *const lp) {
	if (lp->pos >= (signed) lp->bb->count) {
		lp->iseof = 1;
		return -1;
	}
	return lp->bb->items[lp->pos++];
}

static  int lexinput_bb_tell(struct LEXINPUT *const lp) {
	return lp->pos;
}

static  int lexinput_bb_seek(struct LEXINPUT *const lp, int w, int cmd) {
	if (cmd == 0) {
		lp->pos = w;
	} else if (cmd == 1) {
		lp->pos += w;
	} else if (cmd == 2) {
		lp->pos = lp->bb->count = w;
	}
	if (lp->pos < (signed) lp->bb->count) lp->iseof = 0;
	return 0;
}

static  int lexinput_bb_eof(struct LEXINPUT *const lp) {
	if (lp->pos >= (signed) lp->bb->count) {
		return lp->iseof;
	}
	return 0;
}

static  int lexinput_bb_close(struct LEXINPUT *const lp) {
	YASL_ByteBuffer_del(lp->bb);
	lp->bb = 0;
	free(lp);
	return 0;
}

struct LEXINPUT *lexinput_new_bb(const char *const buf, const size_t len) {
	struct LEXINPUT *lp = (struct LEXINPUT *) malloc(sizeof(struct LEXINPUT));
	lp->bb = YASL_ByteBuffer_new(8);
	YASL_ByteBuffer_extend(lp->bb, (unsigned char *) buf, len);
	lp->getc = lexinput_bb_getc;
	lp->tell = lexinput_bb_tell;
	lp->seek = lexinput_bb_seek;
	lp->close = lexinput_bb_close;
	lp->eof = lexinput_bb_eof;
	lp->iseof = 0;
	lp->pos = 0;
	return lp;
}
