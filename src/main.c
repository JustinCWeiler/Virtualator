#include <stdio.h>
#include <string.h>
#include <xed/xed-interface.h>

#include "segfault/handler.h"

#define NBYTES 15

int main(void) {
	setup_handler();

	*(volatile int*)1;

	return 0;
}
