//vERSION 2 DATED 30052016

#ifndef DSL_h
#define DSL_h
#include <Arduino.h>

class DSL{

	byte SEN_DSL;
	byte PWR_DSL;

	float minDiesel;
	float maxDiesel;
	float offLevel;
	float reserveLevel;
	
	bool SEN_ON;
	bool sensorOperation;

	bool sensorConnected;

	double dieselEmptyRate;

	bool generatedDieselLowAlarm;
	bool generatedDieselRateAlarm;

	byte readingCnt;
	unsigned long readingWaitTime;
	unsigned long readingWait;

	HardwareSerial *_Serial;

	double CDiesel;
	double lastReading;
	double lastRateReading;
	unsigned long lastRateReadingTime;

	double sensitivity;
	void (*fdieselRate)();
	void (*fdieselLow)();
	void (*fMachineOff)();


	void isSensorConnected();
	bool readingElligible();
	void turnSensorOff();
	void getAverage();
	void checkDieselLow();
	bool checkDieselRate();
	void turnSensorOn();
	void getReading();
	void setDieselEmptyRate();
	float calculateETA();
  	double V2;

	public:
	DSL(byte DSLSenPin,byte PWRDSLPin,float Maxr,float Minr,float offr, float rr,double sensitivity,HardwareSerial *serial);
	void setCallBackFunctions(void (*funcDieselRate)(), void (*funcDieselLow)(),void (*funcSOff)());
	void update();
	void setSensorOperation(bool);
};
#endif