#ifndef SELF_h
#define SELF_h
#include <Arduino.h>
#include "S_EEPROM.h"

class SELF
{
	private:

	volatile double RPM;
	double tRPM;
	unsigned long int aCNT;
	int CRPM;
	int DRPM;


	byte SEN_RPM;
	byte REL_SELF;
	byte REL_COMPRESS;
	byte REL_DECOMPRESS;
	byte PWR_CMOTOR;
	byte SIG_COMPRESS;
	byte SIG_DECOMPRESS;
	byte REL_ALTERNATOR;


	byte limitSwitchWaitTime;
	byte limitSwitchOperated;
  	unsigned long limitSwitchOperatedAt;

  	bool operating;
	byte operateWaitTime;
	unsigned long operateTime;
	bool switchChanged;
	unsigned long tempSwitchTime;
	byte switchChangedOffTime;

	 bool compressing;
  	bool decompressing;

  	long motorCompressWait;
  	long motorCompressWaitTime;
  	bool waitForCompression;
  
	int selfOnTime;
	int selfOffTime;
	bool selfOn;
	bool triggerSelfOn;
	unsigned long selfWaitTime;

	int averageTime;
	unsigned long averageWaitTime;
	bool ignoreAvgReadings;
	byte ignoredReadingsCnt;
	byte HRPMCnt;

    HardwareSerial* _NSerial;
	bool didDecompress;

	bool firedRPMEvent;
	bool signalOn;
	unsigned long tempSignalTime;

	bool didCompress;
	byte startCnt;

	void (*RPMChange)();
	void (*MachineSwitchedOff)();
	void (*MachineSwitchedOn)();
	void (*startingSelf)();
	void (*stoppingSelf)();
	

	bool isDecompressed();
	bool isCompressed();
	bool stopMotorElligible();
	void stopMotor();

	void compress();
	void decompress();
	bool tryDecompressing();


	bool startSelfElligible();
	void startSelf();	
	bool stopSelfElligible();
	void stopSelf();

	void checkNoRPM();
	//	char checkCMotorStatus();

	bool averageRPMElligible();
	void averageRPM();

	void turnMachineOn();
	void turnMachineOff();
	bool getMachineStatus();

	bool limitSwitchReadingElligible();

	void turnSignalOn();
	void turnSignalOff();
	
	//byte signalWaitTime=25;
	//unsigned long signalWait;
	//bool Power;

	public:

	bool gotTrigger;
	volatile bool machineOn;
	bool machineOff;
	bool sCompress;
	bool sDecompress;

  	volatile unsigned long lastrise;
  	volatile unsigned long currentrise;

	double ARPM;

	SELF(byte,byte,byte,byte,byte,byte,byte,byte,int,int,int,int,HardwareSerial *s);

	void IVR_RPM();
	void IVR_SCOMPRESS();
	void IVR_SDECOMPRESS();

	S_EEPROM* eeprom1;
	void setEEPROM(S_EEPROM* e1);
	
	bool triggerSelf();
	void setCallBackFunctions(void (*funcRPMChange)(),void (*machineSwitchOff)(),void (*machineSwitchOn)(),void (*funcSelfStart)(),void (*funSelfStop)());
	bool setMachineStatus();
	double getRPM();
	double getAverageRPM();

	void ignoreAverageReadings();

	void discardRPMEvent();
	void update();
};
#endif
