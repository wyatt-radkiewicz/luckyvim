#ifndef _vec_h_
#define _vec_h_

#include <stdint.h>
#include <stddef.h>

struct vec {
	uint32_t len, capacity;
	uint8_t data[];
};

void *vec_init(uint32_t elem_size, uint32_t capacity);
void vec_deinit(void *data);
#define vec_push(vec, nelems, elems) _vec_push(vec, sizeof(*(vec)), nelems, elems)
#define vec_pop(vec, nelems, elems) _vec_pop(vec, sizeof(*(vec)), nelems, elems)
void *_vec_push(void *data, uint32_t elem_size, uint32_t nelems, const void *elems);
void _vec_pop(void *data, uint32_t elem_size, uint32_t nelems, void *elems);

#define vec_from_data(_data) ((struct vec *)((uintptr_t)(_data) - offsetof(struct vec, data)))
static inline uint32_t *vec_len(const void *const data) {
	return &vec_from_data(data)->len;
}
static inline uint32_t vec_capacity(const void *const data) {
	return vec_from_data(data)->capacity;
}

#endif

