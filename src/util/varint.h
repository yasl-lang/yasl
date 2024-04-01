#ifndef YASL_VARINT_H_
#define YASL_VARINT_H_

#include <stdlib.h>

// Encodes and decodes integers of variable width.

/**
 * Encodes int
 * @param val the value to encode
 * @param buff the buffer to store the result in
 * @return the number of bytes used
 */
int vint_encode(const size_t val, unsigned char *const buff);

/**
 * Decodes int. Assumes that the buffer contains a valid encoded int.
 * @param buff the buffer to read from.
 * @return the decoded int.
 */
size_t vint_decode(const unsigned char *const buff);

const unsigned char *vint_next(const unsigned char *buff);

#endif
