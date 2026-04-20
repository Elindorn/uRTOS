#ifndef __uRTOS_HEADER__
#define __uRTOS_HEADER__


#include <stddef.h>
#include <stdint.h>


#define uRTOS_PROC_HANDLE(name) void __attribute__((used, noinline, noreturn)) name(void)
#define uRTOS_SCHEDULER(name) void __attribute__((used, noinline)) name(void)

#define uRTOS_INSTALL(vector) ISR(vector, ISR_NAKED) { __asm__ __volatile__ ("jmp uRTOS_TickHandler" ::); }

#define uRTOS_TFLAGS_NONE		0
#define uRTOS_TFLAGS_SUSPENDED	1

#define uRTOS_TIM_PRESC_1		0b001
#define uRTOS_TIM_PRESC_8		0b010
#define uRTOS_TIM_PRESC_64		0b011
#define uRTOS_TIM_PRESC_256		0b100
#define uRTOS_TIM_PRESC_1024	0b101


#ifdef __cplusplus
extern "C" {
#endif


typedef void (*Scheduler_t)(void);
typedef void (*ProcAddr_t)(void);


typedef void* Pointer_t;
typedef Pointer_t StackPtr_t;
typedef Pointer_t BasePtr_t;

typedef uint8_t TaskFlags_t;

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
	TaskFlags_t flags;
} TaskDesc_t;


void uRTOS_Run(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks) __attribute__((noreturn));

void uRTOS_Yield(void);

void uRTOS_DisableTask(TaskId_t id);
void uRTOS_DisableCurrentTask(void);
void uRTOS_EnableTask(TaskId_t id);

unsigned long uRTOS_GetTick();

uRTOS_SCHEDULER(uRTOS_Sched_RoundRobin);


#ifdef __cplusplus
}
#endif

#endif // !__uRTOS_HEADER__
