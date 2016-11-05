#ifndef SELF_h
#define SELF_h
//#include <Arduino.h>

#include "S_EEPROM.h"
#include "masterSPI.h"
#include "Defintions.h"

class SELF
{
	private:
	masterSPI *spi1;

	bool statusDataReceived;
	byte statusData;

	bool selfStartDataReceived;
	byte selfStartData;

	//byte operateWaitTime;

  	bool operating;
	unsigned long operateTime;
	bool compressing;
  	bool decompressing;
  	bool tryingDecompression;

	//int selfOnTime;
	//int selfOffTime;
	bool selfOn;
	bool triggerSelfOn;
	unsigned long selfWaitTime;

	#ifndef disable_debug
  		#ifdef software_SIM
		    HardwareSerial* _NSerial;
		#else
		    SoftwareSerial* _NSerial;
		#endif
	#endif

	bool didDecompress;

	bool firedRPMEvent;
	//bool ignoreHighRPM;

	void (*RPMChange)();
	void (*MachineSwitchedOff)();
	void (*MachineSwitchedOn)();
	//void (*startingSelf)();
	//void (*stoppingSelf)();
	void (*returnSelfCommandStatus)(char);

	
	bool stopMotorElligible();
	void stopMotor();

	void compress();
	void decompress();
	void triggerTryDecompress();
	void stopTryingDecompressing(bool decompressed);
	void operateOnLimitEvent();
	void noRPM();
	void operateOnHighRPMEvent();
	void operateOnCOMPRPM();
	void operateOnDECOMPRPM();
	void operateOnEvent();

	//bool tryDecompressing();
	void operateOnSelfStartData();
	void startSelfCountDown();
	bool startSelfElligible();
	void startSelf();	
	bool stopSelfElligible();
	void stopSelf();


	void turnMachineOn();
	void turnMachineOff();
	bool getMachineStatus();

	void turnSignalOn(bool customInform,bool compressing);
	void turnSignalOff();

	void operateOnMachineStatusData();

	bool queryAgain;

	public:

	bool eventOccured;
	byte eventByte;

	S_EEPROM* eeprom1;

	volatile bool machineOn;
	bool machineOff;

	#ifndef disable_debug
  		#ifdef software_SIM
			SELF(HardwareSerial *s);
		#else
			SELF(SoftwareSerial *s);
		#endif
	#else
		SELF();
	#endif
	void anotherConstructor();
	void setSPI(masterSPI *spi1);
	void setEEPROM(S_EEPROM* e1);
	void triggerSelf();
	void setCallBackFunctions(void (*funcRPMChange)(),void (*machineSwitchOff)(),void (*machineSwitchOn)(),void (*retSelfCmdStatus)(char));

	//void discardRPMEvent();
	//void ignoreHighRPMEvent();
	void queryMachineState();
	void update();
};
#endif