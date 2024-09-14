#include "toml.h"

struct tomlkv {
	uint32_t name;
	struct tomlval val;
};
struct tomltable {
	uint32_t num_used, capacity;
	struct tomlkv entries[];
};

struct toml {
	char *strings;
	struct tomltime *times;
	struct tomltable *root;
};

struct toml *toml_parse(const char *const src) {
	return NULL;
}
void toml_deinit(struct toml *const toml) {
	;
}
const struct tomltable *toml_root(const struct toml *const toml) {
	return toml->root;
}
const struct tomlval *toml_get(const struct tomltable *const table) {
	return NULL;
}
size_t toml_arrlen(const struct tomlval *const array) {
	return 0;
}

