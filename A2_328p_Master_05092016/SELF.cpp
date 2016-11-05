//#include "Defintions.h"
#include <Arduino.h>
#include "SELF.h"
/*
  Gets the Max RPM From the EEPROM Class
*/
#ifndef disable_debug
  #ifdef software_SIM
    SELF::SELF(HardwareSerial *serial)
    {
      _NSerial = serial;
      _NSerial->begin(19200);
      anotherConstructor();
    }
  #else
    SELF::SELF(SoftwareSerial *serial)
    {
      _NSerial = serial;
      _NSerial->begin(19200);
      anotherConstructor();
    }
  #endif
#else
  SELF::SELF()
  {
    anotherConstructor();
  }
#endif

void SELF::anotherConstructor()
{
  pinMode(REL_SELF, OUTPUT);
  digitalWrite(REL_SELF,HIGH);
  
  pinMode(PIN_COMPRESS, OUTPUT);
  digitalWrite(PIN_COMPRESS,LOW);
  pinMode(PIN_DECOMPRESS, OUTPUT);
  digitalWrite(PIN_DECOMPRESS,LOW);
  pinMode(PIN_ALTERNATOR,OUTPUT);
  digitalWrite(PIN_ALTERNATOR,LOW);
  pinMode(PIN_BUZZER,OUTPUT);
  digitalWrite(PIN_BUZZER,LOW);

  firedRPMEvent=false;

  turnMachineOff();
  turnSignalOff();
   
  selfOn = false;
  triggerSelfOn = false;

  didDecompress = false;
  compressing=false;
  decompressing=false;
  queryAgain=false;
  operating=false;
  operateTime=0;
}

void SELF::setSPI(masterSPI *spi1)
{
  this->spi1 = spi1;
}

void SELF::setEEPROM(S_EEPROM* e1)
{
    eeprom1=e1;
}

bool SELF::stopMotorElligible()
{
    return(operating && ((millis()-operateTime)>(unsigned long)MOTORRUNTIME));//(operateWaitTime*100)));
}

void SELF::stopMotor()
{
  operating=false;
  digitalWrite(PIN_COMPRESS,LOW);
  digitalWrite(PIN_DECOMPRESS,LOW);
  compressing=false;
  decompressing=false;
  tryingDecompression=false;
  turnSignalOff();
  #ifndef disable_debug
    _NSerial->println("Motor Off");
  #endif
}

void SELF::compress()
{
  if(!operating)
  {
    turnSignalOn(true,true);     
    operateTime=millis();
    operating=true;
    compressing = true;
   
    digitalWrite(PIN_COMPRESS, HIGH);
    digitalWrite(PIN_DECOMPRESS,LOW);
    turnMachineOn();
    #ifndef disable_debug
      _NSerial->println("CING");
    #endif
  }
}

void SELF::decompress()
{
  if(!operating)
  {
    turnSignalOn(true,false);
    operateTime=millis();
    operating=true;
    decompressing=true;

    digitalWrite(PIN_DECOMPRESS, HIGH);
    digitalWrite(PIN_COMPRESS,LOW);
    turnMachineOff();
    didDecompress = true;
    #ifndef disable_debug
      _NSerial->print("D");
      _NSerial->println("CING");
    #endif
  }
}

void SELF::triggerTryDecompress()
{
  tryingDecompression=true;
  #ifndef disable_debug
    _NSerial->println("Try DEC");
  #endif
  decompress();
}

void SELF::stopTryingDecompressing(bool decompressed)
{
  stopMotor();
  if(decompressed)
    returnSelfCommandStatus('D');
  else
    returnSelfCommandStatus('L');
}

void SELF::operateOnLimitEvent()
{
  if(operating)
  {
    #ifndef disable_debug
      _NSerial->print("LE");
    #endif

    if(decompressing)
    {
      if(tryingDecompression)
        {
          stopTryingDecompressing(true);
          startSelfCountDown();
        }
      else
        stopMotor();
    }
    else if(compressing)
      stopMotor();
  }
}

void SELF::noRPM()
{
  turnMachineOff();
  #ifndef disable_debug
    _NSerial->println("M OFF");
  #endif

  if (didDecompress)
  {
    MachineSwitchedOff();
    didDecompress = false;
  }
}

void SELF::operateOnHighRPMEvent()
{
    if(!firedRPMEvent)
    {
      #ifndef disable_debug
        _NSerial->println("HIGHRPM");
      #endif
      RPMChange();
      firedRPMEvent=true;
    }
}

void SELF::operateOnCOMPRPM()
{
  if(!eeprom1->FORCESTART && selfOn)
  {
    stopSelf();
  #ifndef disable_debug
    _NSerial->println("Self Off.");
  #endif
  }
  compress();
}

