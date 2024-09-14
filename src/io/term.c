#include <stdlib.h>

#include "term.h"
#include "util/log.h"
#include "util/util.h"

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
