#include "test/test_util.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define PREF file

const char *prog_w_args[] = {"./yasl",
			     NULL,
			     NULL};

static void proc_file(void *args, const char *path, size_t plen) {
	int *a = (int *)args;
	if (strcmp(path + plen - 5, ".yasl")) {
		return;
	}
	prog_w_args[1] = path;
	a[0]++;
	char *ofname = (char *)malloc(plen + 5);
	strcpy(ofname, path);
	strcpy(ofname + plen, ".out");
	int ofd = open(ofname, O_RDONLY);
	free(ofname);
	struct stat st;
	fstat(ofd, &st);
	size_t sz = (size_t)st.st_size;
	char *exp_out = (char *)malloc(sz);
	read(ofd, exp_out, sz);
	close(ofd);
	a[1] += assert_output(*prog_w_args, prog_w_args + 1, sz, exp_out,
			      0, NULL, 0, path, 0);
	free(exp_out);
}

int MAIN(void) {
	int a[] = {0, 0};
	walk_dir(a, proc_file, "test/inputs");
	REPORT(a[0], a[1]);
	return !!a[1];
}

