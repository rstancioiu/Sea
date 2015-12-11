#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "fb.h"
#include "asm_tools.h"

void user_process1()
{
    int v1=5;
    while(1)
    {
        v1++;
        if(v1%50000==0) {
			drawRed();
			sys_yield();
		}
    }
}

void user_process2()
{
    int v2=-12;
    while(1)
    {
        v2-=2;
        sys_yield();
    }
}

void user_process3()
{
    int v3=0;
    while(1)
    {
        v3+=5;
        if(v3%250000==0) {
			drawBlue();
			sys_yield();
		}
    }
}

void kmain( void )
{
	FramebufferInitialize();
	drawBlue();
	
    sched_init(COLLABORATIVE);

    create_process((func_t*)&user_process1);
	create_process((func_t*)&user_process2);
	create_process((func_t*)&user_process3);
	
    __asm("cps 0x10"); // switch CPU to USER mode

    while(1)
    {
        sys_yield();
    }
}
