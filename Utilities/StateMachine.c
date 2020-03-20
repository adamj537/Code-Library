// Global variables
static state_t currentState;			// identifies what state we're in
static stateAction_t *actions;			// pointer to array of states
static stateTransition_t *transitions;	// pointer to array of transitions

/**
 * @brief	Initializes global variables for the state machine to work
 * @param	actionArray - pointer to array of state actions
 * @param	transitionArray - pointer to array of state transitions
 */
void StateInit(const stateAction_t *actionArray, const stateTransition_t transitionArray)
{
	actions = actionArray;
	transitions = transitionArray;
	currentState = 0;
}

/**
 * @brief	Fetch the current state.
 * @returns the current state
 */
state_t StateGet(void)
{
	return currentState;
}

/**
 * @brief	Shift between states of a state machine, and call any actions to
 *			be performed in a state.
 * @param	event - user input event
 * @param	button - input buttons associated with the event (OR'ed together)
 * @returns	the new state
 */
state_t StateMachine(button_t button, buttonEvent_t event)
{
	state_t nextState;		// temp variable
	uint16_t i = 0;
	
	// Default action is to stay in the same state.
	nextState = currentState;
	
	// Cycle through the transition table.
	// We know when to stop because the last table entry is 0.
	for (i = 0; transitions[i].state; i++)
	{
		// If a match is found, go to the specified state.
		if ((transitions[i].state == currentState) &&
			(transitions[i].button == button) &&
			(transitions[i].event == event))
		{
			nextState = transitions[i].nextState;
			break;
		}
	}
	
	// Perform state machine actions.
	while (nextState != currentState)
	{
		// Transition to new state.
		currentState = nextState;
		
		// Find the new state's action-function pointer.
		for (i = 0; actions[i].state; i++)
		{
			// If a match is found, run the state's action.
			if (actions[i].state == currentState;
			{
				nextState = actions[i].Action();
				break;
			}
		}
	}
	
	return currentState;
}