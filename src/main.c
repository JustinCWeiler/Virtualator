#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <xed/xed-interface.h>

#include <sys/mman.h>
#include <errno.h>

#include "segfault/handler.h"

#define NBYTES 15

void insert(char* buf, uintptr_t rip, int i, int n);

void func(char* buf) {
	uintptr_t rip;
	asm volatile ("leaq (%%rip), %0" : "=r" (rip));
	int n = 0;
	while (rip >> 4*n) n++;
	for (int i = 0; i < n; i++) {
		insert(buf, rip, i, n);
	}
	buf[0] = '0';
	buf[1] = 'x';
	buf[n + 2] = 0;
}

void insert(char* buf, uintptr_t rip, int i, int n) {
	char val = (rip >> (4*n - 4*i - 4)) & 0xf;
	if (val < 0xa)
		val += '0';
	else
		val += 'a' - 0xa;
	buf[i + 2] = val;
}

void _end_(void) {}

int main(void) {
	void* dev_info[][3] = {
		{(void*)0x1000, (void*)0x2000, NULL},
		{(void*)0xfe000000, (void*)0xfe100000, NULL}
	};
	const size_t n_dev = sizeof(dev_info)/sizeof(dev_info[0]);

	setup_handler(n_dev, dev_info);

	volatile int* mem = mmap((void*)0x20000000, 0x10000000, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
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
		mem[i] = 0xdeadbeef;
	}

	printf("executing in mmap space test\n");
	// values below 0x10000 must be run with sudo
	// smth smth linux security
	// in practice (for raspi a+) this will be 0x8000,
	// which means it will have to be ran with sudo
	void* addr = (void*)0x10000;
	char* imem = mmap(addr, 0x1000, PROT_EXEC | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if ((int)(uintptr_t)imem == -1) {
		printf("Error %d: %s\n", errno, strerror(errno));
		return 1;
	}
	size_t s = (size_t)_end_ - (size_t)func;
	memcpy(addr, func, s);
	char buf[1024];
	((void(*)(char*))addr)(buf);

	printf("code executed at: %s\n", buf);

	return 0;
}
