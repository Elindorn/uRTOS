#include <urtos/urtos_internal.h>

static void uRTOS_InitTimer(const SysInitInfo_t* initInfo)
{
	uint8_t timer = pgm_read_byte(&initInfo->timerNo);
	uint8_t prescaler = pgm_read_byte(&initInfo->timerPrescaler);

	switch (timer)
	{
		case 0:
			__uRTOS_TCCRA(0) = 0x00;
			__uRTOS_TCCRB(0) = prescaler & 0b111;
			__uRTOS_TCNT(0) = 0x00;
			__uRTOS_TIMSK(0) = (1 << TOIE0);
			break;

		case 1:
			__uRTOS_TCCRA(1) = 0x00;
			__uRTOS_TCCRB(1) = prescaler & 0b111;
			__uRTOS_TCNT_H(1) = 0x00;
			__uRTOS_TCNT_L(1) = 0x00;
			__uRTOS_TIMSK(1) = (1 << TOIE1);
			break;

		case 2:
			__uRTOS_TCCRA(2) = 0x00;
			__uRTOS_TCCRB(2) = prescaler & 0b111;
			__uRTOS_TCNT(2) = 0x00;
			__uRTOS_TIMSK(2) = (1 << TOIE2);
			break;
	}
}

static void uRTOS_InitSystem(const SysInitInfo_t* initInfo)
{
	uRTOS_InitTimer(initInfo);

	__uRTOS_STATIC_INFO_PTR->scheduler = uRTOS_Sched_RoundRobin;
}

static void uRTOS_BootstrapTask(TCB_t* tcb, ProcAddr_t handle)
{
	uint8_t* stack = tcb->basePointer;

	// Stack registers init

	tcb->stackPointer = stack - 35; // 35 = r0..r31 (32 bytes) + SREG (1 byte) + PC (2 bytes)
}

static void __attribute__((noreturn)) uRTOS_LaunchFirstTask(TaskId_t id)
{
	while (1);
}

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
		volatile ProcAddr_t procHandle = (ProcAddr_t)pgm_read_word(&tasks[nTasks].handle);
		volatile size_t stackSize = pgm_read_word(&tasks[nTasks].stackSize);

		array->TCBs[nTasks].basePointer = stack;

		if (1 /* Task flags */)
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