void SELF::operateOnDECOMPRPM()
{
  decompress();
}

void SELF::operateOnEvent()
{
  eventOccured=false;
  switch(eventByte)
  {
    case EVENT_LIMITDECOMP:
    case EVENT_LIMITCOMP:
      operateOnLimitEvent();
    break;

    case EVENT_NORPM:
      noRPM();
    break;

    case EVENT_HIGHRPM:
      operateOnHighRPMEvent();
    break;

    case EVENT_COMPRPM:
      operateOnCOMPRPM();
    break;

    case EVENT_DECOMPRPM:
      operateOnDECOMPRPM();
    break;

    case EVENT_FORCESTARTED:
      stopSelf();
    break;

    case EVENT_STARTRPM:
      digitalWrite(PIN_ALTERNATOR,HIGH);
      MachineSwitchedOn();
    break;
  }
}

void SELF::triggerSelf()
{
  spi1->queryState((byte)Q_STARTSEQUENCE,&selfStartData,&selfStartDataReceived);
}

void SELF::operateOnSelfStartData()
{
  selfStartDataReceived=false;
  switch(selfStartData)
  {
    case A_STARTYES:
      startSelfCountDown();
      break;
    case A_STARTON:
      returnSelfCommandStatus('O'); //machine already on
      break;
    case A_STARTTEMP:
      returnSelfCommandStatus('T');
      break;
    case A_STARTCOMPRESS:
      triggerTryDecompress();
      break;
  }
}

void SELF::startSelfCountDown()
{
  triggerSelfOn = true;
  selfWaitTime = millis();
  digitalWrite(PIN_BUZZER,HIGH);  
  returnSelfCommandStatus('D');
}

bool SELF::startSelfElligible()
{
  return (triggerSelfOn && (millis() - selfWaitTime) >= SELFONDELAY);//(selfOnTime * 100));
}

void SELF::startSelf()
{
  digitalWrite(PIN_BUZZER,LOW);
  spi1->inform(I_SELFSTARTED);
  triggerSelfOn = false;
  selfOn = true;
  selfWaitTime = millis();
  digitalWrite(REL_SELF, LOW);
  //startingSelf();
}

bool SELF::stopSelfElligible()
{
  return (selfOn && ((millis() - selfWaitTime) >= SELFRUNTIME));//(selfOffTime * 100)));
}

void SELF::stopSelf()
{
  selfOn = false;
  digitalWrite(REL_SELF, HIGH);
  spi1->inform(I_SELFSTOPPED);
}

void SELF::setCallBackFunctions(void(*funcRPMChange)(), void(*funcMachineOff)(),void (*funcMachineOn)(),void (*returnSelfCommandStatus)(char))
{
  RPMChange = *funcRPMChange;
  MachineSwitchedOff = *funcMachineOff;
  MachineSwitchedOn = *funcMachineOn;
  //startingSelf=*funcStartSelf;
  //stoppingSelf=*funcStopSelf; 
  this->returnSelfCommandStatus=*returnSelfCommandStatus;
}

void SELF::turnMachineOn()
{
  //startCnt=0;
  machineOn = true;
  eeprom1->machineOn=true;  
}

void SELF::turnMachineOff()
{
  firedRPMEvent=false;
  machineOn = false;
  eeprom1->machineOn=false;
  digitalWrite(PIN_ALTERNATOR,LOW);
}

bool SELF::getMachineStatus()
{
  return machineOn;
}

void SELF::turnSignalOn(bool customInform=false,bool compressing=false)
{
  if(customInform)
  {
    if(compressing)
      spi1->inform(I_LIMITCOMPRESS);
    else
      spi1->inform(I_LIMITDECOMPRESS);
    return;
  }
  spi1->inform(I_LIMITPON);

}

void SELF::turnSignalOff()
{
  spi1->inform(I_LIMITPOFF);
}

//void SELF::discardRPMEvent()
//{
//  firedRPMEvent=false;
//}

void SELF::queryMachineState()
{
  queryAgain=!spi1->queryState((byte)Q_MACHINE,&statusData,&statusDataReceived);
}

void SELF::operateOnMachineStatusData()
{
  statusDataReceived=false;
  if(statusData==A_MACHINEON)
    turnMachineOn();
  else
    turnMachineOff();
}

void SELF::update()
{

  if(eventOccured)
    operateOnEvent();

  if (stopSelfElligible())
    stopSelf();

  if (startSelfElligible())
    startSelf();

  if(statusDataReceived)
    operateOnMachineStatusData();

  if(selfStartDataReceived)
    operateOnSelfStartData();


  if(stopMotorElligible())
  {
    #ifndef disable_debug
      _NSerial->print("T");
    #endif
    if(tryingDecompression)
      stopTryingDecompressing(false);
    else
      stopMotor();
  }



}
