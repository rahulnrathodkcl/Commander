#ifndef SELF_R_h
#define SELF_R_h
#include <Arduino.h>

class SELF_R
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
	byte SIG_MOTOR;
	byte REL_ALTERNATOR;
	
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

	byte operateTime;
	unsigned long tempTime;
	bool operating;
	bool compressing;
	bool decompressing;

	void (*RPMChange)();
	void (*MachineSwitchedOff)();
	void (*startingSelf)();
	void (*stoppingSelf)();
	
	void compress();
	void decompress();
	bool tryDecompressing();

	bool isCompressed();
	bool isDecompressed();
	void stopMotor();
	float getAnalogInput();
	bool timeOut();
	bool checkOperatingElligible();

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

	bool firedRPMEvent;
	
	float compressionValue;
	float decompressionValue;
	
	public:

	bool gotTrigger;
	volatile bool machineOn;
	bool machineOff;

  volatile unsigned long lastrise;
  volatile unsigned long currentrise;

	double ARPM;

	SELF_R(byte,byte,byte,byte,byte,byte,byte,int,int,int,int,HardwareSerial *s,float,float);

	void IVR_RPM();

	void ignoreAverageReadings();
	float requestMotorReading();

	void setCallBackFunctions(void (*funcRPMChange)(),void (*machineSwitchOff)(),void (*funcSelfStart)(),void (*funSelfStop)());
	bool triggerSelf();
	bool setMachineStatus();
	double getRPM();
	double getAverageRPM();
	void discardRPMEvent();
	void update();
};
#endif
