#include "SMOTOR_R.h"

SMOTOR_R::SMOTOR_R()
{
	pinMode(PIN_SMOTORPWR,OUTPUT);
	reportedMotorHigh=false;
	reportedMotorLow=false;
	reportedEventLastPos=false;
	
	operating=false;
	backingOff=false;
}

void SMOTOR_R::setEEPROM(S_EEPROM *eeprom1)
{
	this->eeprom1=eeprom1;
}

void SMOTOR_R::setSPI(slaveSPI *spi1)
{
	this->spi1=spi1;
}

void SMOTOR_R::query()
{
	gotQuery=true;
}

void SMOTOR_R::operateOnQuery()
{	
	gotQuery=false;
	spi1->sendData(checkLimit());
}

void SMOTOR_R::reportEvent(byte eventByte)
{
	reportEventByte=eventByte;
	waitForReporting=!(spi1->sendData(reportEventByte));
}

void SMOTOR_R::inform(byte information)
{
	switch(information)
	{
		case I_MOTORRPON:
			turnOn();
			break;
		case I_MOTORRPOFF:
			turnOff();
			break;
		case I_MOTORSWOFF:
			switchOff();
			break;
		case I_MOTORBOFF:
			backOff();
			break;
	}
}

void SMOTOR_R::signalOn()
{
	digitalWrite(PIN_SMOTORPWR,HIGH);	
}

void SMOTOR_R::signalOff()
{
	digitalWrite(PIN_SMOTORPWR,LOW);	
}

void SMOTOR_R::turnOn()
{
	operating=true;
	signalOn();
}

void SMOTOR_R::turnOff()
{
	operating=false;
	backingOff=false;
	signalOff();
	reportedMotorHigh=false;
	reportedMotorLow=false;
	reportedEventLastPos=false;
}

void SMOTOR_R::switchOff()
{
	lastMotorPos=getAnalogInput();
	turnOn();
}

void SMOTOR_R::backOff()
{
	backingOff=true;
	turnOn();
}


unsigned short int SMOTOR_R::getAnalogInput()
{
	unsigned long temp;
	signalOn();
	analogRead(PIN_SMOTORSIG);
	temp=millis();
	while(millis()-temp<2)
	{}
	temp=analogRead(PIN_SMOTORSIG);
	signalOff();
	return (unsigned short int)temp;
}

void SMOTOR_R::checkEvent()
{
	if(operating)
	{
		byte temp=checkLimit();
		if(temp!=A_MOTORNORMAL)
		{
			if(temp==A_MOTORLOW && !reportedMotorLow)
			{
				reportedMotorLow=true;
				reportEvent(EVENT_MOTORLOW);
			}
			else if(temp==A_MOTORPREVIOUSSTATE && !reportedEventLastPos)
			{
				reportedEventLastPos=true;
				reportEvent(EVENT_MOTORPREVIOUSSTATE);
			}
			else if(temp==A_MOTORHIGH && !reportedMotorHigh)
			{
					reportedMotorHigh=true;
					reportEvent(EVENT_MOTORHIGH);
			}
		}
	}
}

byte SMOTOR_R::checkLimit()
{	
	unsigned short int temp=getAnalogInput();
	if(temp<=(eeprom1->MOTORLOW))
		return A_MOTORLOW;
	else if(temp>=(eeprom1->MOTORHIGH))
		return A_MOTORHIGH; 
	else 
	{
		if(operating && backingOff)
		{
			if(temp<=lastMotorPos)
				return A_MOTORPREVIOUSSTATE;
		}
		return A_MOTORNORMAL;
	}
}

void SMOTOR_R::update()
{		
	if(waitForReporting)
		waitForReporting=!(spi1->sendData(reportEventByte));

	if(operating || backingOff)
		checkEvent();
	if(gotQuery)
		operateOnQuery();
}