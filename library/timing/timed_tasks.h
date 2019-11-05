/*
 * This library can be used to schedule periodical tasks.
 *
 * The main concept are task objects (structs) that you
 * register (add). When you want to start handling a given
 * task, activate it. The task will then be executed
 * periodically according to the task's interval.
 *
 * There are two flags of a task you can use to modify
 * the behavior:
 * 		- UC_TT_FLAG_BIT_EXECUTE_WITHOUT_DELAY:
 * 			-> If this bit is set, the task will be
 * 			   executed the first time, when it is added.
 * 			   Otherwise the task will be executed when
 * 			   a time span larger than equal the interval
 * 			   has passed.
 * 		- UC_TT_FLAG_BIT_EXECUTE_ONLY_ONCE:
 * 			-> If this bit is set, the task will be
 * 			   executed only once. After execution the
 * 			   task is in deactivated state.
 *
 * 			   Pro tip: in reality, the active flag of
 * 			   the task will be unset by the "runtime"
 * 			   before the actual function execution.
 * 			   So if you want to, the can activate
 * 			   itself within the function body again
 * 			   (in case you have access to the pointer).
 *
 * 			   This can be used to create periodical
 * 			   tasks which adapt there interval dynamically
 * 			   on their own!
 *
 * The only thing you have to take care of:
 * 		- Call uc_tt_init() at the beginning.
 * 		- Add your tasks and activate them.
 * 		- Call uc_tt_update() periodically.
 * 			-> This interval determines the lowest possible
 * 			   interval this library can approximately
 * 			   guarantee!
 *
 *		- Ah, one more very important thing:
 *			-> The task buffer can hold by default 8 tasks.
 *			   If you need more tasks, define the macro
 *			   UC_TT_TASK_BUFFER and assign an integer number
 *			   representing your desired buffer size before
 *			   including this header file!
 *
 * Have fun!
 *
 *
 *
 *
 * This is free software:
 *
 * 		- You are allowed to execute the code how
 * 		  you want to.
 * 		- This code is open source, you are allowed
 * 		  to inspect what the code does.
 * 		- You are allowed to share/redistribute the
 * 		  code on your own to make it accessible to
 * 		  others or for any reason.
 * 		- You are allowed to modify the code and
 * 		  improve it. Hopefully you share your
 * 		  modifications to make it available to all.
 *
 * Check out my website:	https://hedgehogs-mind.com
 * My github account:	  	https://github.com/hedgehogs-mind
 * Contact me:				peter@hedgehogs-mind.com
 *
 *
 * Yours sincerely,
 *
 * Peter Kuhmann
 */


#ifndef UC_TIMING_TIMED_TASKS_H_
#define UC_TIMING_TIMED_TASKS_H_

#define USE_DUMMY_TIME_API

#ifndef API_GET_TIME_US
	#ifndef USE_DUMMY_TIME_API
		#error Timed tasks library needs API_GET_TIME_US macro to work. Define it or include time.h before this header file.
	#else
		#define API_GET_TIME_US 0
	#endif
#endif

#ifndef UC_TT_TASK_BUFFER
#define UC_TT_TASK_BUFFER 8
#endif

#define UC_TT_FLAG_BIT_ACTIVE_STATUS		 	0
#define UC_TT_FLAG_BIT_EXECUTE_WITHOUT_DELAY 	1
#define UC_TT_FLAG_BIT_EXECUTE_ONLY_ONCE	 	2

struct tt_struct {
	uint8_t flags;
	uint32_t interval_us;
	uint64_t last_executed_us;
	void (*function)();
};

typedef struct tt_struct timed_task;
timed_task *uc_tt_tasks[UC_TT_TASK_BUFFER];

/**
 * Initializes "runtime".
 */
void uc_tt_init() {
	for ( uint8_t i = 0; i < UC_TT_TASK_BUFFER; i++ ) {
		uc_tt_tasks[i] = 0;
	}
}

