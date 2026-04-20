#include <urtos/urtos_internal.h>

static void uRTOS_InitTimer(const SysInitInfo_t* initInfo)
{
	TCCR2A = 0x00;
	TCCR2B = 0b011;
	TCNT2 = 0x00;
	TIMSK2 = 1 << TOIE2;
}

static void uRTOS_InitSystem(const SysInitInfo_t* initInfo)
{
	uRTOS_InitTimer(initInfo);
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

	uRTOS_InitSystem(initInfo);
	uRTOS_LaunchFirstTask(firstRunnable);

	__builtin_unreachable();
}
