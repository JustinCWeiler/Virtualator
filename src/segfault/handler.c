#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <xed/xed-interface.h>

#define __USE_GNU
#include <signal.h>

#include "segfault/handler.h"

#define MAXBYTES 15

#define FETCH_FAULT (1<<4)
#define WRITE_FAULT (1<<1)

static const xed_machine_mode_enum_t mmode = XED_MACHINE_MODE_LONG_64;
static const xed_address_width_enum_t stack_addr_width = XED_ADDRESS_WIDTH_64b;

static int handler_enabled = 0;

/*
 * format of device_info:
 * [
 *     [device_1 lo, device_1 hi, device_1_handler],
 *     ...
 *     [device_n lo, device_n hi, device_n_handler],
 *     null
 * ]
 *
 * type of device_i_handler:
 * void (*)(uintptr_t addr, int rw, size_t width, uint64_t val)
 * */
static void*** dev_info;
static void* dev_min;
static void* dev_max;

static void handler(int sig, siginfo_t* info, ucontext_t* ctx) {
	if (sig != SIGSEGV)
		abort();

	if (!handler_enabled) {
		fprintf(stderr, "Segfault occurred at %p with handler disabled\n", (void*)ctx->uc_mcontext.gregs[REG_RIP]);
		abort();
	}

	xed_decoded_inst_t xedd;
	xed_decoded_inst_zero(&xedd);
	xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);

	// get info about exception
	void* addr = (void*)ctx->uc_mcontext.gregs[REG_RIP];
	greg_t error_code = ctx->uc_mcontext.gregs[REG_ERR];
	xed_decode(&xedd, addr, MAXBYTES);

	// print info
	printf("User program faulted at %p trying to ", addr);
	// instruction fetch
	if (error_code & FETCH_FAULT)
		printf("execute code at ");
	// write
	else if (error_code & WRITE_FAULT)
		printf("write to ");
	// read
	else
		printf("read from ");
	printf("%p\n", info->si_addr);

	// get length of instruction that faulted
	ctx->uc_mcontext.gregs[REG_RIP] += xed_decoded_inst_get_length(&xedd);
}

int setup_handler(size_t n, void* device_info[][3]) {
	xed_tables_init();

	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = (void(*)(int,siginfo_t*,void*))handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);

	dev_info = malloc((n + 1) * sizeof(void**));

	if (n != 0) {
		dev_min = device_info[0][0];
		dev_max = device_info[0][1];

		for (size_t i = 0; i < n; i++) {
			dev_info[i] = malloc(3 * sizeof(void*));
			void* lo = device_info[i][0];
			void* hi = device_info[i][1];
			dev_info[i][0] = lo;
			dev_info[i][1] = hi;
			dev_info[i][2] = device_info[i][2];

			if (dev_min > lo)
				dev_min = lo;
			if (dev_max < hi)
				dev_max = hi;
		}

		dev_info[n] = NULL;
	}

	handler_enabled = 1;

	return 0;
}

void enable_handler(void) { handler_enabled = 1; }
void disable_handler(void) { handler_enabled = 0; }
