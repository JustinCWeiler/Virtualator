#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xed/xed-interface.h>

#include <errno.h>
#include <sys/mman.h>

#include "segfault/handler.h"

#define NBYTES 15

void handler( void* addr, rw_val_t rw, size_t width, uint64_t val ) {
	printf( "program tried to %s %p with width %lu and value 0x%lx\n",
	        rw == READ ? "read from" : "write to",
	        addr,
	        width,
	        val );
}

int main( void ) {
	void* dev_info[][3] = { { (void*)0xfe000000, (void*)0xfe100000, handler } };
	const size_t n_dev = sizeof( dev_info ) / sizeof( dev_info[0] );

	setup_handler( n_dev, dev_info );

	*(volatile int*)0xfe000000;
	*(volatile int*)0xfe000000 = 0;

	return 0;
}
