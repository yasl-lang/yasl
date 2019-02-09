#include "yasl_float.h"

#include <stdio.h>
#include <stdlib.h>

char *float64_to_str(double d) {
	int64_t size = snprintf(NULL, 0, "%f", d);
	char *ptr = malloc(size + 1);
	sprintf(ptr, "%f", d);
	while (ptr[size - 1] == '0' && ptr[size - 2] != '.') {
		size--;
	}
	ptr[size] = '\0';
	ptr = realloc(ptr, size + 1);
	return ptr;
}
