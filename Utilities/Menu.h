/******************************************************************************
 *	This is an example of how to use the StateMachine module to implement a menu.
 *****************************************************************************/

#ifndef MENU_H
#define MENU_H

enum									// Menu state identifiers
{
	ST_HOME = 1,						// need to make sure no state is 0
	ST_NOOP,
};

// State functions (one for each value in the above enumeration)
void MenuHome(void);
void MenuNoOp(void);

#endif /* MENU_H */