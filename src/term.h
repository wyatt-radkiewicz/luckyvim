#ifndef _term_h_
#define _term_h_

#include <stdio.h>

#include "lucky.h"

enum result term_init(void);
void term_deinit(void);
u32 term_width(void);
u32 term_height(void);

#endif

