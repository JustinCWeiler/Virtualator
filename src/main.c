#include <stdio.h>
#include <string.h>
#include <xed/xed-interface.h>

static const xed_machine_mode_enum_t mmode = XED_MACHINE_MODE_LONG_64;
static const xed_address_width_enum_t stack_addr_width = XED_ADDRESS_WIDTH_64b;

#define NBYTES 15

int main(void) {
	xed_tables_init();

	xed_decoded_inst_t xedd;
	memset(&xedd, 0, sizeof(xedd));
	xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);

	xed_ild_decode(&xedd, (void*)main, 15);
	printf("instruction length: %d\n", xedd._decoded_length);

	xedd._decoded_length = 0;

	xed_ild_decode(&xedd, (unsigned char*)main + 6, 15);
	printf("instruction length: %d\n", xedd._decoded_length);

	return 0;
}
