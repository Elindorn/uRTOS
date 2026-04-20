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


#define __uRTOS_TCCRA(timer)	TCCR ## timer ## A
#define __uRTOS_TCCRB(timer)	TCCR ## timer ## B
#define __uRTOS_TIMSK(timer)	TIMSK ## timer
#define __uRTOS_TIFR(timer)		TIFR ## timer
#define __uRTOS_TCNT(timer)		TCNT ## timer
#define __uRTOS_TCNT_H(timer)	TCNT ## timer ## H
#define __uRTOS_TCNT_L(timer)	TCNT ## timer ## L

#define __uRTOS_TIM_OFFSET	0
#define __uRTOS_TIM_MASK	(0b11 << __uRTOS_TIMER_NO)
#define __uRTOS_TIM0		0b00
#define __uRTOS_TIM1		0b01
#define __uRTOS_TIM2		0b10


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
