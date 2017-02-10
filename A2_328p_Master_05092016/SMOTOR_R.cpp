//#include "Defintions.h"


//old motor settings : MOTOR LOW 65 MOTOR HIGH:348 
// old motor works inverse i.e. achieves motor low when accelerator increases and motor high when accelerator decreases

//new motor settings : MOTOR LOW 930 MOTOR HIGH 370
//new motor works straight i.e. achieces motor low when accel dec and motor high when accel inc

//new motor with resistor changed : when motor is full out , that is accel is decreased, voltage decrease to 50
//									when motor is full in, that is accel is inc, voltage inc to 400

#include <Arduino.h>
#include "SMOTOR_R.h"

#ifndef disable_debug
	#ifdef software_SIM
		SMOTOR_R::SMOTOR_R(HardwareSerial *serial,void (*inc)(),void (*dec)(),void (*mOperated)(bool),void (*MnotSwitchOff)(),void(*retStatus)(bool))
		{
			_Serial=serial;
			anotherConstructor(inc,dec,mOperated,MnotSwitchOff,retStatus);
		}
	#else
		SMOTOR_R::SMOTOR_R(SoftwareSerial *serial,void (*inc)(),void (*dec)(),void (*mOperated)(bool),void (*MnotSwitchOff)(),void(*retStatus)(bool))
		{
			_Serial=serial;
			anotherConstructor(inc,dec,mOperated,MnotSwitchOff,retStatus);
		}
	#endif

#else
	SMOTOR_R::SMOTOR_R(void (*inc)(),void (*dec)(),void (*mOperated)(bool),void (*MnotSwitchOff)(),void(*retStatus)(bool))
	{
		anotherConstructor(inc,dec,mOperated,MnotSwitchOff,retStatus);
	}

#endif

void SMOTOR_R::anotherConstructor(void (*inc)(),void (*dec)(),void (*mOperated)(bool),void (*MnotSwitchOff)(),void(*retStatus)(bool))
{
	pinMode(PIN_SLOW,OUTPUT);
	pinMode(PIN_FAST,OUTPUT);

	digitalWrite(PIN_SLOW,LOW);
	digitalWrite(PIN_FAST,LOW);
	
	waitTime=250;//x100
	switchOffWaitTime=600;
	speedWaitTime=6;
	currentOperation='N';

	selfOperating=false;

	finc=*inc;
	fdec=*dec;
	fMachineNotSwitchedOff=*MnotSwitchOff;
	fMotorOperated=*mOperated;
	returnCommandStatus=*retStatus;

	operating=false;
	backingOff=false;
	operationPerformed=false;
	
	spiDataReceived=false;
	queryAgain=false;
	eventOccured=false;
}

bool SMOTOR_R::queryMotorState()
{
	return spi1->queryState((byte)Q_MOTOR,&spiData,&spiDataReceived); 
}


void SMOTOR_R::triggerDecreaseRPM(byte steps)
{
	queryAgain=!queryMotorState();
	actionTriggered='D';
	this->steps=steps;
}

void SMOTOR_R::triggerSwitchOff()
{
	queryAgain=!queryMotorState();
	actionTriggered='S';
}

void SMOTOR_R::triggerIncreaseRPM(byte steps)
{
	queryAgain=!queryMotorState();
	actionTriggered='I';
	this->steps=steps;
}

void SMOTOR_R::increaseRPM()
{
	if(spiData!=A_MOTORHIGH)
	{
		if(operating || selfOperating)
		{
			returnCommandStatus(false);
			return;
		}
		currentOperation='I';
		spi1->inform(I_MOTORRPON);
		fMotorOperated(true);
		digitalWrite(PIN_FAST,LOW);
		digitalWrite(PIN_SLOW,HIGH);
		speedWait=millis();
		operating=true;	
		returnCommandStatus(true);
	}
	else
	{
		#ifndef disable_debug
			_Serial->println("Motor limit");
		#endif
		returnCommandStatus(false);
	}
}

void SMOTOR_R::setSPI(masterSPI *spi1)
{
	this->spi1=spi1;
}

void SMOTOR_R::decreaseRPM()
{	
	if(spiData!=A_MOTORLOW)
	{	
		if(operating || selfOperating)
		{
			returnCommandStatus(false);
			return;
		}
		currentOperation='D';
		spi1->inform(I_MOTORRPON);
		fMotorOperated(true);
		digitalWrite(PIN_SLOW,LOW);
		digitalWrite(PIN_FAST,HIGH);
		speedWait=millis();
		operating=true;
		returnCommandStatus(true);
	}
	else
	{
		#ifndef disable_debug
			_Serial->println("Motor limit");
		#endif
		returnCommandStatus(false);
	}
}

