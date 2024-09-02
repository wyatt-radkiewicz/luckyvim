#include <stdbool.h>
#include <unistd.h>

#include "term.h"

int main(int argc, char **argv) {
	if (!term_init()) return -1;

	printf("%d %d\n", term_width(), term_height());

	bool running = true;
	while (running) {
		char c;
		
		read(1, &c, 1);
		printf("%d\n", c);
		if (c == 'q') running = false;
	}

	term_deinit();	
	return 0;
}

