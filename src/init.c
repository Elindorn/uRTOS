/**
 * @file init.c
 * @brief Kernel initialization routines.
 * 
 * @author Elindorn
 * @copyright Copyright (c) 2026 Elindorn
 * @licence MIT Licence
 */


#include <urtos/urtos_internal.h>
#include <avr/pgmspace.h>


/**
 * @brief Initialize the hardware timer.
 * 
 * This function is declared weak so that an application can override it
 * with a custom timer setup if needed.
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 */
void __attribute__((weak)) uRTOS_InitTimer(const SysInitInfo_t* initInfo)
{
	uint8_t timer = pgm_read_byte(&initInfo->timerNo);
	uint8_t prescaler = pgm_read_byte(&initInfo->timerPrescaler);

	__uRTOS_STATIC_INFO_PTR->sysFlags &= ~__uRTOS_TIM_MASK;

	switch (timer)
	{
		case 0:
			__uRTOS_TCCRA(0) = 0x00;
			__uRTOS_TCCRB(0) = prescaler & 0b111;
			__uRTOS_TCNT(0) = 0x00;
			__uRTOS_TIMSK(0) = (1 << TOIE0);

			__uRTOS_STATIC_INFO_PTR->sysFlags |= __uRTOS_TIM0;
			break;

		case 1:
			__uRTOS_TCCRA(1) = 0x00;
			__uRTOS_TCCRB(1) = prescaler & 0b111;
			__uRTOS_TCNT_H(1) = 0x00;
			__uRTOS_TCNT_L(1) = 0x00;
			__uRTOS_TIMSK(1) = (1 << TOIE1);

			__uRTOS_STATIC_INFO_PTR->sysFlags |= __uRTOS_TIM1;
			break;

		case 2:
			__uRTOS_TCCRA(2) = 0x00;
			__uRTOS_TCCRB(2) = prescaler & 0b111;
			__uRTOS_TCNT(2) = 0x00;
			__uRTOS_TIMSK(2) = (1 << TOIE2);

			__uRTOS_STATIC_INFO_PTR->sysFlags |= __uRTOS_TIM2;
			break;
	}
}

/**
 * @brief Initialize system data and timer.
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 */
static void uRTOS_InitSystem(const SysInitInfo_t* initInfo)
{
	uRTOS_InitTimer(initInfo);

	__uRTOS_STATIC_INFO_PTR->errorHandler = (ErrorCallback_t)pgm_read_word(&initInfo->errorCallback);

	Scheduler_t sched = (Scheduler_t)pgm_read_word(&initInfo->scheduler);
	if (sched) __uRTOS_STATIC_INFO_PTR->scheduler = sched;
	else uRTOS_InvokeErrorHandler(uRTOS_SCHEDULER_NOT_DEFINED);
}

/**
 * @brief Prepare the initial stack frame for a task.
 * 
 * The stack is populated with a fake interrupt frame: PC, 32 zeroed
 * registers, and SREG with I flag set.
 * 
 * @param tcb Task control block to initialize.
 * @param handle Task function pointer.
 */
static void uRTOS_BootstrapTask(TCB_t* tcb, ProcAddr_t handle)
{
	uint8_t* stack = tcb->basePointer;
	
	// PC
	*stack-- = (uint8_t)((uint16_t)handle & 0xff);
	*stack-- = (uint8_t)((uint16_t)handle >> 8);

	// Clear all registers
	for (uint8_t i = 0; i < 32; i++)
	{
		*stack-- = 0x00;
	}

	// SREG (allow interrupts)
	*stack-- = (1 << SREG_I);
	tcb->stackPointer = stack;
}

/**
 * @brief Launch the first task.
 * 
 * Restores the stack pointer and all registers from the task's stack,
 * then executes `reti` to start the task with interrupts enabled.
 * 
 * @param id Index of the first task to run.
 */
static void __attribute__((noreturn)) uRTOS_LaunchFirstTask(TaskId_t id)
{
	__asm__ __volatile__
	(
		// Restore SP
		"out __SP_L__, %A0"		"\n\t"
		"out __SP_H__, %B0"		"\n\t"

		// Restore SREG
		"pop r16"				"\n\t"
		"out __SREG__, r16"		"\n\t"

		// Clear registers
		"pop r31"	"\n\t"	"pop r30"	"\n\t"
		"pop r29"	"\n\t"	"pop r28"	"\n\t"
		"pop r27"	"\n\t"	"pop r26"	"\n\t"
		"pop r25"	"\n\t"	"pop r24"	"\n\t"
		"pop r23"	"\n\t"	"pop r22"	"\n\t"
		"pop r21"	"\n\t"	"pop r20"	"\n\t"
		"pop r19"	"\n\t"	"pop r18"	"\n\t"
		"pop r17"	"\n\t"	"pop r16"	"\n\t"
		"pop r15"	"\n\t"	"pop r14"	"\n\t"
		"pop r13"	"\n\t"	"pop r12"	"\n\t"
		"pop r11"	"\n\t"	"pop r10"	"\n\t"
		"pop r9"	"\n\t"	"pop r8"	"\n\t"
		"pop r7"	"\n\t"	"pop r6"	"\n\t"
		"pop r5"	"\n\t"	"pop r4"	"\n\t"
		"pop r3"	"\n\t"	"pop r2"	"\n\t"
		"pop r1"	"\n\t"	"pop r0"	"\n\t"

		// Restore PC and allow interrupts
		"reti"					"\n\t"
		:
		: "r" (__uRTOS_STATIC_INFO_PTR->array->TCBs[id].stackPointer)
	);

	__builtin_unreachable();
}

/**
 * @brief Second-stage kernel initializer.
 * 
 * Allocates TCBs and stacks, initializes the stack info block, sets up
 * the timer, and launces the first runnable task.
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 * @param tasks Pointer to array of task descriptors (in PROGMEM).
 * @param nTasks Number of tasks in the array.
 */
void uRTOS_Init(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks)
{
	TCBArray_t* array = __uRTOS_NULLPTR;
	TaskId_t firstRunnable = 0;

	Pointer_t tcbArrayEnd = __uRTOS_RAM_END - __uRTOS_SYSTEM_STACK_SIZE - sizeof(StaticInfo_t);
	Pointer_t stacksEnd = tcbArrayEnd - (nTasks * sizeof(TCB_t)) - sizeof(array->nTCBs);

	array = (TCBArray_t*)(stacksEnd + 1);
	array->nTCBs = nTasks;

	Pointer_t stack = stacksEnd;
	while (nTasks--)
	{
		ProcAddr_t procHandle = (ProcAddr_t)pgm_read_word(&tasks[nTasks].handle);
		size_t stackSize = pgm_read_word(&tasks[nTasks].stackSize);
		TaskFlags_t taskFlags = pgm_read_byte(&tasks[nTasks].flags);

		array->TCBs[nTasks].basePointer = stack;

		array->TCBs[nTasks].flags = taskFlags;
		if (!(taskFlags & uRTOS_TFLAGS_SUSPENDED))
		{
			firstRunnable = nTasks;
		}

		uRTOS_BootstrapTask(&array->TCBs[nTasks], procHandle);
		stack -= stackSize;
	}

	__uRTOS_STATIC_INFO_PTR->current = &array->TCBs[firstRunnable];
	__uRTOS_STATIC_INFO_PTR->last = &array->TCBs[array->nTCBs - 1];
	__uRTOS_STATIC_INFO_PTR->array = array;
	// Scheduler will be set in InitSystem

	uRTOS_InitSystem(initInfo);
	uRTOS_LaunchFirstTask(firstRunnable);

	__builtin_unreachable();
}
