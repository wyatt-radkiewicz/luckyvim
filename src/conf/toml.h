#ifndef _toml_h_
#define _toml_h_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

//
// TOML Structure
//
enum tomltype {
	TOML_STRING,
	TOML_INT,
	TOML_FLOAT,
	TOML_BOOL,
	TOML_TIME,
	TOML_ARRAY,
	TOML_TABLE,
};
struct tomltable;
struct tomltime_local {
	bool has_date;
	bool has_time;

	struct {
		int year, mon, day;
	} date;
	struct {
		int hour, min, sec;
		long nano;
	} time;
};
struct tomltime {
	bool islocal;

	union {
		struct tomltime_local local;
		struct timespec abs;
	};
};
struct tomlval {
	enum tomltype type;
	union {
		const char *tstring;
		int64_t tint;
		double tfloat;
		bool tbool;
		const struct tomltime *ttime;
		const struct tomlval *tarray;
		const struct tomltable *ttable;
	};
};
struct toml;

struct toml *toml_parse(const char *const src);
void toml_deinit(struct toml *const toml);
const struct tomltable *toml_root(const struct toml *const toml);
const struct tomlval *toml_get(const struct tomltable *const table);
size_t toml_arrlen(const struct tomlval *const array);
#define toml_isstr(tomlval) ((tomlval)->type == TOML_STRING)
#define toml_isint(tomlval) ((tomlval)->type == TOML_INT)
#define toml_isflt(tomlval) ((tomlval)->type == TOML_FLOAT)
#define toml_isbool(tomlval) ((tomlval)->type == TOML_BOOL)
#define toml_istime(tomlval) ((tomlval)->type == TOML_TIME)
#define toml_isarr(tomlval) ((tomlval)->type == TOML_ARRAY)
#define toml_istable(tomlval) ((tomlval)->type == TOML_TABLE)

#endif

