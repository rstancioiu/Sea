#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "asm_tools.h"

#define NB_PROCESS 5

void user_process1()
{
    int v1=5;
    while(1)
    {
        v1++;
    }
}

void user_process2()
{
    int v2=-12;
    while(1)
    {
        v2-=2;
    }
}

void user_process3()
{
    int v3=0;
    while(1)
    {
        v3+=5;
    }
}

void kmain( void )
{
    sched_init();

    create_process((func_t*)&user_process1);
	create_process((func_t*)&user_process2);
	create_process((func_t*)&user_process3);
	
	//Interruption related
	ENABLE_IRQ();
	timer_init();
	
    __asm("cps 0x10"); // switch CPU to USER mode
    // ******************************************

    while(1)
    {
        sys_yield();
    }
}
