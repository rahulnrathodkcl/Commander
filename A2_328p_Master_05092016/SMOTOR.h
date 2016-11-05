#ifndef SMOTOR_h
#define SMOTOR_h
#include <Arduino.h>

class SMOTOR
{			
	byte REL_SLOW;
	byte REL_FAST;
	byte PWR_LIM;

	byte speedWaitTime;
	unsigned long speedWait;
	
	int offWaitTime;
	unsigned long offWait;

	byte waitTime;
	unsigned long wait;

	unsigned long operateTime;
	bool abnormalStop;

	unsigned long backOffWait;
	bool backingOff;


	bool operationPerformed;
	bool operating;
	char currentOperation;

	void (*finc)();
	void (*fdec)();
	void (*foff)();
	
	void stopOperating();
	bool checkStopOperatingElligible();
	
	bool stopBackingOffElligible();
	void stopBackOff();
	
	bool makeResponseElligible();
	void makeResponse();


	public:
	bool reachedSlowEnd;
	bool reachedFastEnd;

	SMOTOR(byte SLOWPIN,byte FASTPIN,byte PWRPIN);
	void setCallBackFunctions(void (*inc)(),void (*dec)(),void (*off)());
	void increaseRPM();
	void decreaseRPM();
	void switchOff();
	void backOff();
	char checkCurrentOperation();
	char checkLimitSwitch();
	void update();
};
#endif