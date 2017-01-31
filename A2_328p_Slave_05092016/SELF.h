#ifndef SELF_h
#define SELF_h

#include <Arduino.h>
#include "S_EEPROM.h"
#include "slaveSPI.h"
#include "Definitions.h"
class slaveSPI;

class SELF
{
	private:
		S_EEPROM* eeprom1;
		slaveSPI *spi1;

  	bool gotStartQuery;
  	bool gotLimitQuery;
  	bool gotMachineQuery;
  	bool queryingMachineState;

  	bool reportedForceStart; 

	volatile double RPM;
	byte limitSwitchOperated;
  	unsigned long limitSwitchOperatedAt;

	byte compSwitchChangedOffTime;
	byte decompSwitchChangedOffTime;

	byte reportEventByte;
	bool waitForReporting;

	bool compressing;
  	bool decompressing;
  	bool reportedCOMPRPM;
	bool reportedDECOMPRPM;  	  	
	bool reportedLimitCOMPEvent;
	bool reportedLimitDECOMPEvent;

  	byte HRPMCnt;
	bool didCompress;
	bool firedRPMEvent;
	bool signalOn;
	bool selfOn;
	unsigned long tempSignalTime;

	unsigned long tempSelfTime;
	int selfTime;

	byte startCnt;

	void (*sendMotorStatus)(bool);
	void (*startingSelf)();
	void (*stoppingSelf)();

	void operateOnLimitQuery(bool);
	void operateOnMachineQuery();
	void reportEvent(byte);

	bool isDecompressed();

	void compress();
	void decompress();
	void selfStarted();	
	void selfStopped();

	void checkNoRPM();

	void setMachineStatus();
	
	void turnMachineOn();
	void turnMachineOff();
	void turnSignalOn();
	void turnSignalOff();
	void checkAndReportLimitEvent();

	void returnStartQueryAnswer(bool decompressed);
	bool stopSignalElligible();

	public:

	bool gotTrigger;
	volatile bool machineOn;

  	volatile unsigned long lastrise;
  	volatile unsigned long currentrise;

	SELF(void (*retMotorStatus)(bool),void (*funcSelfStart)(),void (*funSelfStop)());
	void setEEPROM(S_EEPROM* e1);
	void setSPI(slaveSPI *spi1);
	void limitQuery();
	void machineQuery();
	void informSelf(byte);
	void informLimit(byte);

	void IVR_RPM();
	void IVR_SDECOMPRESS();
	void triggerMachineStatus();
	double getRPM();
	bool getMachineStatus();
	void startSequenceQuery();
	void update();
};
#endif
