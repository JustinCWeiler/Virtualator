#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <xed/xed-interface.h>

#define __USE_GNU
#include <signal.h>

#include "segfault/handler.h"

#define MAXBYTES 15

#define FETCH_FAULT (1<<4)
#define WRITE_FAULT (1<<1)

static const xed_machine_mode_enum_t mmode = XED_MACHINE_MODE_LONG_64;
static const xed_address_width_enum_t stack_addr_width = XED_ADDRESS_WIDTH_64b;

static xed_decoded_inst_t xedd;

static void handler(int sig, siginfo_t* info, ucontext_t* ctx) {
	if (sig != SIGSEGV)
		abort();

	// get info about exception
	void* addr = (void*)ctx->uc_mcontext.gregs[REG_RIP];
	greg_t error_code = ctx->uc_mcontext.gregs[REG_ERR];

	// print info
	printf("User program faulted trying to ");
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
	xed_ild_decode(&xedd, addr, MAXBYTES);
	ctx->uc_mcontext.gregs[REG_RIP] += xedd._decoded_length;
	xedd._decoded_length = 0;
}

void setup_handler(void) {
	xed_tables_init();

	memset(&xedd, 0, sizeof(xedd));
	xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);

	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = (void(*)(int,siginfo_t*,void*))handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, NULL);
}
