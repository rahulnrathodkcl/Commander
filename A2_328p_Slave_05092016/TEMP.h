#ifndef TEMP_h
#define TEMP_h
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Definitions.h"
#include "slaveSPI.h"
#include "S_EEPROM.h"

class slaveSPI;
class TEMP
{	
	slaveSPI *spi1;
	S_EEPROM *eeprom1;

	bool gotQuery;
	bool tempAlarmRaised;
	bool triggerAlarm;

	unsigned long wait;
	unsigned long getTempWait;
	bool tempRequested;

	OneWire *oneWire;
	DallasTemperature *sensors;
	DeviceAddress tempSensor;
	
	float temperature;
	bool sensorOn;

	bool tempElligible();
	void requestTemp();
	bool getTempElligible();
	void getTemp();
	void operateOnQuery();
	void operateOnEvent();

	public:	

		TEMP();
		void query();	
		void setSPI(slaveSPI *spi1);
		void setEEPROM(S_EEPROM *eeprom1);
		float retTemp();
		bool checkTempLimitReached();
		void init();
		void update();
};
#endif