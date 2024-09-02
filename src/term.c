#include <sys/ioctl.h>
#include <sys/termios.h>
#include <signal.h>
#include <stdlib.h>

#include "term.h"

#define MAX_REGIONS 32

struct {
	uint32_t w, h;
	struct termios info;
	void (*resize_cb)(uint32_t, uint32_t);
} term;

static void resize(int i) {
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	term.w = ws.ws_col;
	term.h = ws.ws_row;
	if (term.resize_cb) term.resize_cb(term.w, term.h);
}

static void ondie(int i) {
	term_deinit();
	exit(1);
}

enum result term_init(void)
{
	printf(_tui_mode);
	printf(_tui_clear);
	signal(SIGWINCH, resize);
	signal(SIGTERM, ondie);
	signal(SIGINT, ondie);
	resize(1);

	{
		tcgetattr(1, &term.info);
		struct termios info = term.info;
		info.c_lflag &= ~ECHO & ~ICANON;
		tcsetattr(1, TCSANOW, &info);
	}

	return OK;
}

void term_deinit(void) {
	tcsetattr(1, TCSANOW, &term.info);
	printf(_tui_clear);
	printf(_norm_mode);
}

uint32_t term_width(void) {
	return term.w;
}

uint32_t term_height(void) {
	return term.h;
}

void term_set_resize_cb(void (*cb)(uint32_t w, uint32_t h)) {
	term.resize_cb = cb;
}

