/**
 * @file   kernel.h
 *
 * @brief kernel data structures used in os.c.
 *
 * CSC 460 Real Time Operating Systems - Mantis Cheng
 *
 * @author Mark Roller
 * @author Curtis St. Pierre
 */
 
#ifndef __KERNEL_H__
#define __KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include "os.h"

/** Disable default prescaler to make processor speed 8 MHz. */

#define Disable_Interrupt()     asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

/** The maximum number of names. Currently the same as the number of tasks. */
#define 	MAXNAME		MAXPROCESS

/** The RTOS timer's prescaler divisor */
#define TIMER_PRESCALER 8

/** The number of clock cycles in one "tick" or 5 ms */
#define TICK_CYCLES     (((F_CPU / TIMER_PRESCALER) / 1000) * TICK)

/** LEDs for OS_Abort() */
#define LED_RED_MASK    (uint8_t)(_BV(PB5))

/** LEDs for OS_Abort() */
#define LED_GREEN_MASK    (uint8_t)(_BV(PB6))


/* Typedefs and data structures. */

typedef void (*voidfuncvoid_ptr) (void);      /* pointer to void f(void) */

/**
 * @brief This is the set of states that a task can be in at any given time.
 */
typedef enum
{
    DEAD = 0,
    RUNNING,
    READY,
    WAITING
}
task_state_t;

/**
 * @brief This is the set of kernel requests, i.e., a request code for each system call.
 */
typedef enum
{
    NONE = 0,
    TIMER_EXPIRED,
    TASK_CREATE,
    TASK_TERMINATE,
    TASK_NEXT,
    TASK_GET_ARG,
    TASK_INTERRUPT,
}
kernel_request_t;


/**
 * @brief The arguments required to create a task.
 */
typedef struct
{
    /** The code the new task is to run.*/
    voidfuncvoid_ptr f;
    /** A new task may be created with an argument that it can retrieve later. */
    int arg;
    /** Priority of the new task: RR, PERIODIC, SYSTEM */
    uint8_t level;
    /** PERIODIC tasks need a period*/
    uint16_t period;
    /*PERIODIC tasks need a worst case run time. */
    uint16_t wcet;
    /*PERIODIC tasks need a start time*/
    int16_t start;
}
create_args_t;


typedef struct td_struct task_descriptor_t;
/**
 * @brief All the data needed to describe the task, including its context.
 */
struct td_struct
{
    /** The stack used by the task. SP points in here when task is RUNNING. */
    uint8_t                         stack[MAXSTACK];
    /** A variable to save the hardware SP into when the task is suspended. */
    uint8_t*               volatile sp;   /* stack pointer into the "workSpace" */
    /** PERIODIC tasks need a period*/
    uint16_t period;
	/*PERIODIC tasks need a worst case run time. */
    uint16_t wcet;
	/*PERIODIC tasks need a start time*/
    int16_t start;
	/*PERIODIC tasks need to keep track of ticks until next run*/
	int16_t ticks;
    /** The state of the task in this descriptor. */
    task_state_t                    state;
    /** The argument passed to Task_Create for this task. */
    int                             arg;
    /** The priority (type) of this task. */
    uint8_t                         level;
    /** 
	 * A link to the previous task descriptor in the queue holding this task.
	 * This is necessary for removing a periodic task from anywhere in the list without
	 * causing issues with the list
	 */
    task_descriptor_t*              previous;
    /** A link to the next task descriptor in the queue holding this task. */
    task_descriptor_t*              next;
    /** Value passed in by the publish/subscribe functions */
    int16_t*                        value;
};


/**
 * @brief Contains pointers to head and tail of a linked list.
 */
typedef struct
{
    /** The first item in the queue. NULL if the queue is empty. */
    task_descriptor_t*  head;
    /** The last item in the queue. Undefined if the queue is empty. */
    task_descriptor_t*  tail;
}
queue_t;

#ifdef __cplusplus
}
#endif

#endif

