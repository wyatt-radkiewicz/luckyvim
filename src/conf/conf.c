#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "conf.h"
#include "io/fs.h"
#include "util/log.h"

static char *alloc_default_string(const char *src) {
	const size_t len = strlen(src);
	char *str = malloc(len + 1);
	memcpy(str, src, len + 1);
	return str;
}

void conf_default(struct conf *conf, const struct host_features *hf) {
	*conf = (struct conf){
#define CONF_OPT(name, type, default, min, max) .name = default,
		CONF_OPTS
#undef CONF_OPT
#define CONF_FLAG(name, default) .name = default,
		CONF_FLAGS
#undef CONF_FLAG
	};

	// Set up colors
#define COLOR(color, name, _plane, dr, dg, db) \
	color_init(conf->colors + COLOR_##color, &(struct color_init_args){ \
		.hf = hf, \
		.plane = _plane, \
		.bit_depth = 24, \
		.r = dr, .g = dg, .b = db, \
	});
	COLORS
#undef COLOR
}

static inline void opt_free_str(void *x) {
	char **str = x;
	if (*str) free(*str);
	*str = NULL;
}

static inline void opt_free_none(void *x) {}

enum result conf_load(struct conf *conf, const struct host_features *hf,
			const char *str) {
	conf_default(conf, hf);
	return ERR;
}

void conf_deinit(struct conf *conf) {
#define CONF_OPT(name, type, _default, min, max) \
	_Generic(conf->name, \
		char * : opt_free_str, \
		default : opt_free_none)((void *)&conf->name);
		CONF_OPTS
#undef CONF_OPT
}

enum result conf_load_rc(struct conf *conf, const struct host_features *hf) {
	errno = 0;

	const char *unresolved_path = getenv(CONF_PATH_ENV);
	if (!unresolved_path) unresolved_path = CONF_PATH;
	char *path = get_realpath(unresolved_path);

	if (path && access(path, F_OK | R_OK) == 0) {
		enum result ret = OK;

		char *str = file_load_as_str(path);
		if (!str) {
			logerr("file is here, but can not open it");
			free(path);
			return ERR;
		}
		if (!conf_load(conf, hf, str)) ret = ERR;

		free(str);
		free(path);
		return ret;
	}

	switch (errno) {
	case ENOENT: logdbg("no config file found"); break;
	case EACCES: logwarn("can't open config file, access denied"); break;
	default: logwarn("error when opening config file"); break;
	}

	logdbg("loading default internal config");
	conf_default(conf, hf);
	free(path);
	return OK;
}

