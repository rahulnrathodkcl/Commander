#include "TEMP.h"

TEMP::TEMP()
{
	wait=0;
	getTempWait=0;
	tempRequested=false;
	
	oneWire=new OneWire(PIN_TEMPSEN);
	sensors=new DallasTemperature(oneWire);

	sensors->begin();
	tempAlarmRaised=false;
	gotQuery=false;

	if(sensors->getDeviceCount()==0)
	{
  		sensorOn=false;
	}	
	else
	{
  		sensorOn=true;
  		sensors->getAddress(tempSensor, 0);	
	}
  	sensors->setWaitForConversion(false);
  	temperature=0;
}

void TEMP::query()
{
	gotQuery=true;	
}

void TEMP::setSPI(slaveSPI *spi1)
{
	this->spi1=spi1;
}

void TEMP::setEEPROM(S_EEPROM *eeprom1)
{
	this->eeprom1=eeprom1;
}

float TEMP::retTemp()
{
	return temperature;
}

bool TEMP::tempElligible()
{
	return (sensorOn && !tempRequested && ((millis()-wait)>=TEMPWAITTIME));	
}

void TEMP::requestTemp()
{
	sensors->requestTemperatures();	
	getTempWait=millis();
	tempRequested=true;
}

bool TEMP::getTempElligible()
{
	return(sensorOn && tempRequested && (millis()-getTempWait)>=GETTEMPWAITTIME);
}

void TEMP::getTemp()
{
	temperature=sensors->getTempC(tempSensor);
	wait=millis();
	tempRequested=false;
}

bool TEMP::checkTempLimitReached()
{
	if(!sensorOn || temperature>(float)(eeprom1->HIGHTEMP))
		return true;
	else
	{
		tempAlarmRaised=false;
		return false;
	}
}

void TEMP::operateOnQuery()
{
	gotQuery=false;
	if(checkTempLimitReached())
		spi1->sendData(A_INVALIDTEMP);
	else
		spi1->sendData(A_VALIDTEMP);
}

void TEMP::operateOnEvent()
{
	if(!tempAlarmRaised)
	{
		tempAlarmRaised=true;
		spi1->sendData(EVENT_HIGHTEMP);
	}
}

void TEMP::init()
{
	if(sensorOn)
		requestTemp();
}

void TEMP::update()
{
	if(!eeprom1->machineOn)
		tempAlarmRaised=false;

	if(!sensorOn && gotQuery)
		spi1->sendData(A_INVALIDTEMP);

	if(tempElligible() || gotQuery)
	{
		requestTemp();
	}
	
	if(getTempElligible())
	{
		getTemp();
		if(gotQuery)
			operateOnQuery();
		else
		{
			if(checkTempLimitReached() && eeprom1->machineOn)
			{
				operateOnEvent();
			}
		}
	}
}