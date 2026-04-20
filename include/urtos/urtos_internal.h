#ifndef __uRTOS_INTERNAL_HEADER__
#define __uRTOS_INTERNAL_HEADER__


#include <stddef.h>
#include <avr/io.h>
#include <urtos/urtos.h>

#define __uRTOS_NULLPTR ((void*)0x0000)
#define __uRTOS_RAM_END ((Pointer_t)(RAMEND))

#define __uRTOS_STATIC_INFO_PTR ((StaticInfo_t*)(RAMEND - sizeof(StaticInfo_t) + 1))

#define __uRTOS_SYSTEM_STACK_START ((void*)(__uRTOS_RAM_END - sizeof(StaticInfo_t)))

#define __uRTOS_SYSTEM_STACK_SIZE 64


#ifdef __cplusplus
extern "C" {
#endif


typedef struct TaskControlBlockStruct
{
	StackPtr_t stackPointer;
	BasePtr_t basePointer;
} TCB_t;

typedef struct TaskControlBlockArrayStruct
{
	size_t nTCBs;
	TCB_t TCBs[];
} TCBArray_t;

typedef struct StaticInfoStruct
{
	TCB_t* current;
	TCB_t* last;

	TCBArray_t* array;

	Scheduler_t scheduler;
} StaticInfo_t;


void uRTOS_Init(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks) __attribute__((noreturn, noinline));


#ifdef __cplusplus
}
#endif

#endif // !__uRTOS_INTERNAL_HEADER__
