/**
 * @file urtos.h
 * @brief Public API for uRTOS - a lightweight real-time operating system for AVR.
 * 
 * uRTOS provides a preemptive/cooperative hybrid scheduler with static task
 * definitions stored in PROGMEM. It designed for simplicity and minimal
 * overhead on ATmega328 and similar AVR MCUs.
 * 
 * @author Elindorn
 * @copyright Copyright (c) 2026 Elindorn
 * @licence MIT Licence
 */


#ifndef __uRTOS_HEADER__
#define __uRTOS_HEADER__


#include <stddef.h>
#include <stdint.h>


/**
 * @defgroup public_api Public API
 * @brief Macros, types and functions available to the application.
 * @{
 */

/**
 * @def uRTOS_PROC_HANDLE(name)
 * @brief Declare a task function.
 * 
 * Task function must never return. The macro adds the necessary attributes
 * to prevent compiler optimizations that could break context switching.
 * 
 * @param name Task function name. 
 */
#define uRTOS_PROC_HANDLE(name) void __attribute__((used, noinline, noreturn)) name(void)

/**
 * @def uRTOS_SCHEDULER(name)
 * @brief Declare a custom scheduler function.
 * 
 * Scheduler functions are called from the tick handler on the system stack.
 * They must be declared with some attributes to ensure correct code generation.
 * 
 * @param name Scheduler function name.
 */
#define uRTOS_SCHEDULER(name) void __attribute__((used, noinline)) name(void)

/**
 * @def uRTOS_INSTALL(vector)
 * @brief Install the uRTOS tick for a given timer overflow interrupt.
 * 
 * This macro creates ISR that jumps to `uRTOS_TickHandler`.
 * It must be placed in the application code.
 * 
 * @param vector AVR interrupt vector name (e.g., TIMER2_OVF_vect).
 */
#define uRTOS_INSTALL(vector) ISR(vector, ISR_NAKED) { __asm__ __volatile__ ("jmp uRTOS_TickHandler" ::); }

/**
 * @defgroup task_flags Task flags
 * @{
 */
#define uRTOS_TFLAGS_NONE		0 /**< No special flags. */
#define uRTOS_TFLAGS_SUSPENDED	1 /**< Task is suspended and will not be scheduled. */
/** @} */ // end of task_flags

/**
 * @defgroup timer_presc Timer prescaler values
 * 
 * Tick frequency is calculated by `CPU_clk/TIM_prescaler/TIM_bitlength`.
 * See your MCU's datasheet.
 * 
 * @{
 */
#define uRTOS_TIM_PRESC_1		0b001 /**< clk/1 */
#define uRTOS_TIM_PRESC_8		0b010 /**< clk/8 */
#define uRTOS_TIM_PRESC_64		0b011 /**< clk/64 */
#define uRTOS_TIM_PRESC_256		0b100 /**< clk/256 */
#define uRTOS_TIM_PRESC_1024	0b101 /**< clk/1024 */
/** @} */ // end of timer_presc

/** @} */ // end of public_api


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Error number type for the error callback.
 */
typedef uint32_t Errno_t;

/**
 * @brief Function pointer type for a scheduler.
 * 
 * Scheduler functions are called from the tick handler on the system stack.
 * They must update `__uRTOS_STATIC_INFO_PTR->current` to point to the next
 * runnable TCB.
 */
typedef void (*Scheduler_t)(void);

/**
 * @brief Function pointer type for a task procedure. 
 */
typedef void (*ProcAddr_t)(void);

/**
 * @brief Function pointer type for error callback
 */
typedef void (*ErrorCallback_t)(Errno_t);

/**
 * @brief Generic pointer type
 */
typedef void* Pointer_t;

/**
 * @brief Pointer to the saved stack pointer of a task.
 */
typedef Pointer_t StackPtr_t;

/**
 * @brief Pointer to the base of a task's stack.
 */
typedef Pointer_t BasePtr_t;

/**
 * @brief Task flags bitfield.
 */
typedef uint8_t TaskFlags_t;

/**
 * @brief Task identifier (index in the task array).
 */
typedef size_t TaskId_t;

/**
 * @brief System initialization information (should be stored in PROGMEM).
 * 
 * Passed to uRTOS_Run() to configure the system.
 */
typedef struct _SystemInitInfoStruct
{
	uint8_t timerNo;				/**< Timer number (0, 1, or 2). */
	uint8_t timerPrescaler;			/**< Timer prescaler (use uRTOS_TIM_PRESC_*). */
	Scheduler_t scheduler;			/**< Pointer to the scheduler function. */
	ErrorCallback_t errorCallback;	/**< Pointer to error handler function. */
} SysInitInfo_t;

/**
 * @brief Task descriptor (should be stored in PROGMEM).
 * 
 * Defines a task's entry point, stack size, and initial flags.
 */
typedef struct _TaskDescriptorStruct
{
	ProcAddr_t handle;
	size_t stackSize;
	TaskFlags_t flags;
} TaskDesc_t;

/**
 * @brief Start the uRTOS kernel.
 * 
 * This function will never return. It switches to the system stack, initializes
 * internal data structures, and launches the first runnable task.
 * 
 * @param initInfo Pointer to system initialization info (in PROGMEM).
 * @param tasks Pointer to array of task descriptors (in PROGMEM).
 * @param nTasks Number of tasks in the array.
 */
void uRTOS_Run(const SysInitInfo_t* initInfo, const TaskDesc_t* tasks, size_t nTasks) __attribute__((noreturn));

/**
 * @brief Yield the processor to the next runnable task.
 * 
 * Can be called from a task to voluntarily give up the processor. It forces
 * an immediate timer overflow interrupt, which triggers the scheduler.
 */
void uRTOS_Yield(void);

/**
 * @brief Suspend a task by its ID.
 * 
 * The suspended task will not be scheduled until re-enabled.
 * 
 * @param id Task identifier (0..nTasks-1).
 */
void uRTOS_DisableTask(TaskId_t id);

/**
 * @brief Suspend the calling task.
 * 
 * The current task is suspended and the scheduler is invoked immediately.
 * The function does not return until the task is re-enabled and scheduled again.
 */
void uRTOS_DisableCurrentTask(void);

/**
 * @brief Resume a suspended task by its ID.
 * 
 * @param id Task identifier (0..nTasks-1).
 */
void uRTOS_EnableTask(TaskId_t id);

/**
 * @brief Get the current system tick count.
 * 
 * The tick counter is incremented in the timer ISR. This counter overflows
 * after 50.9 days since kernal start (F_CPU=16MHz, PRESC=256, 8-bit timer).
 * 
 * @return Number of ticks since the kernel started.
 */
unsigned long uRTOS_GetTick();

/**
 * @brief Default round-robin scheduler.
 * 
 * This scheduler selects the next task in a circular manner, skipping any
 * tasks with non-zero flags (e.g., suspended). It will hangs forever if all
 * tasks is suspended.
 */
uRTOS_SCHEDULER(uRTOS_Sched_RoundRobin);

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

#endif // !__uRTOS_HEADER__
