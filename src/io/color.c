#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"

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

void color_init(struct color *col, struct color_init_args *args) {
	memset(col, 0, sizeof(*col));

	if (args->underline) strcat(col->style, "4;");
	else strcat(col->style, "24;");
	if (args->bold) strcat(col->style, "1;");
	else strcat(col->style, "22;");
	if (args->italic) strcat(col->style, "3;");
	else strcat(col->style, "23;");

	switch (args->bit_depth) {
	case 24:
		if (args->hf->color_depth >= 24) {
			sprintf(col->color, "%c8;2;%d;%d;%dm",
				args->plane == COLOR_PLANE_BG ? '4' : '3',
				args->r, args->g, args->b);
			break;
		}

		args->i = col24to8(args->r, args->g, args->b);
		args->bit_depth = 8;
	case 8:
		if (args->hf->color_depth >= 8) {
			sprintf(col->color, "%c8;5;%dm",
				args->plane == COLOR_PLANE_BG ? '4' : '3',
				args->i);
			break;
		}

		args->i = col8to4(args->i);
		args->bit_depth = 4;
	case 4:
		if (args->i >= BRIGHT_BLACK) {
			sprintf(col->color, CSI "%s%dm",
				args->plane == COLOR_PLANE_BG ? "10" : "9",
				args->i - BRIGHT_BLACK);
		} else {
			sprintf(col->color, CSI "%c%dm",
				args->plane == COLOR_PLANE_BG ? '4' : '3',
				args->i);
		}
		break;
	}
}
void color_gen_default(struct color *col, const struct host_features *hf,
			enum color_plane plane) {
	strcpy(col->style, "0;");
	sprintf(col->color, "%c9m",
		plane == COLOR_PLANE_BG ? '4' : '3');
}
