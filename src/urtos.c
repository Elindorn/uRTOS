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

void uRTOS_Yield()
{
	uint8_t sreg = SREG;
	cli();

	switch (__uRTOS_STATIC_INFO_PTR->sysFlags & __uRTOS_TIM_MASK)
	{
		case __uRTOS_TIM0:
			__uRTOS_TCNT(0) = 0xff;
			break;
	
		case __uRTOS_TIM1:
			__uRTOS_TCNT_L(1) = 0xff;
			__uRTOS_TCNT_H(1) = 0xff;
			break;
	
		case __uRTOS_TIM2:
			__uRTOS_TCNT(2) = 0xff;
			break;
	}

	// Restore SREG (and interrupt flag)
	SREG = sreg;
}

void uRTOS_DisableTask(TaskId_t id)
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->array->TCBs[id].flags |= uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;
}

void uRTOS_DisableCurrentTask()
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->current->flags |= uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;

	uRTOS_Yield();
}

void uRTOS_EnableTask(TaskId_t id)
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->array->TCBs[id].flags &= ~uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;
}

void uRTOS_GetTick()
{
	return __uRTOS_STATIC_INFO_PTR->tick;
}
