#ifndef slaveSPI_h
#define slaveSPI_h

#include <Arduino.h>
#include <SPI.h>
#include "Definitions.h"
#include "S_EEPROM.h"

#include "SELF.h"
#include "SMOTOR_R.h"
#include "TEMP.h"
#include "BATTERY.h"

class SELF;
class SMOTOR_R;
class TEMP;
class BATTERY;

class slaveSPI
{
	SELF *self1;
	SMOTOR_R *smotor1;
	TEMP* temp1;
	S_EEPROM *eeprom1;
	
	bool sendingSensorData;
	bool gotSensorDataTrigger;
	bool lastByte;
	bool sendAnotherSensorDataByte;
	unsigned long tempWaitTime;
	byte waitTime;
	byte sensorData[2];
	byte sensorDataIndex;

	#ifdef CHK_BATTERY
		BATTERY* batteryLevel;
	#endif

	void (*startSequenceQuery)();

	// void (*tempQuery)();
	// void (*smotorQuery)();
	// void (*selfLimitQuery)();
	// void (*selfMachineQuery)();


	// void (*informSelf)(byte);
	// void (*informLimit)(byte);
	// void (*informSmotor)(byte);

	float settingData;
	byte settingByte;
	bool settingReceived;
	volatile bool spiDataReceived;
	volatile byte spiData;
	bool slaveExistenceQuery;

	volatile byte queryAnswer;

	template <typename T> unsigned int SPI_readAnything(T& value);	
	void operateOnSPIData();
	void operateOnSetting();
	void operateOnSensorDataRequest();

	void sendSensorData();
public:

	slaveSPI(void (*startSequenceQuery)(),S_EEPROM *eeprom1);
	#ifdef CHK_BATTERY
		void setObjectReference(SELF *sefl1,SMOTOR_R *smotor1,TEMP *temp1,BATTERY* batteryLevel);
	#else
		void setObjectReference(SELF *sefl1,SMOTOR_R *smotor1,TEMP *temp1);
	#endif

	// void setInformFunction(void(*iSelf)(byte),void (*iLimit)(byte),void (*ismotor)(byte));
	// void setQueryFunction(void(*startSeq)(),void(*Temp)(),void (*smotor)(),void(*selfLimit)(),void (*selfMachine)());
	void receiveInterrupt();
	bool sendData(byte);
	void update();
};
#endif