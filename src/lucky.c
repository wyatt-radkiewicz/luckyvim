#include <stdio.h>

#include "common.h"
#include "conf.h"

void my_logcb(enum log_level l, const char *msg, va_list args) {
	vprintf(msg, args);
	printf("\n");
}

int main(int argc, char **argv) {
	struct conf conf;
	struct host_features hf;

	logcb(my_logcb);
	if (!host_features_find(&hf)) return -1;
	if (!conf_load_rc(&conf, &hf)) return -1;
	printf("%s\n", conf.options.splash_screen);
	conf_deinit(&conf);
	return 0;
}

