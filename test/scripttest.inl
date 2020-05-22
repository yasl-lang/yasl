#include "test/test_util.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "yasl_error.h"

#if MEM
const char *prog_w_args[] = {"valgrind",
			     "--error-exitcode=1",
			     "--leak-check=full",
			     "./yasl",
			     NULL,
			     NULL};
#define PREF mem
#else
#define PREF file
extern const char *prog_w_args[];
#endif

#define REPL_ARG prog_w_args[4]
#define PROG prog_w_args[!MEM * 3]
#define ARGS (prog_w_args + !MEM * 3 + 1)
#define XSTR(x) #x
#define STR(x) XSTR(x)

static void proc_file(void *args, const char *path, size_t plen) {
	int *a = (int *)args;
	if (strcmp(path + plen - 5, ".yasl")) {
		return;
	}
	REPL_ARG = path;
	a[0]++;
#if MEM
	if (qx(PROG, ARGS, NULL, NULL, NULL, NULL)) {
		a[1]++;
		printf("\x1B[31mMemory Error in %s.\x1B[0m\n", path);
	}
#else
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
	a[1] += assert_output(PROG, ARGS, sz, exp_out, 0, NULL, 0, path, 0);
	free(exp_out);
#endif
}

#if MEM
#define GEN_PROC_FILL_ERR(name, exit_code)\
static void proc_file_err_##name(void *args, const char *path, size_t plen) {\
	(void) args;\
	(void) path;\
	(void) plen;\
}
#else
#define GEN_PROC_FILL_ERR(name, exit_code)\
static void proc_file_err_##name(void *args, const char *path, size_t plen) {\
        int *a = (int *)args;\
        if (strcmp(path + plen - 5, ".yasl")) {\
                return;\
        }\
        REPL_ARG = path;\
        a[0]++;\
        char *ofname = (char *)malloc(plen + 5);\
        strcpy(ofname, path);\
        strcpy(ofname + plen, ".err");\
        int ofd = open(ofname, O_RDONLY);\
        free(ofname);\
        struct stat st;\
        fstat(ofd, &st);\
        size_t sz = (size_t)st.st_size;\
        char *exp_out = (char *)malloc(sz);\
        read(ofd, exp_out, sz);\
        close(ofd);\
        a[1] += assert_output(PROG, ARGS, 0, NULL, sz, exp_out, exit_code, path, 0);\
        free(exp_out);\
}
#endif

GEN_PROC_FILL_ERR(assert, YASL_ASSERT_ERROR)

int MAIN(void) {
	int a[] = {0, 0};
	walk_dir(a, proc_file, "test/inputs");
	REPORT(a[0], a[1]);

	int assert_errors[] = {0, 0};
	walk_dir(assert_errors, proc_file_err_assert, "test/errors/assert");
	REPORT(assert_errors[0], assert_errors[1]);

	return !!a[1] || !!assert_errors[1];
}
