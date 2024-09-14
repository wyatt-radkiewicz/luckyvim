#ifndef _util_h_
#define _util_h_

#include <stdint.h>

#define arrlen(a) (sizeof(a) / sizeof((a)[0]))
#define max(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a > _b ? _a : _b; \
})
#define min(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a < _b ? _a : _b; \
})

enum result {
	ERR,
	OK,
};

static inline int utf8_codepoint_len(const uint8_t start) {
	const int clz = __builtin_clz(~start << 24);
	return clz + !clz;
}

#endif

