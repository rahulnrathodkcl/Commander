#include <Arduino.h>
#include <SPI.h>
#include "masterSPI.h"

#ifndef disable_debug
	#ifdef software_SIM
		masterSPI::masterSPI(HardwareSerial *serial1, bool *tempLimit,bool *smotorEventOccured, byte *smotorEventByte,bool *selfEventOccured, byte *selfEventByte,bool *batteryLow,bool *securityEvent)
		{
			_Serial=serial1;
			_Serial->begin(19200);
			anotherConstructor(tempLimit,smotorEventOccured,smotorEventByte,selfEventOccured,selfEventByte,batteryLow,securityEvent);
		}
	#else
		masterSPI::masterSPI(SoftwareSerial *serial1, bool *tempLimit,bool *smotorEventOccured, byte *smotorEventByte,bool *selfEventOccured, byte *selfEventByte,bool *batteryLow,bool *securityEvent)
		{
			_Serial=serial1;
			_Serial->begin(19200);
			anotherConstructor(tempLimit,smotorEventOccured,smotorEventByte,selfEventOccured,selfEventByte,batteryLow,securityEvent);
		}
	#endif
#else
	masterSPI::masterSPI(bool *tempLimit,bool *smotorEventOccured, byte *smotorEventByte,bool *selfEventOccured, byte *selfEventByte,bool *batteryLow,bool *securityEvent)
	{
		anotherConstructor(tempLimit,smotorEventOccured,smotorEventByte,selfEventOccured,selfEventByte,batteryLow,securityEvent);
	}
#endif

void masterSPI::anotherConstructor(bool *tempLimit,bool *smotorEventOccured, byte *smotorEventByte,bool *selfEventOccured, byte *selfEventByte,bool *batteryLow,bool *securityEvent)
{  	
  	this->tempLimit=tempLimit;
  	this->smotorEventOccured=smotorEventOccured;
  	this->smotorEventByte=smotorEventByte;

  	this->selfEventOccured=selfEventOccured;
  	this->selfEventByte=selfEventByte;

  	this->batteryLow=batteryLow;
  	this->securityEvent=securityEvent;

	pinMode(SLAVE_READY_INTERRUPT_PIN,INPUT);
  	pinMode(SS,OUTPUT);
  	digitalWrite(SS,HIGH);
 	  	
  	SPI.begin();
  	SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
}

template <typename T> unsigned int masterSPI::SPI_writeAnything (const T& value)
{
    const byte * p = (const byte*) &value;
    unsigned int i;
    for (i = 0; i < sizeof value; i++)
          SPI.transfer(*p++);
    return i;
 }  // end of SPI_writeAnything


void masterSPI::ISR_SlaveReady()
{
		receiveByte();
		byte temp;
		temp=receivedByte>>4;
		
		#ifndef disable_debug
			_Serial->print("RB:");
			_Serial->println(receivedByte);
		#endif

		if(engaged && temp==CHKINCOMING_ANSWER)
		{	
				*answer=receivedByte;
				*dataReceived=true;
				engaged=false;
				return;
		}

		switch(temp)
		{
			case CHKEVENT_LIMIT:
			case CHKEVENT_RPM:
				*selfEventOccured=true;
				*selfEventByte=receivedByte;
				break;				
			case CHKEVENT_TEMP:
				*tempLimit=true;
				break;		
			case CHKEVENT_MOTOR:
				*smotorEventOccured=true;
				*smotorEventByte=receivedByte;
				break;
			case CHKEVENT_SECURITY:
				*securityEvent=true;
				break;
			#ifdef CHK_BATTERY
			case CHKEVENT_BATTERY:
				*batteryLow=true;
				break;
			#endif
		}
}

bool masterSPI::sendSettings(byte settingsByte ,float data)
{
	if(!engaged)
	{
		#ifndef disable_debug
			_Serial->print("Sending ");
			_Serial->println("Settings");
		#endif
		digitalWrite(SS, LOW);
		SPI.transfer(settingsByte);
		SPI_writeAnything(data);
		digitalWrite(SS, HIGH);
		return true;
	}
	return false;
}

void masterSPI::sendByte(byte data)
{
	digitalWrite(SS,LOW);
	byte b=data;
	SPI.transfer(b);
	digitalWrite(SS,HIGH);
}

bool masterSPI::queryState(byte query,byte *answer,bool *dataReceived)
{
	if(!engaged)
	{
		//acquiredDataFromSlave=false;
		engaged=true;
		engageTime=millis();
		this->answer=answer;
		*dataReceived=false;
		this->dataReceived=dataReceived;
		#ifndef disable_debug
			_Serial->print("Sending ");
			_Serial->print("Query:");
			_Serial->println(query);
		#endif
		sendByte(query);
		return true;
	}
	return false;
}

bool masterSPI::inform(byte data)
{
	if(!engaged)
	{
		#ifndef disable_debug
			_Serial->print("I:");
			_Serial->println(data);
		#endif
		sendByte(data);
	}
	else
		return false;
	return true;
}

byte masterSPI::receiveByte()
{
	digitalWrite(SS,LOW);
	receivedByte=SPI.transfer((byte)Q_EVENT);
	digitalWrite(SS,HIGH);
	return 0x00;
}

void masterSPI::update()
{	
	if(engaged && millis()-engageTime>3500)
		engaged=false;
}