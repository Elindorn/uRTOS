#ifndef __uRTOS_HEADER__
#define __uRTOS_HEADER__


#include <stddef.h>
#include <stdint.h>


#define uRTOS_PROC_HANDLE(name) void __attribute__((used, noinline, noreturn)) name(void)
#define uRTOS_SCHEDULER(name) void __attribute__((used, noinline)) name(void)

#define uRTOS_INSTALL(vector) ISR(vector, ISR_NAKED) { __asm__ __volatile__ ("jmp uRTOS_TickHandler" ::); }

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*Scheduler_t)(void);
typedef void (*ProcAddr_t)(void);


typedef void* Pointer_t;
typedef Pointer_t StackPtr_t;
typedef Pointer_t BasePtr_t;

typedef size_t TaskId_t;

typedef struct _SystemInitInfoStruct
{
	uint8_t timerNo;
	uint8_t timerPrescaler;
	Scheduler_t scheduler;
} SysInitInfo_t;

typedef struct _TaskDescriptorStruct
{
	ProcAddr_t handle;
	size_t stackSize;
} TaskDesc_t;


void uRTOS_Run(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks) __attribute__((noreturn));

uRTOS_SCHEDULER(uRTOS_Sched_RoundRobin);


#ifdef __cplusplus
}
#endif

#endif // !__uRTOS_HEADER__
