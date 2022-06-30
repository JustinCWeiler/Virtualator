#ifndef _SEGFAULT_HANDLER_H_
#define _SEGFAULT_HANDLER_H_

#include <stdint.h>

int setup_handler(size_t n, void* device_info[][3]);
void enable_handler(void);
void disable_handler(void);

#endif
