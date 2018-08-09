/******************************************************************************
 *
 *	Filename:		Tasks.h
 *
 *	Author:			Adam Johnson
 *
 *	Description:	A lightweight task scheduler.
 *
 *	Inspired by:	http://www.embedded.com/design/programming-languages-and-tools/4007578/A-six-step-process-for-migrating-embedded-C-into-a-C--object-oriented-framework
 *
 *	Terms of Use:	MIT License
 *
 *****************************************************************************/

#define MAX_TASKS	10					// maximum number of scheduled tasks

typedef void (*callbackPtr_t) (void);	// a type for function pointer

void    TasksInit(void);										// start
uint8_t TasksAdd(callbackPtr_t Callback, uint16_t msInterval);	// add task
uint8_t TasksDelete(callbackPtr_t Callback);					// delete task
void    TasksRun(void);											// run in main loop
void    TasksISR(void);											// run in interrupt