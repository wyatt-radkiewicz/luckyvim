#ifndef _term_h_
#define _term_h_

#include <stdio.h>

#include "lucky.h"

#define _esc "\x1b"
#define _csi _esc "["
#define _tui_mode _csi "?1049h"
#define _norm_mode _csi "?1049l"
#define _tui_clear _csi "2J"
#define _tui_home _csi "H"
#define _cursor_down _csi "%dB"

enum result term_init(void);
void term_deinit(void);
uint32_t term_width(void);
uint32_t term_height(void);
void term_set_resize_cb(void (*cb)(uint32_t w, uint32_t h));

#endif

