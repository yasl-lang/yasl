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
			*(p++) = 0xE2;
			*(p++) = 0x90;
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

static int qx(const char *prog, const char *const *args,
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
			char *ec = convert(expected_error, eelen);
			char *ac = convert(actual_error, aelen);
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
