#ifndef SHED_H
#define SHED_H
#include "stdint.h"

typedef enum {WAITING, RUNNING, TERMINATED} state;
typedef enum {COLLABORATIVE, PREEMPTIVE, FPP} schedMethod;

struct pcb_s {
	//Process data
	uint32_t R[13];
	uint32_t LR_SVC;
	uint32_t LR_USER;
	uint32_t CPSR_USER;
	void * sp;
	
	//Needed for collaborative ordonnancing
	struct pcb_s * next;
	state currentState;
	int codeRetour;
	
	//Needed for FPP ordonnancing
	int basePriority;
	int priority;
};

void __attribute__((naked)) irq_handler(void);

typedef int(func_t) (void);
struct pcb_s * create_process(func_t* entry);
struct pcb_s * create_fpp_process(func_t* entry, int basePriority, int priority);
void sched_init(schedMethod method);

void sys_yield();
void do_sys_yield(int * new_stack);
void sys_exit(int codeRetour);
void do_sys_exit(int * new_stack, int codeRetour);

#endif