void SMOTOR_R::switchOff()
{
	if(spiData!=A_MOTORLOW)
	{
		if(operating || selfOperating)
		{
			returnCommandStatus(false);
			return;
		}
		currentOperation='S';
		spi1->inform(I_MOTORSWOFF);
		fMotorOperated(true);
		digitalWrite(PIN_SLOW,LOW);
		digitalWrite(PIN_FAST,HIGH);
		operating=true;
		returnCommandStatus(true);
		//return true;
	}
	else
	{	
		#ifndef disable_debug
		_Serial->println("Motor limit");
		#endif
		returnCommandStatus(false);
	}
		//return false;
}


void SMOTOR_R::stopOperating()
{
	digitalWrite(PIN_SLOW,HIGH);
	digitalWrite(PIN_FAST,HIGH);
	spi1->inform(I_MOTORRPOFF);
	fMotorOperated(false);
	operating=false;
	wait=millis();
	operationPerformed=true;
}

bool SMOTOR_R::operateOnEvent()
{
	if(eventOccured)
	{
		#ifndef disable_debug
			_Serial->print("Got Event");
			_Serial->print(" SMOTOR");
		#endif

		eventOccured=false;
		byte r=eventByte;
		if(backingOff)
		{
			if(currentOperation=='B' && (r==EVENT_MOTORPREVIOUSSTATE || r==EVENT_MOTORHIGH))
				return true;
		}
		else if(operating)
		{
			if((r==EVENT_MOTORHIGH && currentOperation=='I') || (r==EVENT_MOTORLOW && (currentOperation=='D' || currentOperation=='S')))
				return true;
		}
	}
	return false;
}

bool SMOTOR_R::operatingTimeOver()
{
	if(operating)
	{
		if(currentOperation!='S' && millis()-speedWait>=((speedWaitTime*steps)*100))
			{
				#ifndef disable_debug
					_Serial->println("TIME LIMIT");
				#endif
					return true;
			}
	}
	return false;
}

void SMOTOR_R::stopBackOff()
{
	currentOperation='N';
	digitalWrite(PIN_SLOW,HIGH);
	digitalWrite(PIN_FAST,HIGH);
	spi1->inform(I_MOTORRPOFF);
	backingOff=false;
	fMotorOperated(false);
}

void SMOTOR_R::backOff()
{
	operationPerformed=false;
	currentOperation='B';
	spi1->inform(I_MOTORBOFF);
	digitalWrite(PIN_SLOW,HIGH);
	digitalWrite(PIN_FAST,LOW);
	backingOff=true;
	fMotorOperated(true);

}
bool SMOTOR_R::makeResponseElligible()
{
	if(operationPerformed)
	{
		if(currentOperation=='S')
		{
			return ((millis()-wait)>=(switchOffWaitTime*100));
		}
		else
			return ((millis()-wait)>=(waitTime*100));	
	}
}

void SMOTOR_R::makeResponse()
{
	if(currentOperation=='I')
	{
		finc();
	}
	else if(currentOperation=='D')
	{
		fdec();
	}
	else if(currentOperation=='S')
	{
		fMachineNotSwitchedOff();
	}

	operationPerformed=false;
}

char SMOTOR_R::checkCurrentOperation()
{
	if(operating)
	{
		return currentOperation;
	}
	if(backingOff)
		return 'B';
	else return 'N';
}

void SMOTOR_R::setSelfOperationStatus(bool temp)
{
	selfOperating=temp;
	stopOperating();
}

void SMOTOR_R::operateOnSPIData()
{
		#ifndef disable_debug
			_Serial->print("SPI DATA:");
			_Serial->println(spiData);
		#endif
			spiDataReceived=false;
			if(actionTriggered=='I')
				increaseRPM();
			else if(actionTriggered=='D')
				decreaseRPM();
			else if(actionTriggered=='S')
				switchOff();	
}

void SMOTOR_R::update()
{
		if(queryAgain)
			queryAgain=queryMotorState();

		if(spiDataReceived)
			operateOnSPIData();

		if(operatingTimeOver() || operateOnEvent())
		{
			if(backingOff)	stopBackOff();
			else 			stopOperating();
		}	

		if(makeResponseElligible())
			makeResponse();

}