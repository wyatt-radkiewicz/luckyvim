#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <wordexp.h>

#include "common.h"

static int col24to8(int r, int g, int b) {
	struct {
		int r, g, b;
	} cols[256] = {
		[BLACK] = { 0, 0, 0 },
		[RED] = { 255, 0, 0 },
		[GREEN] = { 0, 255, 0 },
		[YELLOW] = { 255, 255, 0 },
		[BLUE] = { 0, 0, 255 },
		[MAGENTA] = { 255, 0, 255 },
		[CYAN] = { 0, 255, 255 },
		[WHITE] = { 192, 192, 192 },
		[BRIGHT_BLACK] = { 96, 96, 96 },
		[BRIGHT_RED] = { 255, 128, 128 },
		[BRIGHT_GREEN] = { 128, 255, 128 },
		[BRIGHT_YELLOW] = { 255, 255, 128 },
		[BRIGHT_BLUE] = { 128, 128, 255 },
		[BRIGHT_MAGENTA] = { 255, 128, 255 },
		[BRIGHT_CYAN] = { 128, 255, 255 },
		[BRIGHT_WHITE] = { 255, 255, 255 },
	};

	// Black to White ramp (232 - 256)
	for (int i = 0; i < 24; i++) {
		int idx = i + 232;
		cols[idx].r = i * 10;
		cols[idx].g = i * 10;
		cols[idx].b = i * 10;
	}

	// Build the rest of the table (cause I'm lazy)
	for (int r = 0; r < 6; r++) {
		for (int g = 0; g < 6; g++) {
			for (int b = 0; b < 6; b++) {
				const int idx = 16 + 36 * r + 6 * g + b;
				cols[idx].r = r * 42;
				cols[idx].g = g * 42;
				cols[idx].b = b * 42;
			}
		}
	}

	// Find closest color (with amidetly bad algorithm)
	int closest_dist = INT_MAX;
	int closest_idx = -1;
	for (int i = 0; i < 256;  i++) {
		int dist = abs(r - cols[i].r)
			+ abs(g - cols[i].g)
			+ abs(b - cols[i].b);

		if (dist < closest_dist) {
			closest_idx = i;
			closest_dist = dist;
		}
	}

	return closest_idx;
}

static int col8to4(int idx) {
	if (idx < 16) {
		// Default colors
		return idx;
	} else if (idx < 232) {
		idx -= 16;

		// Find what channels contribute to this the most
		const int r_mask = 0x1, g_mask = 0x2, b_mask = 0x4;

		// The value of the maximum color
		int max_color = 0;

		// The bits correspond to the max bits
		int max_rgb = 0;

		// 3bpc values
		const int b = idx % 6;
		idx /= 6;
		const int g = idx % 6;
		idx /= 6;
		const int r = idx % 6;

		// Check for maximum values
		if (r > g && r > b) {
			max_rgb |= r_mask;
			max_color = r;
		} else if (g > r && g > b) {
			max_rgb |= g_mask;
			max_color = g;
		} else if (b > r && b > g) {
			max_rgb |= b_mask;
			max_color = b;
		} else {
			max_rgb = r_mask | g_mask | b_mask;
			max_color = r;
		}

		// Check for secondary maximums
		if (r > max_color / 2) max_rgb |= r_mask;
		else if (g > max_color / 2) max_rgb |= g_mask;
		else if (b > max_color / 2) max_rgb |= b_mask;

		// Generate final color
		if (max_rgb == 0x7) {
			switch (max_color) {
			case 0: idx = BLACK; break;
			case 1: case 2: idx = BRIGHT_BLACK; break;
			case 3: case 4: idx = WHITE; break;
			case 5: idx = BRIGHT_WHITE; break;
			}
		} else {
			idx = max_rgb + max_color >= 3 ? 8 : 0;
		}
	} else {
		// Grayscale 24 color ramp
		idx -= 232;
		switch (idx / 6) {
		case 0: idx = BLACK; break;
		case 1: idx = BRIGHT_BLACK; break;
		case 2: idx = WHITE;
		case 3: case 4: idx = BRIGHT_WHITE; break;
		}
	}

	return idx;
}