/*
 * Adds a task to the internal buffer to be handled
 * upon the next update call (if it has been activated
 * until then).
 *
 * This method does not activate the task!
 *
 * Parameters:
 * 		- task: pointer to timed_task struct to add.
 */
void uc_tt_add_task(timed_task *task) {
	for ( uint8_t i = 0; i < UC_TT_TASK_BUFFER; i++ ) {
		if ( uc_tt_tasks[i] == 0 ) {
			uc_tt_tasks[i] = task;
			break;
		}
	}
}

/**
 * Deactivates a given task (sets the active flag to zero).
 *
 * This method does not remove the task from the internal task list!
 *
 * Parameters:
 * 		- task: pointer to task struct to deactivate.
 */
void uc_tt_deactivate_task(timed_task *task) {
	if ( task->flags & (1 << UC_TT_FLAG_BIT_ACTIVE_STATUS) ) {
		task->flags &= ~(1 << UC_TT_FLAG_BIT_ACTIVE_STATUS);
	}
}

/*
 * Removes task from internal buffer and deactivates the task.
 *
 * Parameters:
 * 		- task: pointer to timed_task struct to remove.
 */
void uc_tt_remove_task(timed_task *task) {
	for ( uint8_t i = 0; i < UC_TT_TASK_BUFFER; i++ ) {
		if ( uc_tt_tasks[i] == task ) {
			uc_tt_deactivate_task(task);

			uc_tt_tasks[i] = 0;
			break;
		}
	}
}

/*
 * Calls the function associated with this task and
 * updates the last executed time stamp of the task.
 *
 * If the task's EXECUTE_ONLY_ONCE flag is set, the
 * task will be deactivated (before execution of the
 * main function, see introduction of header file for
 * pro tip)!
 *
 * Parameters:
 * 		- task: pointer to task struct to execute.
 */
void uc_tt_execute_task(timed_task *task) {
	if ( task->flags & (1 << UC_TT_FLAG_BIT_ACTIVE_STATUS) ) {
		task->last_executed_us = API_GET_TIME_US;

		if ( task->flags & (1 << UC_TT_FLAG_BIT_EXECUTE_ONLY_ONCE) ) {
			uc_tt_deactivate_task(task);
		}

		(*task->function)();
	}
}

/* Activates a given task by updating the
 * corresponding flag.
 *
 * If the task's WITHOUT_DELAY flag is set, the task
 * will be immediately executed, otherwise the first
 * time on the next update call (and if enough
 * time has passed).
 *
 * Parameters:
 * 		- task: pointer to task strcut to activate.
 */
void uc_tt_activate_task(timed_task *task) {
	if ( !(task->flags & (1 << UC_TT_FLAG_BIT_ACTIVE_STATUS)) ) {
		task->flags |= (1 << UC_TT_FLAG_BIT_ACTIVE_STATUS);

		if ( task->flags & (1 << UC_TT_FLAG_BIT_EXECUTE_WITHOUT_DELAY) ) {
			uc_tt_execute_task(task);
		} else {
			//"Start timer"
			task->last_executed_us = API_GET_TIME_US;
		}
	}
}

/*
 * This method checks for all listed tasks, if they are
 * activated and the time that has passed since the last
 * execution is greater than the interval. In case this
 * happens, a task will be executed again and the
 * execution time stamp will be updated.
 */
void uc_tt_update() {
	uint64_t current_time = 0;
	timed_task *current_task = 0;

	for ( uint8_t i = 0; i < UC_TT_TASK_BUFFER; i++ ) {
		current_time = API_GET_TIME_US;
		current_task = uc_tt_tasks[i];

		if ( current_task > 0 ) {
			if ( current_task->flags & (1 << UC_TT_FLAG_BIT_ACTIVE_STATUS) ) {
				if ( current_time - current_task->last_executed_us > current_task->interval_us ) {
					uc_tt_execute_task(current_task);
				}
			}
		}
	}
}

#endif
