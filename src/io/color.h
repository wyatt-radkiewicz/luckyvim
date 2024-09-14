#ifndef _color_h_
#define _color_h_

#include <stdbool.h>

#include "term.h"

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

#endif

