#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "term.h"
#include "buf.h"

#define vec_block_size(capacity, elem_size) (sizeof(struct vec) + capacity * elem_size)

static inline struct vec *vec_from_data(const void *data) {
	return (struct vec *)((uintptr_t)data - offsetof(struct vec, data));
}
void *vec_init(size_t capacity, size_t elem_size) {
	struct vec *vec = malloc(vec_block_size(capacity, elem_size));
	if (!vec) return NULL;

	*vec = (struct vec) {
		.len = 0,
		.elem_size = elem_size,
		.capacity = capacity,
	};
	return vec->data;
}
void vec_deinit(void *vec) {
	free(vec_from_data(vec));
}
void *vec_push(void *vec, size_t nelems, const void *elems) {
	struct vec *self = vec_from_data(vec);
	if (self->len + nelems > self->capacity) {
		self->capacity = self->capacity * 3 / 2;
		self = realloc(self, vec_block_size(self->capacity, self->elem_size));
		if (!self) return NULL;
	}

	memcpy(self->data + self->elem_size * self->len, elems, nelems * self->elem_size);
	self->len += nelems;
	return self->data;
}
void vec_pop(void *vec, size_t nelems, void *elems) {
	struct vec *self = vec_from_data(vec);
	self->len -= nelems * self->elem_size;
	if (elems) memcpy(elems, self->data + self->elem_size * self->len,
			nelems * self->elem_size);
}
size_t vec_len(const void *vec) {
	return vec_from_data(vec)->len;
}
size_t vec_capacity(const void *vec) {
	return vec_from_data(vec)->capacity;
}

static struct buf *buf;
static void resize(uint32_t w, uint32_t h) {
	buf_draw_view_at_gap(buf, true);
}

int main(int argc, char **argv) {
	if (!term_init()) return -1;

	printf("put in a file name:\n");
	char input_buf[64];
	for (int i = 0; i + 1 < sizeof(input_buf); i++) {
		char c;
		read(1, &c, 1);
		if (c == '\n') {
			break;
		} else {
			input_buf[i] = c;
			input_buf[i + 1] = '\0';
			printf("%c", c);
			fflush(stdout);
		}
	}
	//strcpy(input_buf, "src/buf.c");

	buf = buf_init_from_file(input_buf, 1024);
	if (!buf) {
		term_deinit();
		printf("error when reading file %s!\n", input_buf);
		return 1;
	}

	buf_draw_view_at_gap(buf, true);
	term_set_resize_cb(resize);
	for (char c = '\0'; c != 'q'; read(1, &c, 1));

	buf_deinit(buf);
	term_deinit();	
	return 0;
}

