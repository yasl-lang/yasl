#include "utiltest.h"
#include "yats.h"

#include "util/varint.h"

SETUP_YATS();

#define ASSERT_VINT_ROUNDTRIP(v) {\
	unsigned char buff[10];\
	int written = vint_encode(v, buff);\
	ASSERT_EQ(vint_next(buff) - buff, written);\
	ASSERT_EQ(vint_decode(buff), v);\
}

int utiltest(void) {
	ASSERT_VINT_ROUNDTRIP(120);
	ASSERT_VINT_ROUNDTRIP(3123);
	ASSERT_VINT_ROUNDTRIP(200);
	ASSERT_VINT_ROUNDTRIP(127);
	ASSERT_VINT_ROUNDTRIP(128);
	ASSERT_VINT_ROUNDTRIP(0);
	ASSERT_VINT_ROUNDTRIP(255);
	ASSERT_VINT_ROUNDTRIP(256);
	ASSERT_VINT_ROUNDTRIP(257);

	size_t v1 = 12131;
	size_t v2 = 2135413;
	size_t v3 = 213;

	unsigned char buff[200];
	int w1 = vint_encode(v1, buff);
	int w2 = vint_encode(v2, buff + w1);
	vint_encode(v3, buff + w1 + w2);

	ASSERT_EQ(vint_decode(buff), v1);
	ASSERT_EQ(vint_decode(vint_next(buff)), v2);
	ASSERT_EQ(vint_decode(vint_next(vint_next(buff))), v3);

	return __YASL_TESTS_FAILED__;
}
