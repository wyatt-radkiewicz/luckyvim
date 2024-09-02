#ifndef _lucky_h_
#define _lucky_h_

#include <stddef.h>
#include <stdint.h>

enum result {
	ERR,
	OK,
};

// Vector metadata
struct vec {
	size_t len;
	size_t capacity;
	size_t elem_size;
	uint8_t data[];
};

void *vec_init(size_t capacity, size_t elem_size);
void vec_deinit(void *vec);
void *vec_push(void *vec, size_t nelems, const void *elems);
void vec_pop(void *vec, size_t nelems, void *elems);
size_t vec_len(const void *vec);
size_t vec_capacity(const void *vec);

static inline int utf8_codepoint_size(const uint8_t x) {
	const int clz = __builtin_clz(~x << 24);
	return clz + !clz;
}

#endif

