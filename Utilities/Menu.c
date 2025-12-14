/******************************************************************************
 *	This is an example of how to use the StateMachine module to implement a menu.
 *****************************************************************************/
 
// Code for state transitions.  Defines the paths between all the states, and
// which inputs trigger movement between states.
const stateChange_t stateChangeArray [] =
{
//	State		Input		Next State
	{ST_HOME,	MENU_KEY,	ST_NOOP},
	{ST_NOOP,	MENU_KEY,	ST_HOME},
	{ST_NOOP,	ENTER_KEY,	ST_HOME},

	// This tells StateMachine function to stop.
	// If it is removed, the program will crash!
	{0,			0,			0}
};

// Code for state functions.  Defines the function performed in each state
// as well as a text description displayed to the user at each state.
const state_t menuStateArray [] =
{
//	State		Function
	{ST_HOME,	MenuHome},
	{ST_NOOP,	MenuNoOp},

	// This tells the main loop to stop.
	// If it is removed, the program will crash!
	{0,			0}
};

void MenuHome(void)
{
	// Do something corresponding to the "HOME" state.
}

void MenuNoOp(void)
{
	// Do something corresponding to the "NOOP" state.
	// You could even do nothing at all!
}