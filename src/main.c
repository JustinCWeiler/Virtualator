#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <sys/mman.h>

#include "segfault/handler.h"

#define NBYTES 15

extern void _memcpy( void* dst, void* src, size_t n );

void handler( void* addr, rw_val_t rw, size_t width, uint64_t val ) {
	if ( rw == READ ) {
		printf( "program tried to read from %p with width %lu\n", addr, width );
	}
	else if ( rw == WRITE ) {
		printf( "program tried to write to %p with width %lu and value 0x%lx\n", addr, width, val );
	}
}

int main( void ) {
	void* dev_info[][3] = { { (void*)0xfe000000, (void*)0xfe100000, handler } };
	const size_t n_dev = sizeof( dev_info ) / sizeof( dev_info[0] );

	setup_handler( n_dev, dev_info );

	register volatile uint32_t x = 0;
	register volatile uint64_t y = 0;

	*(volatile uint32_t*)0xfe000000;
	*(volatile uint32_t*)0xfe000000 = 0;
	*(volatile uint32_t*)0xfe000000 = x;

	*(volatile uint64_t*)0xfe000000;
	*(volatile uint64_t*)0xfe000000 = 0;
	*(volatile uint64_t*)0xfe000000 = y;

	_memcpy( (void*)0xfe000000, (void*)0xfe000100, 2 );

	return 0;
}
