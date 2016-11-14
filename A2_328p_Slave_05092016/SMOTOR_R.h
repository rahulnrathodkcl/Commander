#ifndef SMOTOR_R_h
#define SMOTOR_R_h
#include <Arduino.h>
#include "Definitions.h"
#include "slaveSPI.h"
#include "S_EEPROM.h"
class slaveSPI;

class SMOTOR_R
{			
	slaveSPI *spi1;
	S_EEPROM *eeprom1;
	
	bool backingOff;
	bool operating;
	bool gotQuery;
	unsigned short int lastMotorPos;
	
	byte reportEventByte;
	bool waitForReporting;

	bool reportedMotorHigh;
	bool reportedMotorLow;
	bool reportedEventLastPos;

	void operateOnQuery();
	void reportEvent(byte);
	void signalOn();
	void signalOff();
	void turnOn();
	void turnOff();
	void switchOff();
	void backOff();
	unsigned short int getAnalogInput();
	void checkEvent();
	byte checkLimit();

	public:

	SMOTOR_R();
	void setEEPROM(S_EEPROM *eeprom1);
	void setSPI(slaveSPI *spi1);
	void query();
	void inform(byte);
	//void setCallBackFunctions(void (*inc)(),void (*dec)(), void (*motorOperated)(bool));
	void update();
};
#endif