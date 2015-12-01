#ifndef SYSCALL_H
#define SYSCALL_H
#include "stdint.h"
#include "sched.h"

void sys_reboot();
void sys_nop();
void sys_settime();
uint64_t sys_gettime();

void do_sys_reboot(void);
void do_sys_nop(void);
void do_sys_settime(void);
void do_sys_gettime(void);

void __attribute__((naked)) swi_handler(void);

#endif