#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

extern int cli_main(void);

const char *bar = "========================================";

const char *cmake[] = {"cmake", "--build", ".", NULL};

const char *perlcomm[] = {"perl", NULL, NULL};

int neatsystem(const char *const *command, const char *dir) {
	pid_t child = fork();
	if (!child) {
		if (dir != NULL) {
			chdir(dir);
		}
		execvp(*command, (char *const *)command);
		exit(1);
	}
	int status = 0;
	waitpid(child, &status, 0);
	return WEXITSTATUS(status);
}


int perl(const char *file) {
	perlcomm[1] = file;
	return neatsystem(perlcomm, "test");
}

#define PREPX(x) XPREPX(x)
#define XPREPX(a) x ## a

#define DEFMAIN(x) int x ## _main(void) { return perl(#x "test.pl"); } \
  int PREPX(__LINE__)

const char *yasltest[] = {"./yasltest", NULL};

int comp_main(void) {
	return neatsystem(yasltest, NULL);
}
DEFMAIN(file);
DEFMAIN(mem);
DEFMAIN(vm);

static const struct {
	const char *desc;
	int (*func)(void);
} tests[] = {{"compiler", comp_main},
	     {"CLI", cli_main},
	     {"VM", vm_main},
	     {"script", file_main},
	     {"memory", mem_main}};

int main(int argc, char **argv) {
	printf("%.20s\nBuilding...\n", bar);
	if (neatsystem(cmake, NULL)) {
		printf("Build failed\n");
		return(1);
	}
	printf("Build successful\n\n");
	size_t skipmem = argc > 1 && !strcmp(argv[1], "-m");
	bool result = false;
	for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]) - skipmem; i++) {
		printf("%s\nRunning %s tests...\n", bar, tests[i].desc);
		int output = tests[i].func();
		printf("%c%s tests exited with code %i\n\n",
		       tests[i].desc[0] & 0x5F, tests[i].desc + 1, output);
		result = result || output;
	}
	printf("%s\n", bar);
	printf("Tests exited with code %i\n", result);
	return result;
}
