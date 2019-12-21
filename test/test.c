#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

extern int cli_main(void);
extern int vm_main(void);
extern int file_main(void);
extern int mem_main(void);

const char *bar = "========================================";

const char *cmake[] = {"cmake", "--build", ".", NULL};

int neatsystem(const char *const *command) {
	pid_t child = fork();
	if (!child) {
		execvp(*command, (char *const *)command);
		exit(1);
	}
	int status = 0;
	waitpid(child, &status, 0);
	return WEXITSTATUS(status);
}

const char *yasltest[] = {"./yasltest", NULL};

int comp_main(void) {
	return neatsystem(yasltest);
}

static const struct {
	const char *desc;
	int (*func)(void);
} tests[] = {{"C Unit Tests", comp_main},
	     {"YASL Unit Tests", vm_main},
	     {"CLI", cli_main},
	     {"script", file_main},
	     {"memory", mem_main}};

int main(int argc, char **argv) {
	printf("%.20s\nBuilding...\n", bar);
	if (neatsystem(cmake)) {
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
