#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _DEFAULT_SOURCE
#include "test/test_util.h"
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>

#define BUFSIZE 256

typedef struct {
  char *buf;
  size_t size, pos;
} bufr;

#define NEWBUF ((bufr){.buf = (char *)malloc(BUFSIZE * 2), \
                       .size = BUFSIZE * 2, .pos = 0})

static unsigned char clen[256];
static unsigned char cfin[256];

static void setup(void) {
	static bool done = false;
	if (done) {
		return;
	}
	done = true;
	for (int c = 0; c < 256; c++) {
	  	if (c < ' ') {
			clen[c] = 3;
			cfin[c] = c | 128;
		} else {
			clen[c] = 1;
			cfin[c] = c;
		}
	}
	clen[127] = 3;
	cfin[127] = 0xA1;
}

static char *convert(const char *inp, size_t sz) {
	printf("sz: %ld\n", sz);
	size_t i = 1;
	setup();
	const unsigned char *c = (const unsigned char *)inp;
	for (size_t j = 0; j < sz; j++) {
		i += clen[(int)c[j]];
	}
	char *res = (char *)malloc(i);
	unsigned char *p = (unsigned char *)res;
	for (size_t j = 0; j < sz; j++) {
		switch (clen[(int)c[j]]) {
		case 1:
			break;
		case 3:
			//*(p++) = 0xE2;
			//*(p++) = 0x90;
			break;
		default:
			fprintf(stderr, "Memory corruption detected.\n");
			exit(1);
		}
		*(p++) = cfin[(int)c[j]];
	}
	*p = 0;
	return res;
}

static bool readsome(bufr *b, int fd, bool *noread) {
	ssize_t bytes = read(fd, b->buf + b->pos, BUFSIZE);
	if (~bytes) { // No error? continue.
		*noread = false;
		if (bytes) { // Read anything?
			b->pos += bytes;
			if (b->size - b->pos > BUFSIZE) {
				b->buf = (char *)realloc(b->buf, b->size *= 2);
			}
		}
	}
	return bytes != 0;
}

static void pipey(int pipefd[2]) {
	pipe(pipefd);
	for(int i = 0; i < 2; i++) {
		fcntl(pipefd[i], F_SETFL,
		      fcntl(pipefd[i], F_SETFL) | O_NONBLOCK);
		fcntl(pipefd[i], F_SETFD,
		      fcntl(pipefd[i], F_GETFD) | FD_CLOEXEC);
	}
}

int qx(const char *prog, const char *const *args,
       size_t *olen, char **output, size_t *elen, char **error) {
	int opipe_fd[2], epipe_fd[2];
	bufr ob, eb;
	pid_t child;
	bool oopen = output != NULL, eopen = error != NULL;

	if (oopen) {
		pipey(opipe_fd);
	} else {
		opipe_fd[1] = open("/dev/null", O_WRONLY | O_CLOEXEC);
	}
	if (eopen) {
		pipey(epipe_fd);
	} else {
		epipe_fd[1] = open("/dev/null", O_WRONLY | O_CLOEXEC);
	}
	if (!(child = fork())) {
		int argc;
		for (argc = 0; args[argc]; argc++);
		char **argv = (char **)malloc((argc + 2) * sizeof(char *));
		do {
			argv[argc + 1] = (char *)args[argc];
		} while (argc--);
		*argv = basename(strdup(prog));
		dup2(opipe_fd[1], 1);
		dup2(epipe_fd[1], 2);
		execvp(prog, argv);
		exit(1);
	}
	close(opipe_fd[1]);
	close(epipe_fd[1]);
	if (oopen) {
		ob = NEWBUF;
	}
	if (eopen) {
		eb = NEWBUF;
	}
	do {
       		bool noluck = true;
		oopen = oopen && readsome(&ob, *opipe_fd, &noluck);
		eopen = eopen && readsome(&eb, *epipe_fd, &noluck);
		if (noluck) {
			sched_yield();
		}
	} while (oopen || eopen);
	int status;
	waitpid(child, &status, 0);
	if (output != NULL) {
		*output = (char *)realloc(ob.buf, *olen = ob.pos);
		close(*opipe_fd);
	}
	if (error != NULL) {
		*error = (char *)realloc(eb.buf, *elen = eb.pos);
		close(*epipe_fd);
	}
	return WEXITSTATUS(status);
}

#define B "\x1B[31m"
#define E "\x1B[0m\n"
#define PLAINT_BEG B "%s assert failed in %s (line %d):"
#define PLAINT_STR PLAINT_BEG E "%s" B "\n\xE2\x89\xA0" E "%s\n"
#define PLAINT_INT PLAINT_BEG " %d \xE2\x89\xA0 %d" E

