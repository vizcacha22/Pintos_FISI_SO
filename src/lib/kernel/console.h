#ifndef __LIB_KERNEL_CONSOLE_H
#define __LIB_KERNEL_CONSOLE_H

void console_init (void);
void console_panic (void);
void console_print_stats (void);
void putchar_no_lock(char c);

#endif /**< lib/kernel/console.h */
