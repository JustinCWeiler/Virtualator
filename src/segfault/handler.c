#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <xed/xed-interface.h>

#define __USE_GNU
#include <signal.h>

#include "segfault/handler.h"

#define MAXBYTES 15

#define FETCH_FAULT (4)
#define WRITE_FAULT (1)

#define GET_BIT(v, n) (((v)>>(n))&1)

#define MMODE XED_MACHINE_MODE_LONG_64
#define STACK_ADDR_WIDTH XED_ADDRESS_WIDTH_64b

__attribute__((unused)) static const xed_state_t dstate = { .mmode = XED_MACHINE_MODE_LONG_64, .stack_addr_width = XED_ADDRESS_WIDTH_64b };

static const size_t PAGE_SIZE = 4 << 10;
static const size_t PAGE_ALIGN = ~(PAGE_SIZE - 1);

static int handler_enabled = 0;

/*
 * format of device_info:
 * [
 *     [device_1 lo, device_1 hi, device_1_handler],
 *     ...
 *     [device_n lo, device_n hi, device_n_handler],
 * ]
 *
 * */
static void*** dev_info;
static size_t n_dev;
static void* dev_bounds[2];

typedef void (*device_handler_t)(uintptr_t addr, rw_val_t rw, size_t width, uint64_t val);

static inline int dev_addr(void* addr, void** bounds) {
	return n_dev > 0 && bounds[0] <= addr && addr <= bounds[1];
}

static inline uint64_t access_val_info(__attribute__((unused)) xed_decoded_inst_t* xedd, size_t* ret_width) {
	static int i = 0;

	if (i++ % 2 == 0) {
		*ret_width = 64;
		return 0xdeadbeeffeedface;
	}
	else {
		*ret_width = 32;
		return 0xdeadbeef;
	}
}

static void handler(int sig, siginfo_t* info, ucontext_t* ctx) {
	if (sig != SIGSEGV)
		abort(); // should never happen

	if (!handler_enabled) {
		fprintf(stderr, "Segfault occurred at %p with handler disabled\n", (void*)ctx->uc_mcontext.gregs[REG_RIP]);
		abort();
	}

	// get info about exception
	void* fault_addr = (void*)ctx->uc_mcontext.gregs[REG_RIP];
	void* mem_addr = info->si_addr;
	greg_t error_code = ctx->uc_mcontext.gregs[REG_ERR];

	// use device handler
	if (dev_addr(mem_addr, dev_bounds)) {
		// get info about instruction that faulted
		xed_decoded_inst_t xedd;
		xed_decoded_inst_zero(&xedd);
		xed_decoded_inst_set_mode(&xedd, MMODE, STACK_ADDR_WIDTH);
		xed_decode(&xedd, fault_addr, MAXBYTES);
		rw_val_t rw = GET_BIT(error_code, WRITE_FAULT);
		size_t width;
		uint64_t val = access_val_info(&xedd, &width);

		for (size_t i = 0; i < n_dev; i++) {
			if (dev_addr(mem_addr, dev_info[i])) {
				((device_handler_t)dev_info[i][2])((uintptr_t)mem_addr, rw, width, val);
				break;
			}
		}

		ctx->uc_mcontext.gregs[REG_RIP] += xed_decoded_inst_get_length(&xedd);
	}
	// handle like normal memory
	else {
		// TODO
	}
}

void setup_handler(size_t n, void* device_info[][3]) {
	xed_tables_init();

	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = (void(*)(int,siginfo_t*,void*))handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);

	// malloc main array
	dev_info = malloc(n * sizeof(void**));

	// init dev_min and dev_max
	if (n > 0) {
		dev_bounds[0] = device_info[0][0];
		dev_bounds[1] = device_info[0][1];
	}

	// init n_dev
	n_dev = n;

	for (size_t i = 0; i < n; i++) {
		dev_info[i] = malloc(3 * sizeof(void*));

		void* lo = device_info[i][0];
		void* hi = device_info[i][1];

		dev_info[i][0] = lo;
		dev_info[i][1] = hi;
		dev_info[i][2] = device_info[i][2];

		if (dev_bounds[0] > lo)
			dev_bounds[0] = lo;
		if (dev_bounds[1] < hi)
			dev_bounds[1] = hi;
	}

	// round dev_bounds[0] down and dev_bounds[1] up
	dev_bounds[0] = (void*)((uintptr_t)dev_bounds[0] & PAGE_ALIGN);
	dev_bounds[1] = (void*)(((uintptr_t)dev_bounds[1] + PAGE_SIZE - 1) & PAGE_ALIGN);

	handler_enabled = 1;
}

void enable_handler(void) { handler_enabled = 1; }
void disable_handler(void) { handler_enabled = 0; }
