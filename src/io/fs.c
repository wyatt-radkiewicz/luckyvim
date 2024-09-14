#include <unistd.h>
#include <wordexp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#include "fs.h"

char *file_load_as_str(const char *path) {
	struct stat st;
	if (stat(path, &st) != 0) return NULL;

	FILE *fp = fopen(path, "r");
	if (!fp) return NULL;
	char *str = malloc(st.st_size + 1);
	fread(str, st.st_size, 1, fp);
	fclose(fp);
	str[st.st_size] = '\0';
	return str;
}

bool file_exists(const char *path) {
	struct stat st;
	return stat(path, &st) == 0;
}

char *get_realpath(const char *path) {
	wordexp_t exp = {0};
	if (wordexp(path, &exp, WRDE_NOCMD) != 0) return NULL;
	size_t exp_len = 0;
	for (int i = 0; i < exp.we_wordc; i++) exp_len += strlen(exp.we_wordv[i]);

	char *rel = malloc(exp_len + 1);
	if (!rel) return NULL;
	rel[0] = '\0';

	for (int i = 0; i < exp.we_wordc; i++) strcat(rel, exp.we_wordv[i]);

	char *rpath = realpath(rel, NULL);
	free(rel);
	if (!rpath) return NULL;
	return rpath;
}
