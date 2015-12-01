#ifndef SHED_H
#define SHED_H
#include "stdint.h"

typedef enum {WAITING, RUNNING, TERMINATED} state;

struct pcb_s {
	uint32_t R[13];
	uint32_t LR_SVC;
	uint32_t LR_USER;
	uint32_t CPSR_USER;
	void * sp;
	struct pcb_s * next;
	state currentState;
};

typedef int(func_t) (void);
struct pcb_s * create_process(func_t* entry);
void sched_init();

void sys_yieldto(struct pcb_s* dest);
void do_sys_yieldto(int * new_stack);
void sys_yield();
void do_sys_yield(int * new_stack);
void sys_exit();
void do_sys_exit(int * new_stack);

#endif
