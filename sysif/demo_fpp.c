#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "fb.h"
#include "asm_tools.h"

// Help with timers
// ++ until 5000000 = 3s

void user_process1()
{
	drawGreen();
	sys_exit(0);
}

void user_process2()
{
	drawRed();
	sys_exit(0);
}

void user_process3()
{
	drawBlue();
	sys_exit(0);
}

void kmain( void )
{
	FramebufferInitialize();
    sched_init(FPP);
    
    int p = 5;
	for(int i=0;i<5000000;i++) {
		p = (p*p)%27;
	}

    create_fpp_process((func_t*)&user_process1, 3, 3);
	create_fpp_process((func_t*)&user_process2, 1, 1);
	create_fpp_process((func_t*)&user_process3, 1, 1);
	
    __asm("cps 0x10"); // switch CPU to USER mode

    sys_exit(0);
}
