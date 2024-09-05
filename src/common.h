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

struct host_features;

enum result {
	ERR,
	OK,
};

enum color_plane {
	COLOR_PLANE_BG,
	COLOR_PLANE_FG,
};

#define COLORS \
	COLOR(BG, "bg", COLOR_PLANE_BG, 13, 17, 23) \
	COLOR(FG, "fg", COLOR_PLANE_FG, 236, 242, 248) \
	COLOR(STATBAR_BG, "statbar_bg", COLOR_PLANE_BG, 119, 189, 251) \
	COLOR(STATBAR_FG, "statbar_fg", COLOR_PLANE_FG, 13, 17, 23) \
	COLOR(CMDBAR_BG, "cmdbar_bg", COLOR_PLANE_BG, 13, 17, 23) \
	COLOR(CMDBAR_FG, "cmdbar_fg", COLOR_PLANE_FG, 236, 242, 248) \
	COLOR(CMDBAR_STAT, "cmdbar_stat", COLOR_PLANE_FG, 250, 136, 86) \
	COLOR(CMDBAR_ERR, "cmbar_err", COLOR_PLANE_FG, 250, 121, 112)

enum {
#define COLOR(color, name, plane, dr, dg, db) COLOR_##color,
	COLORS
#undef COLOR
	COLOR_MAX,
};

// 16 Default colors.
// First 8 are normal, last 8 are bright versions
#define BASE_COLORS \
	BASE_COLOR(BLACK, "black") \
	BASE_COLOR(RED, "red") \
	BASE_COLOR(GREEN, "green") \
	BASE_COLOR(YELLOW, "yellow") \
	BASE_COLOR(BLUE, "blue") \
	BASE_COLOR(MAGENTA, "purple") \
	BASE_COLOR(CYAN, "cyan") \
	BASE_COLOR(WHITE, "light_gray") \
	BASE_COLOR(BRIGHT_BLACK, "dark_gray") \
	BASE_COLOR(BRIGHT_RED, "bright_red") \
	BASE_COLOR(BRIGHT_GREEN, "bright_green") \
	BASE_COLOR(BRIGHT_YELLOW, "bright_yellow") \
	BASE_COLOR(BRIGHT_BLUE, "bright_blue") \
	BASE_COLOR(BRIGHT_MAGENTA, "pink") \
	BASE_COLOR(BRIGHT_CYAN, "bright_cyan") \
	BASE_COLOR(BRIGHT_WHITE, "white")
enum base_color {
#define BASE_COLOR(color, name) color,
	BASE_COLORS
#undef BASE_COLOR
	DEFAULT_FG,
	DEFAULT_BG,
};

struct color {
	// Cached escape sequence
	char style[16];
	char color[16];
};

struct color_init_args {
	const struct host_features *hf;

	// Color
	int bit_depth;
	union {
		struct { int r, g, b; };
		int i;
	};

	// Color plane
	enum color_plane plane;

	// Flags
	bool underline		: 1;
	bool italic		: 1;
	bool bold		: 1;
};

void color_init(struct color *col, struct color_init_args *args);
void color_gen_default(struct color *col, const struct host_features *hf,
			enum color_plane plane);

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
size_t strview_strncpy(char *buf, size_t len, const struct strview *s);

enum log_level {
	LOG_DBG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERR,
};

typedef void (log_cb_t)(enum log_level l, const char *msg, va_list args);

void logcb(log_cb_t *cb);
#define logdbg(msg) \
	_logdbg("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define loginfo(msg) \
	_loginfo("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logwarn(msg) \
	_logwarn("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logerr(msg) \
	_logerr("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logdbgf(msg, ...) \
	_logdbg("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define loginfof(msg, ...) \
	_loginfo("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define logwarnf(msg, ...) \
	_logwarn("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define logerrf(msg, ...) \
	_logerr("%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
void _logdbg(const char *msg, ...);
void _loginfo(const char *msg, ...);
void _logwarn(const char *msg, ...);
void _logerr(const char *msg, ...);

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

char *file_load_as_str(const char *path);
bool file_exists(const char *path);

struct host_features {
	int color_depth;
};

enum result host_features_find(struct host_features *hf);

char *get_realpath(const char *path);

#endif

