#ifndef _term_h_
#define _term_h_

#define ESC "\x1b"
#define CSI ESC "["

struct host_features {
	int color_depth;
};

enum result host_features_find(struct host_features *hf);

#endif

