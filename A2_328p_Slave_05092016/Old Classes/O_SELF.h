#ifndef SELF_h
#define SELF_h
#include <Arduino.h>

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

	byte cntforoff;
	byte cntforon;

  long motorCompressWait;
  long motorCompressWaitTime;
  bool waitForCompression;
  
	char cMotorStatus;

  bool motorCompression;
  bool motorDecompression;
  

	int selfOnTime;
	int selfOffTime;
	bool selfOn;
	bool triggerSelfOn;
	unsigned long selfWaitTime;

	int averageTime;
	unsigned long averageWaitTime;
	bool ignoreAvgReadings;
	byte ignoredReadingsCnt;

    HardwareSerial* _NSerial;
	bool didDecompress;
	bool throw2Readings;
	byte throwReadingsCnt;

	bool varStopCompression;
	bool varStopDecompression;

	unsigned long operatedWait;
	unsigned long operatedWaitTime;

	void stopCompression();
	bool stopCompressionElligible();

	void stopDecompression();
	bool stopDecompressionElligible();

	void (*RPMChange)();
	void (*MachineSwitchedOff)();
	void (*startingSelf)();
	void (*stoppingSelf)();
	
	void compress();
	void decompress();
	bool tryDecompressing();

	bool startSelfElligible();
	void startSelf();	
	bool stopSelfElligible();
	void stopSelf();

	void checkNoRPM();
	char checkCMotorStatus();

	bool averageRPMElligible();
	void averageRPM();

	void turnMachineOn();
	void turnMachineOff();
	bool getMachineStatus();

	void turnSignalOn();
	void turnSignalOff();
	byte signalWaitTime=25;
	unsigned long signalWait;
	bool Power;

	bool firedRPMEvent;
	
	
	public:

	bool gotTrigger;
	volatile bool machineOn;
	bool machineOff;

  volatile unsigned long lastrise;
  volatile unsigned long currentrise;

	double ARPM;

	SELF(byte,byte,byte,byte,byte,byte,byte,int,int,int,int,HardwareSerial *s);

	void IVR_RPM();
	void IVR_SCOMPRESS();
	void IVR_SDECOMPRESS();

	void ignoreAverageReadings();

	void setCallBackFunctions(void (*funcRPMChange)(),void (*machineSwitchOff)(),void (*funcSelfStart)(),void (*funSelfStop)());
	bool triggerSelf();
	bool setMachineStatus();
	double getRPM();
	double getAverageRPM();
	void discardRPMEvent();
	void update();
};
#endif
