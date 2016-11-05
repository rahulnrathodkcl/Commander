#include "SMOTOR.h"

SMOTOR::SMOTOR(byte SLOWPIN,byte FASTPIN,byte PWRPIN)
{
	REL_SLOW=SLOWPIN;
	REL_FAST=FASTPIN;
	PWR_LIM=PWRPIN;

	waitTime=250;//x100

	speedWaitTime=4;
	offWaitTime=400;

	operating=false;
	operationPerformed=false;
}

void SMOTOR::setCallBackFunctions(void (*inc)(),void (*dec)(),void (*off)())
{
	finc=*inc;
	fdec=*dec;
	foff=*off;
}

void SMOTOR::increaseRPM()
{
	currentOperation='I';
	digitalWrite(PWR_LIM,HIGH);
	digitalWrite(REL_FAST,HIGH);
	digitalWrite(REL_SLOW,LOW);	
	speedWait=millis();
	operating=true;
}

void SMOTOR::decreaseRPM()
{	
	currentOperation='D';
	digitalWrite(PWR_LIM,HIGH);
	digitalWrite(REL_SLOW,HIGH);
	digitalWrite(REL_FAST,LOW);
	speedWait=millis();
	operating=true;
}

void SMOTOR::switchOff()
{
	currentOperation='S';
	digitalWrite(PWR_LIM,HIGH);
	digitalWrite(REL_SLOW,HIGH);
	digitalWrite(REL_FAST,LOW);
	offWait=millis();
	operating=true;
}

void SMOTOR::stopOperating()
{
	digitalWrite(PWR_LIM,LOW);
	digitalWrite(REL_SLOW,LOW);
	digitalWrite(REL_FAST,LOW);

	if(currentOperation=='S')	operateTime=millis()-offWait;
	
	operating=false;
	wait=millis();
	operationPerformed=true;
}

bool SMOTOR::checkStopOperatingElligible()
{
	if(operating)
	{
		if(currentOperation=='S')
			return (millis()-offWait>=(offWaitTime*100));
		else
			return (millis()-speedWait>=(speedWaitTime*100));
	}	
	return false;
}

bool SMOTOR::stopBackingOffElligible()
{
	if(abnormalStop)
		return (backingOff && millis()-backOffWait>=operateTime);
	else
		return (backingOff && millis()-backOffWait>=(offWaitTime*100));
}

void SMOTOR::stopBackOff()
{
	digitalWrite(PWR_LIM,LOW);
	digitalWrite(REL_SLOW,LOW);
	digitalWrite(REL_FAST,LOW);
	backingOff=false;
	abnormalStop=false;
}

void SMOTOR::backOff()
{
	digitalWrite(PWR_LIM,HIGH);
	digitalWrite(REL_SLOW,LOW);
	digitalWrite(REL_FAST,HIGH);
	backingOff=true;
	backOffWait=millis();
}

bool SMOTOR::makeResponseElligible()
{
	return (operationPerformed && (millis()-wait)>=(waitTime*100));	
}

void SMOTOR::makeResponse()
{
	if(currentOperation=='S')
	{
		foff();
	}
	else if(currentOperation=='I')
	{
		finc();
	}
	else if(currentOperation=='D')
	{
		fdec();
	}
	operationPerformed=false;
}

char SMOTOR::checkCurrentOperation()
{
	if(operating)
	{
		return currentOperation;
	}
	else return 'N';
}

char SMOTOR::checkLimitSwitch()
{	
	unsigned long t=millis();
	digitalWrite(PWR_LIM,HIGH);
	while(millis()-t<10 && !(reachedSlowEnd || reachedFastEnd))
	{}
		if(reachedSlowEnd)
			return 'S';
		else if(reachedFastEnd)
			return 'F'; 
		else 
			return 'N';
}

void SMOTOR::update()
{
		if(checkStopOperatingElligible())
			stopOperating();

		if(makeResponseElligible())
			makeResponse();

		if(stopBackingOffElligible())
			stopBackOff();
		
		if((reachedSlowEnd || reachedFastEnd))
		{
			if(!backingOff)
			{
				stopOperating();
				abnormalStop=true;
			}
			else
			{
				stopBackOff();
			}
		}
}