#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "conf.h"

static char *alloc_default_string(const char *src) {
	const size_t len = strlen(src);
	char *str = malloc(len + 1);
	memcpy(str, src, len + 1);
	return str;
}

void conf_default(struct conf *conf, const struct host_features *hf) {
	*conf = (struct conf){
		.options = {
#define CONF_OPT(name, type, default, min, max) .name = default,
		CONF_OPTS
#undef CONF_OPT
		},
		.flags = {
#define CONF_FLAG(name, default) .name = default,
		CONF_FLAGS
#undef CONF_FLAG
		},
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

struct parser {
	const char *src;
	int line;

	// Parent struct
	struct strview keys[8];
	int nkeys;

	// Max size of escaped string
	char *esc;

	// Current color being parsed
	struct color *color;
	struct color_init_args color_args;
};

enum result parse_whitespace(struct parser *p, bool include_newlines) {
	while (*p->src == ' ' || *p->src == '\t' || *p->src == '\n'
			|| *p->src == '\r' || *p->src == '#') {
		if (*p->src == '\n') {
			if (!include_newlines) return OK;
			p->line++;
		}
		if (*p->src == '#') {
			if (!include_newlines) {
				logerrf("comment at line %d when expecting whitespace",
					p->line);
				return ERR;
			}

			while (*p->src != '\n') ++p->src;
			++p->line;
		}
		++p->src;
	}

	return OK;
}

// Add char to esc
enum result parse_string_char(struct parser *p, struct strview *str, bool do_esc) {
	if (!do_esc || *p->src != '\\') {
		const size_t len = utf8_codepoint_len(*p->src);
		if (!(p->esc = vec_push(p->esc, len, p->src))) {
			logerrf("can not allocate new escaped string on line %d",
				p->line);
			return ERR;
		}
		p->src += len;
		str->len += len;
		return OK;
	}

	p->src++;
	unsigned long num;
	switch (*p->src) {
	case 'u': case 'U': {
		const int ndigits = *p->src++ == 'u' ? 4 : 8;
		const char *src = p->src;
		num = strtoul(p->src, (char **)&p->src, 16);
		if (!p->src) {
			logerrf("invalid integer format for unicode char on line %d",
				p->line);
			return ERR;
		}
		if (ndigits != p->src - src) {
			logerrf("too many or too few digits in unicode char on line %d",
				p->line);
			return ERR;
		}
		break;
	}
	case 'n':
		if (!(p->esc = vec_push(p->esc, 1, &(char){'\n'}))) return ERR;
		str->len++;
		p->src++;
		return OK;
	default:
		if (!(p->esc = vec_push(p->esc, 1, p->src++))) return ERR;
		str->len++;
		return OK;
	}

	// Encode num as character literal
	if (num < 0x80) {
		if (!(p->esc = vec_push(p->esc, 1, &(char){ num }))) return ERR;
		str->len += 1;
	} else if (num < 0x800) {
		if (!(p->esc = vec_push(p->esc, 2, &(char[]){
			num >> 6	| 0xc0,
			num & 0x3f	| 0x80,
		}))) return ERR;
		str->len += 2;
	} else if (num < 0x10000) {
		if (!(p->esc = vec_push(p->esc, 3, &(char[]){
			num >> 12		| 0xe0,
			(num >> 6 & 0x3f)	| 0x80,
			num & 0x3f		| 0x80,
		}))) return ERR;
		str->len += 3;
	} else {
		if (!(p->esc = vec_push(p->esc, 4, &(char[]){
			num >> 18		| 0xf0,
			(num >> 12 & 0x3f)	| 0x80,
			(num >> 6 & 0x3f)	| 0x80,
			num & 0x3f		| 0x80,
		}))) return ERR;
		str->len += 4;
	}

	return OK;
}

struct strview *parse_string(struct parser *p, struct strview *str) {
	*vec_len(p->esc) = 0;

	if (*p->src != '\'' && *p->src != '"') {
		logerrf("expected string on line %d", p->line);
		return NULL;
	}

	const bool is_literal = *p->src == '\'';
	const bool is_multiline = p->src[0] == p->src[1] && p->src[0] == p->src[2];

	p->src += is_multiline ? 3 : 1;
	*str = (struct strview){ .str = p->esc, .len = 0 };

	while (*p->src != (is_literal ? '\'' : '"')) {
		if (*p->src == '\n') {
			if (is_multiline) {
				p->line++;
				if (str->len == 0) p->src++;
			} else {
				logerrf("newline in non-multiline string on line %d",
					p->line);
				return NULL;
			}
		}
		if (!parse_string_char(p, str, !is_literal)) return NULL;
	}
	p->src += is_multiline ? 3 : 1;

	return str;
}

struct strview *parse_word(struct parser *p, struct strview *str, bool allow_quoted) {
	if (allow_quoted && (*p->src == '\'' || *p->src == '"')) {
		return parse_string(p, str);
	}
	if (!isalpha(*p->src) && *p->src != '_') {
		logerrf("expected word on line %d", p->line);
		return NULL;
	}

	*str = (struct strview){
		.str = p->src,
		.len = 0,
	};
	while (isalnum(*p->src) || *p->src == '_') {
		str->len++;
		p->src++;
	}

	return str;
}

static int set_int(struct conf *conf, struct parser *p,
			int min, int max, enum result *result) {
	if (!isdigit(*p->src)) {
		logerrf("expected integer on line %d", p->line);
		*result = ERR;
		return 0;
	}
	int val = strtol(p->src, (char **)&p->src, 0);
	if (val < min || val > max) {
		char buf[256];
		strview_strncpy(buf, ARRLEN(buf), p->keys + p->nkeys - 1);
		logerrf("value %s out of range [%d, %d]", buf, min, max);
		*result = ERR;
		return 0;
	}

	*result = OK;
	return val;
}

static char *set_str(struct conf *conf, struct parser *p, enum result *res) {
	struct strview view;
	*res = parse_string(p, &view) ? OK : ERR;
	char *str = malloc(view.len + 1);
	memcpy(str, view.str, view.len);
	str[view.len] = '\0';
	return str;
}

static inline void opt_free_str(void *x) {
	char **str = x;
	if (*str) free(*str);
	*str = NULL;
}

static inline void opt_free_none(void *x) {}

static enum result set_opt(struct conf *conf, struct parser *p) {
	if (p->nkeys != 2 || !strview_eq(p->keys, &STRVIEW("options"))) {
		logerrf("line %d: options must be in 'options' table", p->line);
		return ERR;
	}

#define CONF_OPT(name, type, _default, min, max) \
	if (strview_eq(p->keys + 1, &STRVIEW(#name))) { \
		enum result res; \
		_Generic(conf->options.name, \
			char * : opt_free_str, \
			default : opt_free_none)((void *)&conf->options.name); \
		conf->options.name = _Generic(conf->options.name, \
			uint8_t : set_int(conf, p, min, max, &res), \
			char * : set_str(conf, p, &res) \
		); \
		return res; \
	}
		CONF_OPTS
#undef CONF_OPT

	logerrf("unknown option found on line %d", p->line);
	return ERR;
}

static enum result set_bool(struct conf *conf, struct parser *p, bool *out) {
	struct strview ident;
	if (!parse_word(p, &ident, false)) return ERR;
	if (strview_eq(&ident, &STRVIEW("true"))) {
		*out = true;
	} else if (strview_eq(&ident, &STRVIEW("false"))) {
		*out = false;
	} else {
		logerrf("expected 'true' or 'false' on line %d", p->line);
		return ERR;
	}

	return OK;
}

static int set_base_col(struct conf *conf, struct parser *p, enum result *res) {
	if (*p->src != '"') return set_int(conf, p, 0, 255, res);
	
	struct strview str;
	if (!parse_string(p, &str)) return ERR;

#define BASE_COLOR(color, name) \
	if (strview_eq(&str, &STRVIEW(name))) { \
		*res = OK; \
		return color; \
	}
	BASE_COLORS
#undef BASE_COLOR

	logerrf("expected int or color string in color on line %d", p->line);
	return ERR;
}

static enum result set_color(struct conf *conf, struct parser *p) {
	if (isdigit(*p->src) || *p->src == '"') {
		enum result res;
		p->color_args.bit_depth = 8;
		p->color_args.i = set_base_col(conf, p, &res);
		return res;
	}

	if (*p->src++ != '[') {
		logerrf("expected array on line %d", p->line);
		return ERR;
	}
	if (!parse_whitespace(p, true)) return ERR;

	int nums[5];
	int nums_len = 0;
	while (*p->src != ']') {
		if (nums_len == ARRLEN(nums)) {
			logerrf("on line %d: colors can only be up to 5 long", p->line);
			return ERR;
		}

		enum result res;
		if (nums_len == 4) nums[nums_len++] = set_base_col(conf, p, &res);
		else nums[nums_len++] = set_int(conf, p, 0, 255, &res);
		if (!res) return ERR;

		if (!parse_whitespace(p, true)) return ERR;
		if (*p->src != ',') {
			if (*p->src == ']') break;
			logerrf("missing ] on line %d", p->line);
			return ERR;
		}
		p->src++;
		if (!parse_whitespace(p, true)) return ERR;
	}
	p->src++;

	switch (nums_len) {
	case 1:
		p->color_args.bit_depth = 8;
		p->color_args.i = nums[0];
		break;
	case 2:
		switch (p->color_args.hf->color_depth) {
		case 4:
			if (nums[1] > 15) goto erange;
			p->color_args.bit_depth = 4;
			p->color_args.i = nums[1];
		default:
			p->color_args.bit_depth = 8;
			p->color_args.i = nums[0];
			break;
		}
	case 3:
		p->color_args.bit_depth = 24;
		p->color_args.r = nums[0];
		p->color_args.g = nums[1];
		p->color_args.b = nums[2];
		break;
	case 4:
	case 5:
		switch (p->color_args.hf->color_depth) {
		case 4:
			if (nums_len == 5) {
				if (nums[4] > 15) goto erange;
				p->color_args.bit_depth = 4;
				p->color_args.i = nums[4];
				break;
			}
		case 8:
			p->color_args.bit_depth = 8;
			p->color_args.i = nums[3];
			break;
		default:
			p->color_args.bit_depth = 24;
			p->color_args.r = nums[0];
			p->color_args.g = nums[1];
			p->color_args.b = nums[2];
			break;
		}
	}

	return OK;

erange:
	logerrf("on line %d: expected range for basic color is 0-15", p->line);
	return ERR;
}

static enum result set_table(struct conf *conf, struct parser *p,
				const struct host_features *hf);

static enum result is_color(struct conf *conf, struct parser *p,
				const struct host_features *hf) {
	// Check if we're setting a color, then do colors instead
	if (p->nkeys != 2 || !strview_eq(p->keys, &STRVIEW("colors"))) {
		return OK;
	}

	if (p->color) {
		color_init(p->color, &p->color_args);
		p->color = NULL;
	}
#define COLOR(col, name, _plane, dr, dg, db) \
	if (strview_eq(p->keys + p->nkeys - 1, \
		&(struct strview){ .str = name, \
			.len = sizeof(name) - 1})) { \
		p->color = conf->colors + COLOR_##col; \
		p->color_args = (struct color_init_args){ \
			.hf = hf, \
			.plane = _plane, \
		}; \
		return OK; \
	}
	COLORS
#undef COLOR

	char buf[256];
	strview_strncpy(buf, ARRLEN(buf), p->keys + p->nkeys - 1);
	logerrf("no color by name %s on line %d", buf, p->line);
	return ERR;
}

static enum result parse_key(struct conf *conf, struct parser *p,
				const struct host_features *hf) {
	if (!parse_whitespace(p, false)) return ERR;
	while (true) {
		if (!parse_word(p, p->keys + p->nkeys++, true)) return ERR;
		if (!is_color(conf, p, hf)) return ERR;
		if (!parse_whitespace(p, false)) return ERR;
		if (p->nkeys > ARRLEN(p->keys)) {
			logerrf("hit recursion limit on line %d", p->line);
			return ERR;
		}
		if (*p->src != '.') break;
		p->src++;
	}

	if (!parse_whitespace(p, false)) return ERR;
	return OK;
}

static enum result parse_kv(struct conf *conf, struct parser *p,
				const struct host_features *hf, bool expect_newln) {
	if (!parse_whitespace(p, true)) return ERR;
	if (*p->src == '\0') return OK;

	// parse [section] headers
	if (expect_newln && *p->src == '[') {
		p->src++;
		p->nkeys = 0;
		if (!parse_key(conf, p, hf)) return ERR;
		if (*p->src++ != ']') {
			logerrf("expected ']' on line %d", p->line);
			return ERR;
		}
		if (!parse_whitespace(p, false)) return ERR;
		if (*p->src == '\0') {
			return OK;
		} else if (*p->src++ != '\n') {
			logerrf("expected newline after table header on line %d",
				p->line);
			return ERR;
		}
		p->line++;
	}

	const int reset_keys = p->nkeys;

	if (!parse_key(conf, p, hf)) return ERR;

	// Make sure equals sign is there
	if (*p->src++ != '=') {
		logerrf("expected '=' on line %d", p->line);
		return ERR;
	}

	if (!parse_whitespace(p, false)) return ERR;

	// This is a color
	if (p->color) {
		// Just setting color
		if (p->nkeys == 2) {
			if (!set_color(conf, p)) return ERR;
			goto check_newln;
		} else if (p->nkeys != 3) {
			logerrf("invalid color field on line %d", p->line);
			return ERR;
		}

		// Setting either color or something else
		if (strview_eq(p->keys + 2, &STRVIEW("color"))) {
			if (!set_color(conf, p)) return ERR;
			goto check_newln;
		} else if (strview_eq(p->keys + 2, &STRVIEW("underline"))) {
			bool val;
			if (!set_bool(conf, p, &val)) return ERR;
			p->color_args.underline = val;
		} else if (strview_eq(p->keys + 2, &STRVIEW("italic"))) {
			bool val;
			if (!set_bool(conf, p, &val)) return ERR;
			p->color_args.italic = val;
		} else if (strview_eq(p->keys + 2, &STRVIEW("bold"))) {
			bool val;
			if (!set_bool(conf, p, &val)) return ERR;
			p->color_args.bold = val;
		} else {
			logerrf("invalid color field on line %d", p->line);
			return ERR;
		}

		if (p->color_args.plane == COLOR_PLANE_BG) {
			logwarnf("line %d: can not have style for bg color",
				p->line);
			p->color_args.underline = false;
			p->color_args.italic = false;
			p->color_args.bold = false;
		}

		goto check_newln;
	}

	// Now this can be many different things...
	switch (*p->src) {
	case 't': case 'f': {
		bool val;
		
		if (p->nkeys != 2 || !strview_eq(p->keys, &STRVIEW("flags"))) {
			logerrf("expected flag to be in 'flags' table on line %d",
				p->line);
			return ERR;
		}
		if (!set_bool(conf, p, &val)) return ERR;
#define CONF_FLAG(name, default) \
		if (strview_eq(p->keys + 1, &STRVIEW(#name))) { \
			conf->flags.name = val; \
			return OK; \
		}
		CONF_FLAGS
#undef CONF_FLAG

		logerr("unknown flag on line %d");
		return ERR;
	}
	case '"': case '[':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		if (!set_opt(conf, p)) return ERR;
		break;
	case '{':
		if (!set_table(conf, p, hf)) return ERR;
		break;
	default:
		logerrf("unknown type on line %d", p->line);
		break;
	}

check_newln:
	// Expect new line
	if (!parse_whitespace(p, !expect_newln)) return ERR;
	if (expect_newln) {
		if (*p->src != '\n' && *p->src != '\0') {
			logerrf("expected newline on line %d", p->line);
			return ERR;
		}
		if (*p->src == '\n') p->src++, p->line++;
	} else {
		if (*p->src != ',' && *p->src != '}') {
			logerrf("expected '}' or ',' on line %d", p->line);
			return ERR;
		}
		if (*p->src == ',') p->src++;
	}

	p->nkeys = reset_keys;
	if (p->nkeys < 2 && p->color) {
		color_init(p->color, &p->color_args);
		p->color = NULL;
	}

	return OK;
}

static enum result set_table(struct conf *conf, struct parser *p,
				const struct host_features *hf) {
	p->src++;
	while (*p->src != '}') {
		if (!parse_kv(conf, p, hf, false)) return ERR;
	}
	p->src++;

	return OK;
}

enum result conf_load(struct conf *conf, const struct host_features *hf,
			const char *str) {
	enum result ret = OK;

	conf_default(conf, hf);
	struct parser p = {
		.src = str,
		.line = 1,
		.esc = vec_init(1, 64),
	};

	while (*p.src != '\0') {
		if (!parse_kv(conf, &p, hf, true)) {
			ret = ERR;
			goto cleanup;
		}
	}
	if (p.color) {
		color_init(p.color, &p.color_args);
		p.color = NULL;
	}

cleanup:
	vec_deinit(p.esc);
	return ret;
}

void conf_deinit(struct conf *conf) {
#define CONF_OPT(name, type, _default, min, max) \
	_Generic(conf->options.name, \
		char * : opt_free_str, \
		default : opt_free_none)((void *)&conf->options.name);
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

