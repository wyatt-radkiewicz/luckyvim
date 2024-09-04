#include <stdio.h>

#include "common.h"
#include "conf.h"

void my_logcb(enum log_level l, const char *msg, va_list args) {
	vprintf(msg, args);
	printf("\n");
}

int main(int argc, char **argv) {
	struct conf conf;

	logcb(my_logcb);
	if (!conf_load(&conf, 24,
		"[options]\n"
		"tab_width = 4\n"
		"\n"
		"[flags]\n"
		"relative_numbers = false\n"
		"\n"
		"[colors]\n"
		"status_bar_fg = 0\n"
		"status_bar_bg = \"black\"")) {
		printf("configure loading error\n");
		return -1;
	}

	printf("%s\n", conf.options.splash_screen);

	conf_deinit(&conf);
	return 0;
}

