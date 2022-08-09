#ifndef _SEGFAULT_HANDLER_H_
#define _SEGFAULT_HANDLER_H_

#include <stddef.h>

// Device handler function has following signature:
// void device_handler(uintptr_t addr, rw_val_t rw, size_t width, uint64_t val);
void setup_handler( size_t n, void* device_info[][3] );
void enable_handler( void );
void disable_handler( void );

typedef enum { READ, WRITE } rw_val_t;

#endif