void color_init_24(struct color *col, enum color_plane plane, int max_depth,
		int r, int g, int b) {
	if (max_depth < 24) {
		color_init_8(col, plane, max_depth, col24to8(r, g, b));
		return;
	}
	sprintf(col->esc_seq, CSI "%c8;2;%d;%d;%dm",
		plane == COLOR_PLANE_BG ? '4' : '3', r, g, b);
}
void color_init_8(struct color *col, enum color_plane plane,
		int max_depth, int idx) {
	if (max_depth < 8) {
		color_init_4(col, plane, col8to4(idx));
		return;
	}
	sprintf(col->esc_seq, CSI "%c8;5;%dm",
		plane == COLOR_PLANE_BG ? '4' : '3', idx);
}
void color_init_4(struct color *col, enum color_plane plane, int idx) {
	if (idx >= BRIGHT_BLACK) {
		sprintf(col->esc_seq, CSI "%s%dm",
			plane == COLOR_PLANE_BG ? "10" : "9", idx - BRIGHT_BLACK);
	} else {
		sprintf(col->esc_seq, CSI "%c%dm",
			plane == COLOR_PLANE_BG ? '4' : '3', idx);
	}
}
void color_gen_default(struct color *col, enum color_plane plane) {
	sprintf(col->esc_seq, CSI "%c9m",
		plane == COLOR_PLANE_BG ? '4' : '3');
}

log_cb_t *_logcb;

void logcb(log_cb_t *cb) {
	_logcb = cb;
}
#define LOGFN(name, level) \
	void name(const char *msg, ...) { \
		va_list args; \
		va_start(args, msg); \
		if (_logcb) _logcb(level, msg, args); \
		va_end(args); \
	}
LOGFN(_logdbg, LOG_DBG)
LOGFN(_loginfo, LOG_INFO)
LOGFN(_logwarn, LOG_WARN)
LOGFN(_logerr, LOG_ERR)

#define VEC_SIZE(elems, capacity) (sizeof(struct vec) * (capacity) * (elems))
void *vec_init(size_t elem_size, size_t capacity) {
	struct vec *vec = malloc(VEC_SIZE(capacity, elem_size));
	vec->len = 0;
	vec->capacity = capacity;
	vec->elem_size = elem_size;
	return vec->data;
}
void vec_deinit(void *data) {
	free(vec_from_data(data));
}
void *vec_push(void *data, size_t nelems, const void *elems) {
	struct vec *vec = vec_from_data(data);
	if (vec->len + nelems > vec->capacity) {
		vec->capacity *= 2;
		vec = realloc(vec, VEC_SIZE(vec->elem_size, vec->capacity));
	}

	memcpy(vec->data + vec->len * vec->elem_size, elems, nelems * vec->elem_size);
	vec->len += nelems;
	return vec->data;
}
void vec_pop(void *data, size_t nelems, void *elems) {
	struct vec *vec = vec_from_data(data);
	vec->len -= nelems;
	if (!elems) return;
	memcpy(elems, vec->data + vec->len * vec->elem_size, nelems * vec->elem_size);
}

char *file_load_as_str(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return NULL;

	FILE *fp = fopen(path, "r");
	if (!fp) return NULL;
	char *str = malloc(st.st_size + 1);
	fread(str, st.st_size, 1, fp);
	fclose(fp);
	str[st.st_size] = '\0';
	return str;
}

enum result host_features_find(struct host_features *hf) {
	const char *term = getenv("TERM");
	const char *color_term = getenv("COLORTERM");

	if (!term && !color_term) {
		logerr("host_features_find: $TERM environment variable not set!");
		return ERR;
	}

	{
		const char *colors = color_term ? color_term : term;
		if (strstr(colors, "truecolor") || strstr(colors, "24")) {
			hf->color_depth = 24;
		} else if (strstr(colors, "256")) {
			hf->color_depth = 8;
		} else {
			hf->color_depth = 4;
		}
	}
	return OK;
}

char *get_realpath(const char *path) {
	wordexp_t exp = {0};
	if (wordexp(path, &exp, WRDE_NOCMD) != 0) return NULL;
	size_t exp_len = 0;
	for (int i = 0; i < exp.we_wordc; i++) exp_len += strlen(exp.we_wordv[i]);

	char *rel = malloc(exp_len + 1);
	if (!rel) return NULL;
	rel[0] = '\0';

	for (int i = 0; i < exp.we_wordc; i++) strcat(rel, exp.we_wordv[i]);

	char *rpath = realpath(rel, NULL);
	free(rel);
	if (!rpath) return NULL;
	return rpath;
}

