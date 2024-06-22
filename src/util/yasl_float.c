#include "yasl_float.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const char *float64_to_str(const yasl_float d) {
	if (isinf(d) || isnan(d)) {
		char *ptr;
		if (d > 0) {
			ptr = (char *)malloc(sizeof("inf") + 1);
			strcpy(ptr, "inf");
		} else if (d < 0) {
			ptr = (char *)malloc(sizeof("-inf") + 1);
			strcpy(ptr, "-inf");
		} else {
			ptr = (char *)malloc(sizeof("nan") + 1);
			strcpy(ptr, "nan");
		}
		return ptr;
	}

	int size = snprintf(NULL, 0, "%f", d);
	char *ptr = (char *)malloc((size_t)size+ 1);
	snprintf(ptr, (size_t)size+1, "%f", d);
	while (ptr[size - 1] == '0' && ptr[size - 2] != '.') {
		size--;
	}
	ptr[size] = '\0';
	ptr = (char *)realloc(ptr, (size_t)size + 1);
	return ptr;
}
