#include "lexinput.h"

int lxgetc(LEXINPUT *lp)
{
#undef getc
    return lp->getc(lp);
}

int lxtell(LEXINPUT *lp)
{
    return lp->tell(lp);
}

int lxseek(LEXINPUT *lp, int w, int cmd)
{
    return lp->seek(lp, w, cmd);
}

int lxclose(LEXINPUT *lp)
{
    return lp->close(lp);
}
int lxeof(LEXINPUT *lp)
{
    return lp->eof(lp);
}

static  int lexinput_file_getc(LEXINPUT *lp)
{
    return fgetc(lp->fp);
}
static  int lexinput_file_tell(LEXINPUT *lp)
{
    return ftell(lp->fp);
}
static  int lexinput_file_seek(LEXINPUT *lp, int w, int cmd)
{
    return fseek(lp->fp, w, cmd);
}
static  int lexinput_file_eof(LEXINPUT *lp)
{
    return feof(lp->fp);
}
static  int lexinput_file_close(LEXINPUT *lp)
{
    fclose(lp->fp);
    lp->fp = 0;
    return 0;
}

LEXINPUT *lexinput_new_file(FILE *fp)
{
    LEXINPUT *lp = (LEXINPUT *)malloc(sizeof(LEXINPUT));
    lp->fp = fp;
    lp->getc = lexinput_file_getc;
    lp->tell = lexinput_file_tell;
    lp->seek = lexinput_file_seek;
    lp->close = lexinput_file_close;
    lp->eof = lexinput_file_eof;
    return lp;
}

#include "bytebuffer/bytebuffer.h"

static  int lexinput_bb_getc(LEXINPUT *lp)
{
    return lp->bb->bytes[lp->pos++];
}
static  int lexinput_bb_tell(LEXINPUT *lp)
{
    return lp->pos;
}
static  int lexinput_bb_seek(LEXINPUT *lp, int w, int cmd)
{
    if (cmd == 0) {
        lp->pos = w;
    }
    else if (cmd == 1) {
        lp->pos += w;
    }
    else if (cmd == 2) {
        lp->pos = lp->bb->count = w;
    }
    return 0;
}
static  int lexinput_bb_eof(LEXINPUT *lp)
{
    return (unsigned int)lp->pos >= lp->bb->count;
}
static  int lexinput_bb_close(LEXINPUT *lp)
{
// should free bb
    lp->bb = 0;
    return 0;
}

LEXINPUT *lexinput_new_bb(char *buf, int len)
{
    LEXINPUT *lp = (LEXINPUT *)malloc(sizeof(LEXINPUT));
    lp->bb = bb_new(len);
    bb_append(lp->bb, (unsigned char *)buf, len);
    lp->getc = lexinput_bb_getc;
    lp->tell = lexinput_bb_tell;
    lp->seek = lexinput_bb_seek;
    lp->close = lexinput_bb_close;
    lp->eof = lexinput_bb_eof;
    return lp;
}
