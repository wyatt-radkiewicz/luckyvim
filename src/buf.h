#ifndef _buf_h_
#define _buf_h_

#include <stddef.h>
#include <stdint.h>

#include "lucky.h"

struct point {
	// What index in the gap buffer is point is
	size_t idx;

	// What line the gap is on
	int line;

	// What column the point is 'inbetween'
	// A value of 0 would mean before the first character in the line
	// A value of 1 would mean after the first character but before the 2nd
	int col;
};

struct buf {
	uint8_t *vec_buf;

	struct point gap_pos;
	size_t gap_len;
};

struct buf *buf_init_from_file(const char *file_path, size_t gapsize);
void buf_deinit(struct buf *buf);
void buf_draw_view_at_gap(struct buf *buf, bool wrap);

#endif

