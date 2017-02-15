#include "BATTERY.h"

BATTERY::BATTERY()
{
	pinMode(PIN_BATTERYPWR,OUTPUT);
	digitalWrite(PIN_BATTERYPWR,LOW);
	alarmTrigger=false;
	alarmed=false;
	gotQuery=false;
	selfOn=false;
	lastCheck=0;
}

void BATTERY::setSPI(slaveSPI *spi1)
{
	this->spi1=spi1;
}

void BATTERY::query()
{
	gotQuery=true;
}

void BATTERY::inform(byte informByte)
{
	if(informByte==I_SELFSTARTED)
		selfOn=true;
	else
		selfOn=false;
}

void BATTERY::operateOnQuery()
{
	gotQuery=false;
	batteryLevel=getBatteryLevel();

	if(!checkSufficientLevel() && alarmed==false)
		spi1->sendData(A_BATTERYLOW);
	else
		spi1->sendData(A_BATTERYOK);
}


bool BATTERY::checkSufficientLevel()
{	
	if(!selfOn) //fMotorStatus()=='N')
	{
			if(batteryLevel<=2 || batteryLevel>80)
				return false;
			else if(batteryLevel>3 && alarmed==true)
			{
				alarmed=false;
				alarmTrigger=false;
			}
	}
	return true;
}

byte BATTERY::getBatteryLevel()
{
	float temp;
	unsigned int t;

	digitalWrite(PIN_BATTERYPWR,HIGH);
	analogRead(PIN_BATTERYSEN);
	t=millis();
	while(millis()-t<2)
	{}
	temp=analogRead(PIN_BATTERYSEN);

	digitalWrite(PIN_BATTERYPWR,LOW);
	temp=temp*5.0/1024.0;
	temp=temp*4.030;
	temp=temp-11.20;

	temp=temp*10;
	lastCheck=millis();
	return (byte)temp;
}

void BATTERY::checkInitialBatteryLevel()
{
	checkBatteryLevel();
}

void BATTERY::checkBatteryLevel()
{
	batteryLevel=getBatteryLevel();
	if(!checkSufficientLevel() && !alarmed)
	{
			alarmTrigger=true;
	}
}

void BATTERY::update()
{
	if(gotQuery)
		operateOnQuery();

	if((millis()-lastCheck>180000L) && !selfOn)
		checkBatteryLevel();

	if(alarmTrigger && !alarmed)
	{
		if(spi1->sendData(EVENT_BATTERYLOW))
		{
			alarmed=true;
			alarmTrigger=false;
		}
	}
}