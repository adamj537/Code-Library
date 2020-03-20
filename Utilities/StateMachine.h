// State machines are a collection of states, where each state has associated (1) actions and (2) transitions.

typedef uint16_t state_t;				// state identifier

typedef struct							// STATE TRANSITION
{
	state_t state;						// state this transition applies to
	button_t button;					// button combination (i.e. BUTTON_A | BUTTON_B)
	buttonEvent_t event;				// button event (i.e. HOLD, PRESS & RELASES)
	state_t nextState;					// state to transition to
} stateTransition_t;

typedef struct							// STATE ACTION
{
	state_t state;						// state this action applies to
	functionPtr Action;					// function to call when state occurs
} stateAction_t;

// Set up the state machine.
void StateInit(const stateAction_t *actionArray, const stateTransition_t transitionArray);

// Fetch the current state.
state_t StateGet(void);

// Clock the state machine.
state_t StateMachine(button_t button, buttonEvent_t event);