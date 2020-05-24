#include "varint.h"

int vint_encode(const size_t val, unsigned char *const buff) {
	if (val <= 0x7F) {
		*buff = (unsigned char)val;
		return 1;
	}
	unsigned char tmp = (val & 0x7F) | (1 << 7);
	*buff = tmp;
	return 1 + vint_encode(val >> 7, buff + 1);
}

size_t vint_decode(const unsigned char *const buff) {
	if (!(*buff & (1 << 7))) {
		return *buff;
	}
	unsigned char tmp = *buff & 0x7F;
	return tmp | (vint_decode(buff + 1) << 7);
}

const unsigned char *vint_next(const unsigned char *buff) {
	while ((*buff & (1 << 7))) buff++; // fast forward to next
	return buff + 1;
}