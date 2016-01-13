#include "syscall.h"
#include "sched.h"
#include "hw.h"
#include "fb.h"
#include "asm_tools.h"

// Help with timers
// ++ until 5000000 = 3s

void user_process1()
{
	conway1();
	sys_exit(0);
}

void user_process2()
{
	conway2();
	sys_exit(0);
}

void user_process3()
{
	conway3();
	sys_exit(0);
}

void user_process4()
{
	conway4();
	sys_exit(0);
}

void kmain( void )
{
	FramebufferInitialize();
    sched_init(PREEMPTIVE);
    
    int p = 5;
	for(int i=0;i<5000000;i++) {
		p = (p*p)%27;
	}
	
	init_lines();

    create_process((func_t*)&user_process1);
	create_process((func_t*)&user_process2);
	create_process((func_t*)&user_process3);
	create_process((func_t*)&user_process4);
	
    __asm("cps 0x10"); // switch CPU to USER mode

    sys_exit(0);
}
