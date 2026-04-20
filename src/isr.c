#include "urtos/urtos_internal.h"

void __attribute__((used, noinline)) uRTOS_Sched_RoundRobin()
{
	TCB_t* next = __uRTOS_STATIC_INFO_PTR->current;

	next += 1;

	if (next > __uRTOS_STATIC_INFO_PTR->last)
	{
		next = __uRTOS_STATIC_INFO_PTR->array->TCBs;
	}
	
	__uRTOS_STATIC_INFO_PTR->current = next;
}
