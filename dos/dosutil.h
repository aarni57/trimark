#ifndef DOSUTIL_H
#define DOSUTIL_H

#define disable_interrupts() asm volatile ("cli")
#define enable_interrupts() asm volatile ("sti")

#endif
