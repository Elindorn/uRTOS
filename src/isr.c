#include "urtos/urtos_internal.h"

void __attribute__((used, noinline)) uRTOS_Sched_RoundRobin()
{
	TCB_t* next = __uRTOS_STATIC_INFO_PTR->current;

	do
	{
		next += 1;

		if (next > __uRTOS_STATIC_INFO_PTR->last)
		{
			next = __uRTOS_STATIC_INFO_PTR->array->TCBs;
		}
	} while (next->flags != 0);
	
	__uRTOS_STATIC_INFO_PTR->current = next;
}

void __attribute__((naked, used, noinline)) uRTOS_TickHandler()
{
	// Save context
	__asm__ __volatile__
	(
		"push r0"				"\n\t"
		"push r1"				"\n\t"
		"push r2"				"\n\t"
		"push r3"				"\n\t"
		"push r4"				"\n\t"
		"push r5"				"\n\t"
		"push r6"				"\n\t"
		"push r7"				"\n\t"
		"push r8"				"\n\t"
		"push r9"				"\n\t"

		"push r10"				"\n\t"
		"push r11"				"\n\t"
		"push r12"				"\n\t"
		"push r13"				"\n\t"
		"push r14"				"\n\t"
		"push r15"				"\n\t"
		"push r16"				"\n\t"
		"push r17"				"\n\t"
		"push r18"				"\n\t"
		"push r19"				"\n\t"

		"push r20"				"\n\t"
		"push r21"				"\n\t"
		"push r22"				"\n\t"
		"push r23"				"\n\t"
		"push r24"				"\n\t"
		"push r25"				"\n\t"
		"push r26"				"\n\t"
		"push r27"				"\n\t"
		"push r28"				"\n\t"
		"push r29"				"\n\t"

		"push r30"				"\n\t"
		"push r31"				"\n\t"

		"in r0, __SREG__"		"\n\t"
		"push r0"				"\n\t"

		"clr r1"				"\n\t"
		:
		:
		: "memory"
	);

	// Save SP to TCB
	__asm__ __volatile__
	(
		"in r30, __SP_L__"		"\n\t"
		"in r31, __SP_H__"		"\n\t"
		"st %a0, r30"			"\n\t"
		"std %a0+1, r31"		"\n\t"
		:
		: "b" (&(__uRTOS_STATIC_INFO_PTR->current->stackPointer))
		: "r30", "r31", "memory"
	);

	// Load system stack
	__asm__ __volatile__
	(
		"ldi r30, lo8(%0)"		"\n\t"
		"ldi r31, hi8(%0)"		"\n\t"
		"out __SP_L__, r30"		"\n\t"
		"out __SP_H__, r31"		"\n\t"
		:
		: "i" (__uRTOS_SYSTEM_STACK_START)
		: "r30", "r31"
	);

	// Call scheduler

	// Restore the SP of a new task
	// (current TCB in static info already updated)
	__asm__ __volatile__
	(
		"ld r30, %a0"			"\n\t"
		"ldd r31, %a0+1"		"\n\t"
		"out __SP_L__, r30"		"\n\t"
		"out __SP_H__, r31"		"\n\t"
		:
		: "b" (&(__uRTOS_STATIC_INFO_PTR->current->stackPointer))
		: "r30", "r31"
	);

		// Restore context
	__asm__ __volatile__
	(
		"pop r0"				"\n\t"
		"out __SREG__, r0"		"\n\t"

		"pop r31"				"\n\t"
		"pop r30"				"\n\t"

		"pop r29"				"\n\t"
		"pop r28"				"\n\t"
		"pop r27"				"\n\t"
		"pop r26"				"\n\t"
		"pop r25"				"\n\t"
		"pop r24"				"\n\t"
		"pop r23"				"\n\t"
		"pop r22"				"\n\t"
		"pop r21"				"\n\t"
		"pop r20"				"\n\t"

		"pop r19"				"\n\t"
		"pop r18"				"\n\t"
		"pop r17"				"\n\t"
		"pop r16"				"\n\t"
		"pop r15"				"\n\t"
		"pop r14"				"\n\t"
		"pop r13"				"\n\t"
		"pop r12"				"\n\t"
		"pop r11"				"\n\t"
		"pop r10"				"\n\t"

		"pop r9"				"\n\t"
		"pop r8"				"\n\t"
		"pop r7"				"\n\t"
		"pop r6"				"\n\t"
		"pop r5"				"\n\t"
		"pop r4"				"\n\t"
		"pop r3"				"\n\t"
		"pop r2"				"\n\t"
		"pop r1"				"\n\t"
		"pop r0"				"\n\t"

		"reti"					"\n\t"
	);

	__builtin_unreachable();
}
