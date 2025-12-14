/******************************************************************************
*
*	Function:		SetpointCycle
*
*	Description:	This function attempts to set a process variable to a
*					setpoint for a certain amount of time, and also implements
*					a timeout.  Setpoint and rate of change are checked.
*
*	Parameters:		double setpoint - the desired setpoint.
*					double errorTol - the allowable tolerance in the setpoint.
*					double rateTol - tolerance in the derivative of setpoint.
*					int dwellTime - how long do we need to wait at setpoint?
*					float frequency - how often (Hz) do we sample process?
*					int timeout - timeout (seconds).
*
*	Return Value:	int - nonzero values indicate errors.
*
******************************************************************************/

static int SetpointCycle (double setpoint, double errorTol, double rateTol,
	int dwellTime, float frequency, int timeout)
{
	int status;						//return value from functions (0 = OK)
	double *reading;				//pointer to array of readings
	int numReadings = 0;			//size of reading array
	double error;					//difference between reading and setpoint
	double rate;					//rate of change of readings
	int counter;					//counter
	double mark;					//time stamp
	double dwellLeft = dwellTime;	//dwell time (seconds)
	double timeoutLeft = timeout;	//timeout (seconds)
	
	// Determine how big the array of readings must be (per the frequency).
	if (frequency <= 0) return -1;
	else if (frequency < 1) numReadings = 2;
	else numReadings = (int)frequency;
	
	// Allocate memory for an array of readings (and initialize to setpoint).
	reading = malloc (sizeof(double) * numReadings);
	for (counter = 0; counter < numReadings; counter++){
		reading[counter] = setpoint;
	}
	
	// Set setpoint.
	status = ControllerSetSetpoint(setpoint);
	
	// Get start time.
	mark = Timer();
	
	// Take readings until they are within tolerance for "dwellTime" seconds.
	while (dwellLeft > 0.0) {
		// Check the timeout.
		if ((timeoutLeft <= 0.0) && (timeout != 0)) {
			PopupAlarm ("Not able to reach stability.");
			dwellLeft = dwellTime;
			timeoutLeft = timeout;
			mark = Timer();	//start over
		}
		
		// Increment the array of readings.
		for (counter = numReadings - 1; counter > 0; counter--){
			reading[counter] = reading[counter-1];
		}
		
		// Get a new reading.
		status = ReferenceGetReading(&reading[0]);
		
		// Some errors (like a device out of range) require starting over.
		if (status == DEVICE_OUT_OF_RANGE){
			dwellLeft = dwellTime;
		}
		
		// Other errors should make us stop (and return an error).
		else if (status) break;
		
		// Otherwise, process the new data.
		else {
			// Calculate error.
			error = reading[0] - setpoint;
		
			// Calculate rate of change.
			rate = (reading[0] - reading[numReadings - 1]) * frequency;
		
			// Check tolerance.
			if((error < -errorTol) || (error > errorTol)){
				dwellLeft = dwellTime;
			}
			
			// Reset timeout if necessary.
			if((rate < -rateTol) || (rate > rateTol)){
				dwellLeft = dwellTime;
			}
		}
		
		// Update GUI with time, process value, error, rate.
		GUIPrintf ("Stability Time Left = %d:%02d", (int)(dwellLeft / 60),
			(int)dwellLeft % 60);
		GUISetProcessValue (reading[0]);
		GUISetError (error);
		GUISetRate (rate);
	
		// Wait here so we take readings at the desired frequency.
		mark += 1/frequency;
		while(Timer() < mark) {ProcessSystemEvents();}
		dwellLeft -= 1/frequency;
		timeoutLeft -= 1/frequency;
	}
	
	// Free the array of readings.
	free (reading);
	
	return status;
}