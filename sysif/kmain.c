#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "fb.h"
#include "asm_tools.h"

struct pcb_s * root;

void user_process0()
{
    int v1=5;
    while(1)
    {
		drawFlag0();
        v1++;
        if(v1%10000==0) {
			sys_yield();
		}
    }
}

void user_process1()
{
    int v2=5;
    while(1)
    {
		drawFlag1();
        v2++;
        if(v2%10000==0) {
			sys_yield();
		}
    }
}

void user_process2()
{
    int v3=0;
    while(1)
    {
		drawFlag2();
        v3+=5;
        if(v3%50000==0) {
			sys_yield();
		}
    }
}

void draw_process()
{
	while(1) {
		sys_draw(1);
		drawSched(root);
		sys_draw(0);
	}
}

void kmain( void )
{
	FramebufferInitialize();
	drawBlue();
	
    root = sched_init(COLLABORATIVE);
	
	create_process((func_t*)&user_process0);
    create_process((func_t*)&user_process1);
	create_process((func_t*)&user_process2);
	create_draw_process((func_t*)&draw_process);
	
    __asm("cps 0x10"); // switch CPU to USER mode

    while(1)
    {
        sys_yield();
    }
}
