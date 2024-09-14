#include <stdlib.h>
#include <string.h>

#include "vec.h"

#define vecsize(elems, capacity) (sizeof(struct vec) * (capacity) * (elems))
void *vec_init(uint32_t elem_size, uint32_t capacity) {
	struct vec *vec = malloc(vecsize(capacity, elem_size));
	vec->len = 0;
	vec->capacity = capacity;
	return vec->data;
}
void vec_deinit(void *data) {
	free(vec_from_data(data));
}
void *_vec_push(void *data, uint32_t elem_size, uint32_t nelems, const void *elems) {
	struct vec *vec = vec_from_data(data);
	if (vec->len + nelems > vec->capacity) {
		vec->capacity *= 2;
		vec = realloc(vec, vecsize(elem_size, vec->capacity));
	}

	if (elems) memcpy(vec->data + vec->len * elem_size, elems, nelems * elem_size);
	vec->len += nelems;
	return vec->data;
}
void _vec_pop(void *data, uint32_t elem_size, uint32_t nelems, void *elems) {
	struct vec *vec = vec_from_data(data);
	vec->len -= nelems;
	if (elems) memcpy(elems, vec->data + vec->len * elem_size, nelems * elem_size);
}
