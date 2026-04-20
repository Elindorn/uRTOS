#include <urtos/urtos_internal.h>

void uRTOS_Run(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks)
{
	cli();

	// Save arguments to registers
	register const SysInitInfo_t* arg0 asm("r24") = initInfo;
	register const TaskDesc_t* arg1 asm("r22") = tasks;
	register size_t arg2 asm("r20") = nTasks;

	// Rebind stack to system one
	__asm__ __volatile__
	(
		"out __SP_L__, %A0"		"\n\t"
		"out __SP_H__, %B0"		"\n\t"
		:
		: "r" ((uint16_t)__uRTOS_SYSTEM_STACK_START)
		: "memory"
	);

	uRTOS_Init(arg0, arg1, arg2);

	__builtin_unreachable();
}
