
#include "slaveSPI.h"
#include "Definitions.h"

slaveSPI::slaveSPI(void (*startSequenceQuery)(),S_EEPROM *eeprom1)
{
	// _Serial=s1;
	// _Serial->begin(19200);
	this->startSequenceQuery=*startSequenceQuery;
	//
	this->eeprom1=eeprom1;
	pinMode(PIN_INTERRUPTMASTER,OUTPUT);
	SPCR |= bit (SPE);	//enable SPI slave mode
  	pinMode(MISO, OUTPUT);

	sendingSensorData=false;

	waitTime=3;
	sensorDataRequestWaitTime=10;

	SPI.attachInterrupt();
}

#ifdef CHK_BATTERY
	void slaveSPI::setObjectReference(SELF *self1,SMOTOR_R *smotor1,TEMP *temp1,BATTERY *batteryLevel)
	{
		this->temp1=temp1;
		this->smotor1=smotor1;
		this->self1=self1;	
		this->batteryLevel=batteryLevel;
	}

#else
	void slaveSPI::setObjectReference(SELF *self1,SMOTOR_R *smotor1,TEMP *temp1)
	{
		this->temp1=temp1;
		this->smotor1=smotor1;
		this->self1=self1;	
	}
#endif
// void slaveSPI::setInformFunction(void(*iSelf)(byte),void (*iLimit)(byte),void (*ismotor)(byte))
// {
// //	informSelf=*iSelf;
// //	informLimit=*iLimit;
// //	informSmotor=*ismotor;
// }

// void slaveSPI::setQueryFunction(void(*startSeq)(),void(*Temp)(),void (*smotor)(),void(*selfLimit)(),void (*selfMachine)())
// {
// //	startSequenceQuery=*startSeq;
// //	tempQuery=*temp;
// //	smotorQuery=*smotor;
// //	selfLimitQuery=*selfLimit;
// //	selfMachineQuery=*selfMachine;
// }

template <typename T> unsigned int slaveSPI::SPI_readAnything(T& value)
{
	byte * p = (byte*) &value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = SPI.transfer (0);
	return i;
}  

void slaveSPI::receiveInterrupt()
{
	byte temp = SPDR;
	if(temp==Q_EVENT)
	{
		digitalWrite(PIN_INTERRUPTMASTER,LOW);
		if(sendingSensorData)
		{
			if(!lastByte)
				gotSensorDataTrigger=true;
			else
				sendingSensorData=false;
		}
		// _Serial->println("Querying For Data");
		// _Serial->println(SPDR);	
		return;
	}

	byte temp2=temp>>4;
	
	if(temp2==CHKINCOMING_SETTING)
	{
		SPI_readAnything(settingData);
		settingByte=temp;
		settingReceived=true;
	}
	else
	{
		spiDataReceived=true;
		spiData=SPDR;
		digitalWrite(PIN_INTERRUPTMASTER,LOW);
	}
}  // end of interrupt routine SPI_STC_vect

void slaveSPI::operateOnSPIData()
{
	// _Serial->print("Data Received:");
	// _Serial->println(spiData);
	spiDataReceived=false;
	byte temp=spiData >> 4;
	switch(temp)
	{
		case CHKREQUEST_SENSOR:
			operateOnSensorDataRequest();
			return;
		break;
		case CHKINCOMING_QUERY:	//query
			switch(spiData)
			{
				case Q_TEMP:
					temp1->query();
					break;
				case Q_LIMIT:
					self1->limitQuery();
					break;
				case Q_MACHINE:
					self1->machineQuery();
					break;
				case Q_MOTOR:
					smotor1->query();
					break;
				case Q_STARTSEQUENCE:
					startSequenceQuery();
					break;
				case Q_SLAVEEXISTENCE:
					slaveExistenceQuery=true;
					break;
				#ifdef CHK_BATTERY
				case Q_BATTERY:
					batteryLevel->query();
					break;
				#endif
			}
		break;
		case CHKINFORM_SELF:
			self1->informSelf(spiData);
			#ifdef CHK_BATTERY
			batteryLevel->inform(spiData);
			#endif
		break;
		case CHKINFORM_LIMIT:
			self1->informLimit(spiData);
		break;
		case CHKINFORM_MOTORRP:
			smotor1->inform(spiData);
		break;
	}
}

void slaveSPI::operateOnSetting()
{
	settingReceived=false;
	unsigned short int temp=settingData;
	switch(settingByte)
	{
		case S_HIGHRPM:
			eeprom1->saveHighRPMSettings(temp);
		break;
		case S_COMPRPM:
			eeprom1->saveCompRPMSettings(temp);
		break;
		case S_DECOMPRPM:
			eeprom1->saveDecompRPMSettings(temp);
		break;
		case S_HIGHTEMP:
			eeprom1->saveTempSettings(temp);
			break;
		case S_MOTORRHIGH:
			eeprom1->saveMotorHighSettings(temp);
		break;
		case S_MOTORRLOW:
			eeprom1->saveMotorLowSettings(temp);
		break;
		case S_FORCESTART:
			eeprom1->saveForceStartSettings((byte)temp);
		break;
	}
}

bool slaveSPI::sendData(byte data)
{
	if(!sendingSensorData)
	{
		SPDR=data;
		digitalWrite(PIN_INTERRUPTMASTER,HIGH);
		return true;
	}
	return false;
}

void slaveSPI::operateOnSensorDataRequest()
{
	sensorDataIndex=0;
	lastByte=false;
	gotSensorDataTrigger=false;
	sendAnotherSensorDataByte=false;

	sensorDataRequestTime=millis();
	sendingSensorData=true;	
	if(spiData==ASK_RPM)
	{
		unsigned short int temp=self1->getRPM();
		sensorData[0]=temp;
		sensorData[1]=temp>>8;
		sendSensorData();
	}
}

void slaveSPI::sendSensorData()
{
	SPDR=sensorData[sensorDataIndex++];
	if(sensorDataIndex>1)
		lastByte=true;
	digitalWrite(PIN_INTERRUPTMASTER,HIGH);	
}

void slaveSPI::update()
{
	if(slaveExistenceQuery)
	{
		slaveExistenceQuery=false;
		eeprom1->gotMasterQuery=true;
		// _Serial->println("Got Master Init Query..");
		sendData(A_SLAVEEXISTS);
	}

	if(spiDataReceived)
		operateOnSPIData();

	if(settingReceived)
		operateOnSetting();


	if(sendingSensorData)
	{
		if(millis()-sensorDataRequestTime>sensorDataRequestWaitTime)
		{
			sendingSensorData=false;
			return;
		}
		
		if(gotSensorDataTrigger)
		{
			gotSensorDataTrigger=false;
			sendAnotherSensorDataByte=true;
			tempWaitTime=millis();
		}
	
		if(sendAnotherSensorDataByte && millis()-tempWaitTime>waitTime)
		{
			sendAnotherSensorDataByte=false;
			sendSensorData();
		}
	}
}