/**
 * @file urtos.c
 * @brief Public API implementation for uRTOS.
 * 
 * @author Elindorn
 * @copyright Copyright (c) 2026 Elindorn
 * @licence MIT Licence
 */


#include <urtos/urtos_internal.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>


/**
 * @brief Start the uRTOS kernel.
 * 
 * Disables interrupts, switches to the system stack, and calls the second
 * stage initializer.
 * 
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 * @param tasks Pointer to array of task descriptors (in PROGMEM).
 * @param nTasks Number of tasks in the array.
 */
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

/**
 * @brief Voluntarily yield the processor.
 * 
 * Resets the timer counter and sets the overflow flag, then re-enables
 * interrupts. The pending interrupt will fire immediately and invoke the
 * scheduler.
 */
void uRTOS_Yield()
{
	uint8_t sreg = SREG;
	cli();

	switch (__uRTOS_STATIC_INFO_PTR->sysFlags & __uRTOS_TIM_MASK)
	{
		case __uRTOS_TIM0:
			__uRTOS_TCNT(0) = 0x00;
			__uRTOS_TIFR(0) |= 1 << TOV0;
			break;
	
		case __uRTOS_TIM1:
			__uRTOS_TCNT_L(1) = 0x00;
			__uRTOS_TCNT_H(1) = 0x00;
			__uRTOS_TIFR(1) |= 1 << TOV1;
			break;
	
		case __uRTOS_TIM2:
			__uRTOS_TCNT(2) = 0x00;
			__uRTOS_TIFR(2) |= 1 << TOV2;
			break;
	}

	// Restore SREG (and interrupt flag)
	SREG = sreg;
}

/**
 * @brief Suspend a task by its ID.
 * 
 * @param id Task identifier (0..nTasks-1).
 */
void uRTOS_DisableTask(TaskId_t id)
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->array->TCBs[id].flags |= uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;
}

/**
 * @brief Suspend the calling task.
 */
void uRTOS_DisableCurrentTask()
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->current->flags |= uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;

	uRTOS_Yield();
}

/**
 * @brief Resume a suspended task by its ID.
 * 
 * @param id Task identifier (0..nTasks-1).
 */
void uRTOS_EnableTask(TaskId_t id)
{
	uint8_t sreg = SREG;
	cli();

	__uRTOS_STATIC_INFO_PTR->array->TCBs[id].flags &= ~uRTOS_TFLAGS_SUSPENDED;

	SREG = sreg;
}

/**
 * @brief Get the current system tick count.
 * 
 * @return Number of ticks since the kernel started.
 */
unsigned long uRTOS_GetTick()
{
	return __uRTOS_STATIC_INFO_PTR->tick;
}

void __uRTOS_InvokeErrorHandler(Errno_t error)
{
	if (__uRTOS_STATIC_INFO_PTR->errorHandler)
		__uRTOS_STATIC_INFO_PTR->errorHandler(error);

	while (1)
		_NOP();
}
