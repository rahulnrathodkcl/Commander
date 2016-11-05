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

	void operateOnQuery();
	bool checkSufficientLevel();
	byte getBatteryLevel();
	void reportEvent(byte);
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