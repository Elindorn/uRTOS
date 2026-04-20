#include <urtos/urtos_internal.h>

void uRTOS_Init(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks)
{
	Pointer_t tcbArrayEnd = __uRTOS_RAM_END - __uRTOS_SYSTEM_STACK_SIZE - sizeof(StaticInfo_t);
	Pointer_t stacksEnd = tcbArrayEnd - (nTasks * sizeof(uint32_t /* TCB */));

	Pointer_t stack = stacksEnd;
	while (nTasks--)
	{
		volatile ProcAddr_t procHandle = (ProcAddr_t)pgm_read_word(&tasks[nTasks].handle);
		volatile size_t stackSize = pgm_read_word(&tasks[nTasks].stackSize);
	}
}
