#ifndef SMOTOR_R_h
#define SMOTOR_R_h

//#include <Arduino.h>

#include "masterSPI.h"
#include "Defintions.h"

class SMOTOR_R
{			

	masterSPI *spi1;
	bool queryAgain;

	bool selfOperating;

	byte steps;
	
	byte speedWaitTime;
	unsigned long speedWait;
	unsigned long switchOffWaitTime;
	
	byte waitTime;
	unsigned long wait;

	bool backingOff;

	bool operationPerformed;
	bool operating;
	char currentOperation;
	char actionTriggered;

	void (*finc)();
	void (*fdec)();
	void (*fMachineNotSwitchedOff)();
	void (*fMotorOperated)(bool);
	void (*returnCommandStatus)(bool);
	
	bool queryMotorState();

	void increaseRPM();
	void decreaseRPM();
	void switchOff();

	void stopOperating();
	bool operateOnEvent();
	bool operatingTimeOver();
	void stopBackOff();

	bool makeResponseElligible();
	void makeResponse();
	void operateOnSPIData();

	#ifndef disable_debug
		#ifdef software_SIM
			HardwareSerial *_Serial;
		#else
			SoftwareSerial *_Serial;
		#endif
	#endif

	public:

	bool eventOccured;
	byte eventByte;
	
	bool spiDataReceived;
	byte spiData;

	#ifndef disable_debug
		#ifdef software_SIM
			SMOTOR_R(HardwareSerial *serial,void (*inc)(),void (*dec)(), void (*motorOperated)(bool),void (*mnotSwitchedOff)(),void(*retStatus)(bool));
		#else
			SMOTOR_R(SoftwareSerial *serial,void (*inc)(),void (*dec)(), void (*motorOperated)(bool),void (*mnotSwitchedOff)(),void(*retStatus)(bool));
		#endif
	#else
		SMOTOR_R(void (*inc)(),void (*dec)(), void (*motorOperated)(bool),void (*mnotSwitchedOff)(),void(*retStatus)(bool));
	#endif
	void anotherConstructor(void (*inc)(),void (*dec)(), void (*motorOperated)(bool),void (*mnotSwitchedOff)(),void(*retStatus)(bool));
	
	void triggerDecreaseRPM(byte steps=1);	
	void triggerSwitchOff();
	void triggerIncreaseRPM(byte steps=1);
	
	void setSPI(masterSPI *spi1);

	void backOff();
	char checkCurrentOperation();
	void setSelfOperationStatus(bool);
	void update();
};
#endif