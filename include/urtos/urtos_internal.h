/**
 * @file urtos_internal.h
 * @brief Internal definitions for uRTOS.
 * 
 * This header is **not** part of the public API. It contains hardware-specific
 * macros and stuctures used by the kernel implementation. Applications
 * should not include this file directly.
 * 
 * @author Elindorn
 * @copyright Copyright (c) 2026 Elindorn
 * @licence MIT Licence
 */


#ifndef __uRTOS_INTERNAL_HEADER__
#define __uRTOS_INTERNAL_HEADER__


#include <stddef.h>
#include <avr/io.h>
#include <urtos/urtos.h>


/**
 * @defgroup mem_layout Memory layout constants
 * @{
 */
/** Null pointers representations. */
#define __uRTOS_NULLPTR ((void*)0x0000)
/** Top of RAM. */
#define __uRTOS_RAM_END ((Pointer_t)(RAMEND))
/** Address of the StaticInfo_t structure (placed at the very top of RAM). */
#define __uRTOS_STATIC_INFO_PTR ((StaticInfo_t*)(RAMEND - sizeof(StaticInfo_t) + 1))
/** Starting address of the system stack (grows downward). */
#define __uRTOS_SYSTEM_STACK_START ((void*)(__uRTOS_RAM_END - sizeof(StaticInfo_t)))
/** Size of the system stack in bytes. */
#define __uRTOS_SYSTEM_STACK_SIZE 64
/** @} */ // end of mem_layout


/**
 * @defgroup tim_regs Timer register access macros
 * @{
 */
#define __uRTOS_TCCRA(timer)	TCCR ## timer ## A		/**< Timer/Counter Control Register A */
#define __uRTOS_TCCRB(timer)	TCCR ## timer ## B		/**< Timer/Counter Control Register B */
#define __uRTOS_TIMSK(timer)	TIMSK ## timer			/**< Timer Interrupt Mask Register */
#define __uRTOS_TIFR(timer)		TIFR ## timer			/**< Timer Interrupt Flag Register */
#define __uRTOS_TCNT(timer)		TCNT ## timer			/**< Timer Counter (8-bit) */
#define __uRTOS_TCNT_H(timer)	TCNT ## timer ## H		/**< Timer Counter High byte (16-bit) */
#define __uRTOS_TCNT_L(timer)	TCNT ## timer ## L		/**< Timer Counter Low byte (16-bit) */
/** @} */ // end of tim_regs

/**
 * @defgroup tim_id_flags Timer identifications bits (stored in sysFlags)
 * @{
 */
#define __uRTOS_TIM_OFFSET	0		/**< Bit offset of timer ID in sysFlags. */
#define __uRTOS_TIM_MASK	(0b11 << __uRTOS_TIM_OFFSET) /**< Mask for timer ID bits. */
#define __uRTOS_TIM0		0b00	/**< Timer 0 selected. */
#define __uRTOS_TIM1		0b01	/**< Timer 1 selected. */
#define __uRTOS_TIM2		0b10	/**< Timer 2 selected. */
/** @} */ // end of tim_id_flags


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Task Control Block (TCB).
 * 
 * Holds the saved stack pointer, stack base, and status flags for one task.
 */
typedef struct _TaskControlBlockStruct
{
	StackPtr_t stackPointer;	/**< Saved SP when task is not running. */
	BasePtr_t basePointer;		/**< Base of the task's stack. */
	TaskFlags_t flags;			/**< Task flags. */
} TCB_t;

/**
 * @brief Array of TCBs placed immediately before task stacks.
 */
typedef struct _TaskControlBlockArrayStruct
{
	size_t nTCBs;	/**< Number of valid TCB entries. */
	TCB_t TCBs[];	/**< Flexible array of TCBs. */
} TCBArray_t;

/**
 * @brief Static information block located at the top of RAM.
 * 
 * This structure is accessed via `__uRTOS_STATIC_INFO_PTR`. It contains
 * kernel state that must survive across context switches.
 */
typedef struct _StaticInfoStruct
{
	TCB_t* current;			/**< Currently executing task. */
	TCB_t* last;			/**< Last TCB in the array (cached, for boundary check). */

	TCBArray_t* array;		/**< Pointer to the TCB array. */

	unsigned long tick;		/**< System tick counter. */

	Scheduler_t scheduler;	/**< Current scheduler function. */
	ErrorCallback_t errorHandler;	/**< Error handler function. */

	uint8_t sysFlags;		/**< System flags (timer ID, etc.). */
} StaticInfo_t;

/**
 * @brief Second stage of system initialization.
 * 
 * Called after switching to the system stack. This function never returns.
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 * @param tasks Pointer to array of task descriptors (in PROGMEM).
 * @param nTasks Number of tasks in the array.
 */
void uRTOS_Init(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks) __attribute__((noreturn, noinline));

/**
 * @brief Timer tick handler (naked).
 * 
 * This function is jumped to from the user-installed ISR. It saves the
 * current task context, invokes the scheduler on the system stack, and
 * restores the context of the next task.
 */
void uRTOS_TickHandler(void);

/**
 * @brief Error handler wrapper.
 * 
 * This wrapper checks callback is not a nullpointer and halt the system forever
 * after callback completion.
 */
void __uRTOS_InvokeErrorHandler(Errno_t error);


#ifdef __cplusplus
}
#endif

#endif // !__uRTOS_INTERNAL_HEADER__
