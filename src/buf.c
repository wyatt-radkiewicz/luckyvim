#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "buf.h"

struct buf *buf_init_from_file(const char *file_path, size_t gapsize) {
	struct stat st;
	if (stat(file_path, &st)) return NULL;

	FILE *file = fopen(file_path, "rb");
	if (!file) return NULL;

	struct buf *buf = malloc(sizeof(*buf));
	*buf = (struct buf){
		.vec_buf = vec_init(st.st_size + gapsize + 1, 1),
		.gap_pos = {
			.idx = 0,
			.line = 1,
			.col = 0,
		},
		.gap_len = gapsize,
	};
	memset(buf->vec_buf, 0, gapsize);
	if (fread(buf->vec_buf + gapsize, st.st_size, 1, file) != 1) return NULL;
	buf->vec_buf[gapsize + st.st_size] = '\0';

	return buf;
}

void buf_deinit(struct buf *buf) {
	vec_deinit(buf->vec_buf);
	free(buf);
}

#include "term.h"
void buf_draw_view_at_gap(struct buf *buf, bool wrap) {
	const int col_start = buf->gap_pos.col;
	uint8_t *p = buf->vec_buf + buf->gap_pos.idx + buf->gap_len;

	printf(_tui_clear _tui_home);

	for (int y = 0; y < term_height();) {
		while (y < term_height()) {
			// Get line contents
			uint8_t *const line_start = p;
			size_t line_width = 0;
			for (; line_width < term_width(); line_width++) {
				if (*p == '\n') break;
				p += utf8_codepoint_size(*p);
			}

			// Draw the line
			const uint8_t tmp = *p;
			*p = '\0';
			printf("%s", line_start);
			*p = tmp;
			printf("\r" _cursor_down, 1);

			if (*p == '\n' || *p == '\0' || !wrap) break;
			y++;
		}

		// Consume the rest of the line
		while (*p != '\n') p++;
		p++, y++;

		// Goto same column
		while (y < term_height()) {
			int i;
			for (i = col_start; i; i--, p += utf8_codepoint_size(*p)) {
				if (*p == '\n') {
					p++, y++;
					printf(_csi _cursor_down, 1);
					break;
				}
			}
			if (!i) break;
		}
	}

	fflush(stdout);
}

