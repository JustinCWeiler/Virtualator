#include <Zydis/Zydis.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_GNU
#include <signal.h>

#include "segfault/handler.h"

#define MAXBYTES 15

#define FETCH_FAULT ( 4 )
#define WRITE_FAULT ( 1 )

#define GET_BIT( v, n ) ( ( ( v ) >> ( n ) ) & 1 )

#define MMODE ZYDIS_MACHINE_MODE_LONG_64
#define STACK_ADDR_WIDTH ZYDIS_STACK_WIDTH_64

static const size_t PAGE_SIZE = 4 << 10;
static const size_t PAGE_ALIGN = ~( PAGE_SIZE - 1 );

static ZydisDecoder dec;

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

typedef void ( *device_handler_t )( uintptr_t addr, rw_val_t rw, size_t width, uint64_t val );

static int dev_addr( void* addr, void* bounds[] ) {
	return n_dev > 0 && bounds[0] <= addr && addr <= bounds[1];
}

typedef struct {
	struct {
		uint32_t read : 1;
		uint32_t write : 1;
		uint32_t fetch : 1;
	} rwf;
	uint32_t width;
	size_t val;
	void* dst;
} fault_type_t;

static void access_fault_info( ZydisDecodedInstruction* inst,
                               ZydisDecodedOperand* ops,
                               ucontext_t* ctx,
                               fault_type_t* ret ) {
	(void)ctx;
	ret->width = 0;
	ret->rwf.read = 0;
	ret->rwf.write = 0;
	ret->rwf.fetch = 0;
	for ( ZyanU8 i = 0; i < inst->operand_count; i++ ) {
		if ( ops[i].type == ZYDIS_OPERAND_TYPE_MEMORY ) {
			if ( ret->width != 0 ) assert( ret->width == ops[i].size );
			ret->width = ops[i].size;
			if ( ops[i].actions == ZYDIS_OPERAND_ACTION_READ ) {
				ret->rwf.read = 1;
			}
			else if ( ops[i].actions == ZYDIS_OPERAND_ACTION_WRITE ) {
				ret->rwf.write = 1;
			}
		}
		else if ( ops[i].type == ZYDIS_OPERAND_TYPE_REGISTER ) {
			// TODO
		}
		else if ( ops[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE ) {
			// TODO
		}
	}
	// TODO
	ret->val = 0xdeadbeeffeedface;
}

static void handler( int sig, siginfo_t* info, ucontext_t* ctx ) {
	if ( sig != SIGSEGV ) abort(); // should never happen

	if ( !handler_enabled ) {
		fprintf(
		  stderr, "Segfault occurred at %p with handler disabled\n", (void*)ctx->uc_mcontext.gregs[REG_RIP] );
		abort();
	}

	// get info about exception
	void* fault_addr = (void*)ctx->uc_mcontext.gregs[REG_RIP];
	void* mem_addr = info->si_addr;

	// use device handler
	if ( dev_addr( mem_addr, dev_bounds ) ) {
		// get info about instruction that faulted
		ZydisDecodedInstruction inst;
		ZydisDecodedOperand ops[ZYDIS_MAX_OPERAND_COUNT];
		ZydisDecoderDecodeFull(
		  &dec, fault_addr, ZYDIS_MAX_INSTRUCTION_LENGTH, &inst, ops, ZYDIS_MAX_OPERAND_COUNT, 0 );

		fault_type_t ft;
		access_fault_info( &inst, ops, ctx, &ft );

		rw_val_t rw = ft.rwf.read ? READ : WRITE;

		for ( size_t i = 0; i < n_dev; i++ ) {
			if ( dev_addr( mem_addr, dev_info[i] ) ) {
				( (device_handler_t)dev_info[i][2] )( (uintptr_t)mem_addr, rw, ft.width, ft.val );
				break;
			}
		}

		ctx->uc_mcontext.gregs[REG_RIP] += inst.length;
	}
	// handle like normal memory
	else {
		// TODO
	}
}

void setup_handler( size_t n, void* device_info[][3] ) {
	struct sigaction act;
	sigemptyset( &act.sa_mask );
	act.sa_sigaction = (void ( * )( int, siginfo_t*, void* ))handler;
	act.sa_flags = SA_SIGINFO;
	sigaction( SIGSEGV, &act, NULL );

	// malloc main array
	dev_info = malloc( n * sizeof( void** ) );

	// init dev_min and dev_max
	if ( n > 0 ) {
		dev_bounds[0] = device_info[0][0];
		dev_bounds[1] = device_info[0][1];
	}

	// init n_dev
	n_dev = n;

	for ( size_t i = 0; i < n; i++ ) {
		dev_info[i] = malloc( 3 * sizeof( void* ) );

		void* lo = device_info[i][0];
		void* hi = device_info[i][1];

		dev_info[i][0] = lo;
		dev_info[i][1] = hi;
		dev_info[i][2] = device_info[i][2];

		if ( dev_bounds[0] > lo ) dev_bounds[0] = lo;
		if ( dev_bounds[1] < hi ) dev_bounds[1] = hi;
	}

	// round dev_bounds[0] down and dev_bounds[1] up
	dev_bounds[0] = (void*)( (uintptr_t)dev_bounds[0] & PAGE_ALIGN );
	dev_bounds[1] = (void*)( ( (uintptr_t)dev_bounds[1] + PAGE_SIZE - 1 ) & PAGE_ALIGN );

	// initialize zydis decoder
	ZydisDecoderInit( &dec, MMODE, STACK_ADDR_WIDTH );

	handler_enabled = 1;
}

void enable_handler( void ) {
	handler_enabled = 1;
}
void disable_handler( void ) {
	handler_enabled = 0;
}
