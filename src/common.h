#ifndef _common_h_
#define _common_h_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ESC "\x1b"
#define CSI ESC "["

#define ARRLEN(a) (sizeof(a) / sizeof((a)[0]))
#define MAX(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a > _b ? _a : _b; \
})
#define MIN(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a < _b ? _a : _b; \
})

enum result {
	ERR,
	OK,
};

enum color_plane {
	COLOR_PLANE_BG,
	COLOR_PLANE_FG,
};

#define COLORS \
	COLOR(BG, COLOR_PLANE_BG, 13, 17, 23) \
	COLOR(FG, COLOR_PLANE_FG, 236, 242, 248) \
	COLOR(STATUS_BAR_BG, COLOR_PLANE_BG, 119, 189, 251) \
	COLOR(STATUS_BAR_FG, COLOR_PLANE_FG, 13, 17, 23) \
	COLOR(COMMAND_BAR_BG, COLOR_PLANE_BG, 13, 17, 23) \
	COLOR(COMMAND_BAR_FG, COLOR_PLANE_FG, 236, 242, 248) \
	COLOR(COMMAND_BAR_STATUS, COLOR_PLANE_FG, 250, 136, 86) \
	COLOR(COMMAND_BAR_ERROR, COLOR_PLANE_FG, 250, 121, 112)

enum {
#define COLOR(color, plane, dr, dg, db) COLOR_##color,
	COLORS
#undef COLOR
	COLOR_MAX,
};

// 16 Default colors.
// First 8 are normal, last 8 are bright versions
#define BASE_COLORS \
	BASE_COLOR(BLACK) \
	BASE_COLOR(RED) \
	BASE_COLOR(GREEN) \
	BASE_COLOR(YELLOW) \
	BASE_COLOR(BLUE) \
	BASE_COLOR(MAGENTA) \
	BASE_COLOR(CYAN) \
	BASE_COLOR(WHITE) \
	BASE_COLOR(BRIGHT_BLACK) \
	BASE_COLOR(BRIGHT_RED) \
	BASE_COLOR(BRIGHT_GREEN) \
	BASE_COLOR(BRIGHT_YELLOW) \
	BASE_COLOR(BRIGHT_BLUE) \
	BASE_COLOR(BRIGHT_MAGENTA) \
	BASE_COLOR(BRIGHT_CYAN) \
	BASE_COLOR(BRIGHT_WHITE)
enum base_color {
#define BASE_COLOR(color) color,
	BASE_COLORS
#undef BASE_COLOR
	DEFAULT_FG,
	DEFAULT_BG,
};

struct color {
	uint8_t plane;

	// Cached escape sequence
	char esc_seq[24];
};

void color_init_24(struct color *col, enum color_plane plane, int max_depth,
		int r, int g, int b);
void color_init_8(struct color *col, enum color_plane plane,
		int max_depth, int idx);
void color_init_4(struct color *col, enum color_plane plane, int idx);
void color_gen_default(struct color *col, enum color_plane plane);

enum {
	STYLE_NONE,
	STYLE_BOLD	= 0x1,
	STYLE_ITALIC	= 0x2,
	STYLE_UNDERLINE	= 0x4,
	STYLE_HIGHLIGHT	= 0x8
};

struct style {
	uint8_t palette_idx;
	uint8_t flags;
};

struct strview {
	const char *str;
	size_t len;
};

#define STRVIEW(literal) ((struct strview){ .str = literal, .len = sizeof(literal) - 1 })

static inline bool strview_eq(const struct strview *a, const struct strview *b) {
	if (a->len != b->len) return false;
	return memcmp(a->str, b->str, a->len) == 0;
}

enum log_level {
	LOG_INFO,
	LOG_WARN,
	LOG_ERR,
};

typedef void (log_cb_t)(enum log_level l, const char *msg, va_list args);

void logcb(log_cb_t *cb);
void loginfo(const char *msg, ...);
void logwarn(const char *msg, ...);
void logerr(const char *msg, ...);

struct vec {
	size_t len;
	size_t capacity;
	size_t elem_size;
	uint8_t data[];
};

void *vec_init(size_t elem_size, size_t capacity);
void vec_deinit(void *data);
void *vec_push(void *data, size_t nelems, const void *elems);
void vec_pop(void *data, size_t nelems, void *elems);

static inline struct vec *vec_from_data(const void *data) {
	return (struct vec *)((uintptr_t)data - offsetof(struct vec, data));
}
static inline size_t *vec_len(const void *data) {
	return &vec_from_data(data)->len;
}
static inline size_t vec_capacity(const void *data) {
	return vec_from_data(data)->capacity;
}

static inline int utf8_codepoint_len(const uint8_t start) {
	const int clz = __builtin_clz(~start << 24);
	return clz + !clz;
}

#endif

