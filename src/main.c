#include <stdio.h>
#include <string.h>
#include <xed/xed-interface.h>

#include <sys/mman.h>
#include <errno.h>

#include "segfault/handler.h"

#define NBYTES 15

int main(void) {
	setup_handler();

	volatile int* mem = mmap((void*)0xfe000000, 16, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if ((int)(uintptr_t)mem == -1) {
		printf("Error: %s\n", strerror(errno));
		return 1;
	}

	printf("read test\n");
	for (int i = 0; i < 16; i++) {
		mem[i];
	}

	printf("write test\n");
	for (int i = 0; i < 16; i++) {
		mem[i] = i;
	}

	return 0;
}
