#ifndef _conf_h_
#define _conf_h_

#include "common.h"

#define CONF_FLAGS \
	CONF_FLAG(relative_numbers, true) \
	CONF_FLAG(show_line_numbers, true) \
	CONF_FLAG(show_tab_line, true) \
	CONF_FLAG(expand_tab, false)
#define CONF_OPTS \
	CONF_OPT(tab_width, uint8_t, 8, 1, 16) \
	CONF_OPT(splash_screen, char *, alloc_default_string( \
"                     .-===-\n" \
"                  .====             ____\n" \
"                 ====          .=====\n" \
"                 ===    ___/|\\========..-\n" \
"                   '=====/====\\=.............\n" \
"                  =.====.|====|=====.=.....:..,.              __\n" \
"                ==....===|=|===\\=\\/====\\==.=  '-            /  \\n" \
"               =....==\\_/|\\===|| .\"\\\\====|====.              |  /-+\\n" \
"    /\"\"\\      ./...=/==/\"'-|==| |' \\ .\\===|=\\==.          _ /  / __|\n" \
"   |    |    / /=====/___|  |/  '..='\" \\==|=\\==|    __.-\"\" |   >'   |\n" \
"  __\\__  '-,_' |====/   \"\":=|'   '\"\"===|==|||  \\|_-''/ /  /    >   /\n" \
" /     '-,  \\_|===/=|==\"'  |  __ .     \\=| ',-''   |  |  '\\   >'\".|\n" \
"|  .___  /  \\  \\==/|=\\      /~"  "\\    ,'==\'       \\   \\   \\---T-'\n" \
"\\     '''\\   |  \\=||==\\.   \\       /_-'|\\===\\        '\\  \"-.__.-\"\n" \
" |  ____-<   |   \\||===\'\"\"-T-----T\"   /||==|          '/\"\"\"\n" \
"  \\  ___,/.  /   / \\===\\    \\--..\"/   / /|==|_____.---\"\"\n" \
"   '_    /.-\"  -\"   \\==|\\.   '\\  /  /' :_|=/\"\n" \
"     '\"''.=-\".__.-T\"-+==\\ T.   \\/ .\" |T==|/\n" \
"          _.-'===/'=/='\\=\\| \"-._T/  /`|=='\\" \
	), 0, 0)

struct conf {
	struct color colors[COLOR_MAX];

	struct {
#define CONF_OPT(name, type, default, min, max) type name;
		CONF_OPTS
#undef CONF_OPT
	} options;

	struct {
#define CONF_FLAG(name, default) bool name : 1;
		CONF_FLAGS
#undef CONF_FLAG
	} flags;
};

void conf_default(struct conf *conf, int color_bit_depth);
enum result conf_load(struct conf *conf, int color_bit_depth, const char *str);
void conf_deinit(struct conf *conf);

#endif

