#include "util.h"
#include "hw.h"
#include "syscall.h"

#define PANIC() do { kernel_panic(__FILE__,__LINE__) ; } while(0)

void sys_reboot()
{
	__asm("mov r0, %0" : : "r"(1));
	__asm("SWI #0");
}

void sys_nop()
{
	__asm("mov r0, %0" : : "r"(2));
	__asm("SWI #0");
}

void sys_settime(uint64_t date_ms){
	__asm("mov r0, %0" : : "r"(3));
	int first32 = date_ms >> 32;
	int last32 = date_ms;
	__asm("mov r1, %0" : : "r"(first32));
	__asm("mov r2, %0" : : "r"(last32));
	__asm("SWI #0");
}

uint64_t sys_gettime(){
	__asm("mov r0, %0" : : "r"(4));
    __asm("SWI #0");
    uint32_t first32, last32;
    __asm("mov %0, r0" : "=r"(first32));
    __asm("mov %0, r1" : "=r"(last32));
    uint64_t date_ms = ((uint64_t)first32) << 32 | last32;
    return date_ms;
}

void do_sys_reboot(void){
	__asm("mov PC, %0" : :"r"(0x8000));
}

void do_sys_nop(void){
	
}

void do_sys_settime(void){
	/*
	uint32_t first32 = *(old_stack + 1);
	uint32_t last32 = *(old_stack + 2);
	uint64_t date_ms = ((uint64_t)first32) << 32 | last32;
	set_date_ms(date_ms);
	*/
}


void do_sys_gettime(void){
	/*
	uint64_t date_ms = get_date_ms();
	int first32 = date_ms >> 32;
	int last32 = date_ms;
	(old_stack+0) = first32;
	(old_stack+1) = last32; 
	*/
}

void __attribute__((naked)) swi_handler(void){
	__asm("stmfd sp!, {r0-r12,LR}");
	int * new_stack;
	__asm("mov %0, sp" : "=r"(new_stack));
	int numAppel;
	__asm("mov %0, r0" : "=r"(numAppel));
	if(numAppel==1){
		do_sys_reboot();
	}else if(numAppel==2){
		do_sys_nop();
	}else if(numAppel==3){
		do_sys_settime();
	}else if(numAppel==4){
		do_sys_gettime();
	}else if(numAppel==5){
		do_sys_yieldto(new_stack);
	}else if(numAppel==6){
		do_sys_yield(new_stack);
	}else if(numAppel==7){
		do_sys_exit(new_stack);
	}else{
		PANIC();
	}
	__asm("ldmfd sp!, {r0-r12,PC}^");
}