bool assert_output(const char *prog, const char *const *args,
		   size_t eolen, const char *expected_output,
		   size_t eelen, const char *expected_error,
		   int expected_status,
		   const char *file, int line) {
	char *actual_output, *actual_error;
	size_t aolen, aelen;
	int actual_status;
	bool differences = false;
	actual_status = qx(prog, args,
			   &aolen,
			   expected_output == NULL ? NULL : &actual_output,
			   &aelen,
			   expected_error == NULL ? NULL : &actual_error);
	if (expected_output != NULL) {
		if (aolen != eolen
		    || memcmp(expected_output, actual_output, eolen)) {
			char *ec = convert(expected_output, eolen);
			char *ac = convert(actual_output, aolen);
			printf(PLAINT_STR, "output", file, line, ec, ac);
			free(ec);
			free(ac);
			differences = true;
		}
		free(actual_output);
	}
	if (expected_error != NULL) {
		if (aelen != eelen
		    || memcmp(expected_error, actual_error, eelen)) {
			printf("%ld, %ld\n", aelen, eelen );
			char *ec = convert(expected_error, eelen);
			char *ac = convert(actual_error, aelen);
			printf("%x\n", ac[aelen-1]);
			printf(PLAINT_STR, "error", file, line, ec, ac);
			free(ec);
			free(ac);
			differences = true;
		}
		free(actual_error);
	}
	if (expected_status != actual_status) {
		printf(PLAINT_INT, "status", file, line,
		       expected_status, actual_status);
		differences = true;
	}
	return differences;
}

static size_t extend_cpy(size_t size, size_t blen, char **beg,
		      size_t clen, const char *cont) {
	if (size < blen + clen + 1) {
		do {
			size *= 2;
		} while (size < blen + clen + 1);
		*beg = (char *)realloc(*beg, size + 1);
	}
	(*beg)[blen] = '/';
	memcpy(*beg + blen + 1, cont, clen);
	(*beg)[blen + clen + 1] = 0;
	return size;
}

// 0 if weird file, 1 if regular file, 2 if directory
static char gettype(size_t *size, size_t blen, char **curpath,
		   struct dirent *ent) {
#if defined(_DIRENT_HAVE_D_TYPE) && defined(DT_LNK) && defined(DT_REG) && defined(DT_DIR) && defined(DT_UNKNOWN)
	switch(ent->d_type) {
	case DT_REG: return 1;
	case DT_DIR: return 2;
	case DT_LNK: case DT_UNKNOWN: break;
	default: return 0;
	}
#endif
	*size = extend_cpy(*size, blen, curpath,
			   strlen(ent->d_name), ent->d_name);
	struct stat st;
	if (!stat(*curpath, &st)) {
		switch (st.st_mode & S_IFMT) {
		case S_IFDIR: return 2;
		case S_IFREG: return 1;
		}
	}
	return 0;
}

typedef struct _towalk {
	struct _towalk *next;
	size_t pos, len;
	char *name;
} towalk;

void walk_dir(void *args,
	      void (*proc_file)(void *args,
				const char *path,
				size_t plen),
	      const char *dirpath) {
	towalk *pile = NULL;
	size_t pend = strlen(dirpath);
	size_t psize = pend * 2;
	char *path = (char *)malloc(psize + 1);
	memcpy(path, dirpath, pend + 1);
	goto firststart;
	while (pile != NULL) {
		pend = pile->pos + pile->len + 1;
		psize = extend_cpy(psize, pile->pos, &path,
				   pile->len, pile->name);
		free(pile->name);
		{
			towalk *t = pile;
			pile = pile->next;
			free(t);
		}
	firststart:;
		DIR *d = opendir(path);
		struct dirent *ent;
		while ((ent = readdir(d)) != NULL) {
			if (*ent->d_name == '.') {
				continue;
			}
			char t = gettype(&psize, pend, &path, ent);
			if (!t) {
				continue;
			}
			size_t len = strlen(ent->d_name);
			if (t == 1) {
				psize = extend_cpy(psize, pend, &path,
						   len, ent->d_name);
				proc_file(args, path, pend + len + 1);
			} else {
				towalk *t = (towalk *)malloc(sizeof(towalk));
				t->next = pile;
				pile = t;
				pile->pos = pend;
				pile->len = len;
				pile->name = (char *)malloc(len);
				memcpy(pile->name, ent->d_name, len);
			}
		}
		closedir(d);
	}
	free(path);
}
