#ifndef _fs_h_
#define _fs_h_

#include <stdbool.h>

char *file_load_as_str(const char *path);
bool file_exists(const char *path);
char *get_realpath(const char *path);

#endif

