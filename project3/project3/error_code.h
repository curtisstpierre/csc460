/**
 * @file   error_code.h
 *
 * @brief Error messages returned in OS_Abort().
 *        Green errors are initialization errors
 *        Red errors are runt time errors
 *
 * CSC 460 Real Time Operating Systems - Mantis Cheng
 *
 * @author Mark Roller
 * @author Curtis St. Pierre
 */
#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

enum {

/** GREEN ERRORS -- Initialize time errors. */

/** PERIODIC name is out of range [1 .. MAXNAME] */
ERR_1_PERIOD_LT_WCET,

/** PERIODIC task assigned name IDLE */
ERR_2_PERIODIC_NAMED_IDLE,

/** PERIODIC set to start before zero */
ERR_3_PERIODIC_START_BEFORE_ZERO,

/** PERIODIC name not in PPP array */
ERR_4_PERIODIC_PERIOD_LT_ONE,

/** Time interval 0 in PPP array */
ERR_5_PERIODIC_TASK_COLLISION,


/** RED ERRORS -- Run time errors. */

/** User called OS_Abort() */
ERR_RUN_1_USER_CALLED_OS_ABORT,

/** Too many tasks created. Only allowed MAXPROCESS at any time.*/
ERR_RUN_2_TOO_MANY_TASKS,

/** PERIODIC task still running at end of time slot. */
ERR_RUN_3_PERIODIC_TOOK_TOO_LONG,

/** ISR made a request that only tasks are allowed. */
ERR_RUN_4_ILLEGAL_ISR_KERNEL_REQUEST,

/** RTOS Internal error in handling request. */
ERR_RUN_5_RTOS_INTERNAL_ERROR,

/** Error when init services reaches the max services. */
ERR_RUN_6_INIT_SERVICE_MAX_ERROR,

/** Error when attempting to subscribe a periodic task. */
ERR_RUN_7_SUBSCRIBE_PERIODIC,

/** Error if a periodic task is found to be queued. */
ERR_RUN_8_SUBSCRIBE_PERIODIC_FOUND,

};


#endif
