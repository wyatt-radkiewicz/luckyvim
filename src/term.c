#include <sys/ioctl.h>
#include <sys/termios.h>
#include <signal.h>
#include <stdlib.h>

#include "term.h"

#define _esc "\x1b"
#define _tui_mode _esc "[?1049h"
#define _norm_mode _esc "[?1049l"
#define _tui_clear _esc "[2J"
#define MAX_REGIONS 32

struct {
	u32 w, h;
	struct termios info;
} term;

static void resize(int i) {
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	term.w = ws.ws_col;
	term.h = ws.ws_row;
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

u32 term_width(void) {
	return term.w;
}

u32 term_height(void) {
	return term.h;
}

