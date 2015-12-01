#include "sched.h"
#include "util.h"
#include "syscall.h"
#include "kheap.h"
#include "hw.h"
#include "asm_tools.h"

struct pcb_s * current_process, * root_process, * last_process;

void __attribute__((naked)) irq_handler(void){
	int lr_irq;
	__asm("mov %0, lr" : "=r"(lr_irq));	
	
	//Passage en SVC
	__asm("cps 0x13");
	__asm("push {%0}" : : "r"(lr_irq-4));
	__asm("stmfd sp!, {r0-r12}");
	
	int * new_stack;
	__asm("mov %0, sp" : "=r"(new_stack));	
	
	//Changement de contexte
	do_sys_yield(new_stack);
	__asm("ldmfd sp!, {r0-r12}");
	__asm("pop {%0}" : "=r"(lr_irq));
	set_next_tick_default();
	ENABLE_TIMER_IRQ();
	ENABLE_IRQ();
	__asm("cps 0x10");
	
	//Jump
	__asm("mov pc, %0" : : "r"(lr_irq));
	
}

void sched_init(){
	kheap_init();
	
	//Scheduler related
	current_process = (struct pcb_s*) kAlloc(sizeof(struct pcb_s));
	void * sp = kAlloc(10*1024);
	current_process->sp = sp + 2560;
	current_process->CPSR_USER = 0x150;
	current_process->currentState = RUNNING;
	current_process->next = current_process;
	root_process = current_process;
	last_process = current_process;
}

struct pcb_s * create_process(func_t* entry){
	struct pcb_s* s = (struct pcb_s*) kAlloc(sizeof(struct pcb_s));
	void * sp = kAlloc(10*1024);
	s->sp = sp + 2560;
	s->LR_SVC = (uint32_t)entry;
	s->LR_USER = (uint32_t)entry;
	s->CPSR_USER = 0x150;
	s->currentState =  WAITING;
	s->next = last_process->next;
	last_process->next = s;
	last_process = s;
	return s;
}

void elect()
{
    while(current_process->next->currentState == TERMINATED) {
		if(current_process->next == current_process) {
			terminate_kernel();
		}
		struct pcb_s * next = current_process->next->next;
		kFree((void*) current_process->next, sizeof(struct pcb_s*));
		current_process->next = next;
	}
    current_process = current_process->next;
}

void do_sys_yieldto(int * new_stack){
	//Get new process
	struct pcb_s* dest = (struct pcb_s*) *(new_stack + 1);
	
	//Backup old process
	__asm("mrs %0, spsr" : "=r"(current_process->CPSR_USER));
	for(int i=0;i<13;i++) {
		current_process->R[i] = *(new_stack + i);
	}
	current_process->LR_SVC = *(new_stack + 13);
	current_process->currentState = WAITING;
	
	//Apply new process
	current_process = dest;
	__asm("msr spsr, %0" : : "r"(current_process->CPSR_USER));
	for(int i=0;i<13;i++) {
		*(new_stack + i) = current_process->R[i];
	}
	*(new_stack + 13) = current_process->LR_SVC;
	current_process->currentState = RUNNING;
}

void sys_yieldto(struct pcb_s* dest){
	__asm("mov %0, lr" : "=r"(current_process->LR_USER));
	__asm("mov %0, sp" : "=r"(current_process->sp));
	__asm("mov r0, %0" : : "r"(5));
	__asm("mov r1, %0" : : "r"(dest));
	__asm("SWI #0");
	__asm("mov lr, %0" : : "r"(current_process->LR_USER));
	__asm("mov sp, %0" : : "r"(current_process->sp));
}

void do_sys_yield(int * new_stack){
	//Backup old process
	__asm("mrs %0, spsr" : "=r"(current_process->CPSR_USER));
	for(int i=0;i<13;i++) {
		current_process->R[i] = *(new_stack + i);
	}
	current_process->LR_SVC = *(new_stack + 13);
	
	//Apply new process
	elect();
	__asm("msr spsr, %0" : : "r"(current_process->CPSR_USER));
	for(int i=0;i<13;i++) {
		*(new_stack + i) = current_process->R[i];
	}
	*(new_stack + 13) = current_process->LR_SVC;
}

void sys_yield(){
	__asm("mov %0, lr" : "=r"(current_process->LR_USER));
	__asm("mov %0, sp" : "=r"(current_process->sp));
    __asm("mov r0, %0" : : "r"(6));
    __asm("SWI #0");
    __asm("mov lr, %0" : : "r"(current_process->LR_USER));
	__asm("mov sp, %0" : : "r"(current_process->sp));
}

void do_sys_exit(int * new_stack, int codeRetour){
	//End old process
	current_process->currentState = TERMINATED;
	current_process->codeRetour = codeRetour;
	
	//Apply new process
	elect();
	__asm("msr spsr, %0" : : "r"(current_process->CPSR_USER));
	for(int i=0;i<13;i++) {
		*(new_stack + i) = current_process->R[i];
	}
	*(new_stack + 13) = current_process->LR_SVC;
	current_process->currentState = RUNNING;
}

void sys_exit(int codeRetour){
    __asm("mov r0, %0" : : "r"(7));
    __asm("mov r1, %0" : : "r"(codeRetour));
    __asm("SWI #0");
}
