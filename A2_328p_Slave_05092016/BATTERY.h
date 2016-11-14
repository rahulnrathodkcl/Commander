#ifndef BATTERY_h

#define BATTERY_h
#include "Definitions.h"
#include "slaveSPI.h"
class BATTERY
{
	slaveSPI *spi1;

	byte batteryLevel;
	unsigned long lastCheck;
	bool alarmed;
	bool gotQuery;
	bool selfOn;
	bool alarmTrigger;

	void operateOnQuery();
	bool checkSufficientLevel();
	byte getBatteryLevel();
	void checkBatteryLevel();

	public:
	BATTERY();
	void setSPI(slaveSPI *spi1);
	void query();
	void inform(byte);
	void checkInitialBatteryLevel();
	void update();
};
#endif