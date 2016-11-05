#include "SELF.h"
#include <Arduino.h>

/*
  Gets the Max RPM From the EEPROM Class
*/

SELF::SELF(void (*sMotorStatus)(bool),void (*funcSelfStart)(),void (*funSelfStop)())
{
  sendMotorStatus=*sMotorStatus;
  startingSelf=*funcSelfStart;
  stoppingSelf=*funSelfStop;

  pinMode(PIN_CMOTORPWR, OUTPUT);
  pinMode(PIN_DECOMP, INPUT);
  pinMode(PIN_RPMSEN,INPUT_PULLUP);
  
  digitalWrite(PIN_CMOTORPWR,LOW);
  
  HRPMCnt=0;
  firedRPMEvent=false;

  turnMachineOff();
  turnSignalOff();
   
  gotTrigger = false;

  selfOn = false;

  RPM = 0;
  lastrise = 0;
  currentrise = 0;

  didCompress=false;
  reportedForceStart=false;

  compressing=false;
  decompressing=false;

  reportedDECOMPRPM=false;
  reportedCOMPRPM=false;

  compSwitchChangedOffTime = 9; //x100
  decompSwitchChangedOffTime = 1; //x100

  gotLimitQuery=false;
  gotMachineQuery=false;
  gotStartQuery=false;
}

void SELF::setEEPROM(S_EEPROM* e1)
{
    eeprom1=e1;
}

void SELF::setSPI(slaveSPI *spi1)
{
  this->spi1=spi1;
}

void SELF::limitQuery()
{
    gotLimitQuery=true;
    turnSignalOn();
}

void SELF::machineQuery()
{
    gotMachineQuery=true;
}

void SELF::operateOnLimitQuery(bool decompressed)
{
    turnSignalOff();
    gotLimitQuery=false;
    if(decompressed)
        spi1->sendData(A_LIMITDECOMPRESS);
    else
        spi1->sendData(A_LIMITCOMPRESS);
}

void SELF::operateOnMachineQuery()
{
    gotMachineQuery=false;
    if(machineOn)
        spi1->sendData(A_MACHINEON);
    else
        spi1->sendData(A_MACHINEOFF);
}

void SELF::informSelf(byte spiData)
{
  if(spiData==I_SELFSTARTED)
      selfStarted();
  else
      selfStopped();  
}

void SELF::informLimit(byte spiData)
{
  switch(spiData)
  {
    case I_LIMITDECOMPRESS:
      decompress();
      break;
    case I_LIMITCOMPRESS:
      compress();
      break;

    case I_LIMITPON:
      turnSignalOn();
      break;
    case I_LIMITPOFF:
      turnSignalOff();
    break;
  }
}

void SELF::reportEvent(byte event)
{
    spi1->sendData(event);
}

void SELF::IVR_RPM()
{
  double tempRPM = 0;
  double crise,lrise;
  noInterrupts();
  crise=currentrise;
  lrise=lastrise;
  interrupts();  
  if (crise != 0 && lrise != 0 )
  {
    RPM = 60000.0 / (crise - lrise);
    tempRPM = RPM;
    
    if(didCompress)
    {
      if(!reportedForceStart && eeprom1->FORCESTART && tempRPM>(eeprom1->COMPRPM+20))
      {
        reportedForceStart=true;
        reportEvent(EVENT_FORCESTARTED);
      }

      if(tempRPM>(eeprom1->COMPRPM) && startCnt<20)
        startCnt++;
      else if(startCnt>=20)
      {
        didCompress=false;
        reportEvent(EVENT_STARTRPM);
      }
    }

    if(firedRPMEvent && tempRPM<(eeprom1->RPM))
    {
        HRPMCnt=0;
        firedRPMEvent=false;
    }    

    if(!firedRPMEvent && machineOn)
    {
        if(tempRPM>(eeprom1->RPM))
        {
          if(HRPMCnt<20)
            HRPMCnt++;
          else
          {
            firedRPMEvent=true;
            reportEvent(EVENT_HIGHRPM);
            HRPMCnt=0;
          }
        }
        else
          HRPMCnt=0;
    }
    
    if (getMachineStatus()) //machine Is ON
    {
      if (tempRPM <= (eeprom1->DECOMPRPM) && !reportedDECOMPRPM)
        {
          reportedDECOMPRPM=true;
          reportEvent(EVENT_DECOMPRPM);
        }
    }
    else
    {
      if (!reportedCOMPRPM && tempRPM >= (eeprom1->COMPRPM))
      {
        reportedCOMPRPM=true;
        reportEvent(EVENT_COMPRPM);
      }
    }
    //_NSerial->println(tempRPM);
  }
}

void SELF::IVR_SDECOMPRESS()
{
  //if(digitalRead(PIN_DECOMP)==LOW)
  //{  
  if(isDecompressed())
  {
    limitSwitchOperated=true;
    limitSwitchOperatedAt=millis();
  }
  //}
}


bool SELF::isDecompressed()
{
  return (!digitalRead(PIN_DECOMP));
}

//bool SELF::stopMotorElligible()
//{
//  if(compressing)
//  {
//    return(operating && (millis()-operateTime>(operateWaitTime*100) || (switchChanged && millis()-tempSwitchTime>(compSwitchChangedOffTime*100))));
//  }
//  else 
//  {
//    return(operating && (millis()-operateTime>(operateWaitTime*100) || (switchChanged && millis()-tempSwitchTime>(decompSwitchChangedOffTime*100))));
//  }
//}
//
//void SELF::stopMotor()
//{
//  operating=false;
//  //digitalWrite(REL_COMPRESS,HIGH);
//  //digitalWrite(REL_DECOMPRESS,HIGH);
//
//  compressing=false;
//  decompressing=false;
//  switchChanged=false;
//
//  turnSignalOff();
//
//  Serial.println("Motor Off....");
//}

void SELF::compress()
{
    compressing=true;
    turnSignalOn();    
    didCompress=true;
    turnMachineOn();
    reportedCOMPRPM=false;
}

void SELF::decompress()
{
    decompressing=true;
    turnSignalOn();
    turnMachineOff();
    reportedDECOMPRPM=false;
}

void SELF::selfStarted()
{
  selfOn=true;
  startingSelf();
}

void SELF::selfStopped()
{
  selfOn=false;
  stoppingSelf();
}

void SELF::checkNoRPM()
{
  unsigned long temp,cTime;
  double tempRPM;

  if(gotTrigger)
    return;
  else
  {
    noInterrupts();
    temp = currentrise;
    interrupts();
    tempRPM = RPM;
    cTime = millis();
  }

  if (temp!=0 && (cTime - temp)>= 3000)
  {
    if (tempRPM != 0)
    {
      RPM = 0;
      if(!didCompress)
      {
        reportEvent(EVENT_NORPM);        
      }
      turnMachineOff();
    }
  }
}

void SELF::triggerMachineStatus()
{
  queryingMachineState=true;
  turnSignalOn();
}

void SELF::setMachineStatus()
{
  double tempRPM;
  bool decompressState=isDecompressed();
  queryingMachineState=false;
  
  tempRPM = RPM;
    if (tempRPM > (eeprom1->DECOMPRPM) && !decompressState)
    {
      turnSignalOff();
      turnMachineOn();
    }
    else if (tempRPM > 0 && tempRPM < (eeprom1->DECOMPRPM) && decompressState)
    {
      turnSignalOff();
      turnMachineOff();
    }
}

//bool SELF::setMachineStatus()
//{
//  double tempRPM;
//  tempRPM = RPM;
//
//  if(!selfOn)
//  {
//    turnSignalOn();
//    while(millis()-tempSignalTime<50)
//    {}
//    if (tempRPM > (eeprom1->DECOMPRPM) && !isDecompressed())
//    {
//      turnSignalOff();
//      turnMachineOn();
//      return true;
//    }
//    else if (tempRPM > 0 && tempRPM < (eeprom1->DECOMPRPM) && isDecompressed())
//    {
//      turnSignalOff();
//      turnMachineOff();
//      return false;
//    }
//  }
//}

double SELF::getRPM()
{
  double tempRPM;
  tempRPM = RPM;
  return tempRPM;
}

void SELF::turnMachineOn()
{
  startCnt=0;
  machineOn = true;
  eeprom1->machineOn=true;  
}

void SELF::turnMachineOff()
{
  startCnt=0;
  didCompress=false;
  machineOn = false;
  firedRPMEvent=false;
  reportedCOMPRPM=false;
  reportedDECOMPRPM=false;
  reportedForceStart=false;
  eeprom1->machineOn=false;
}

bool SELF::getMachineStatus()
{
  return machineOn;
}

//bool SELF::limitSwitchReadingElligible()
//{
//  return (signalOn && limitSwitchOperated && millis()-limitSwitchOperatedAt>(limitSwitchWaitTime));
//}

void SELF::turnSignalOn()
{ 
  limitSwitchOperated=false;
  signalOn=true; 
  tempSignalTime=millis();
  reportedLimitCOMPEvent=false;
  reportedLimitDECOMPEvent=false;  
  digitalWrite(PIN_CMOTORPWR, HIGH);
}

void SELF::turnSignalOff()
{
  signalOn=false;
  limitSwitchOperated=false;
  compressing=false;
  decompressing=false;
  reportedLimitCOMPEvent=false;
  reportedLimitDECOMPEvent=false;  
  digitalWrite(PIN_CMOTORPWR, LOW);
}

void SELF::checkAndReportLimitEvent()
{
  if(limitSwitchOperated && signalOn)
  {
    bool decompressState=isDecompressed();
    if(compressing && !decompressState)
    {
      if(!reportedLimitCOMPEvent && millis()-limitSwitchOperatedAt>(compSwitchChangedOffTime*100))
       {
        reportedLimitCOMPEvent=true;
        //_NSerial->println("Comp. Limit");
        reportEvent(EVENT_LIMITCOMP);
       }
    }
    else if(decompressing && decompressState)
    {
      if(!reportedLimitDECOMPEvent && millis()-limitSwitchOperatedAt>(decompSwitchChangedOffTime*100))
      {
        reportedLimitDECOMPEvent=true;
        //_NSerial->println("Decomp. Limit");
        reportEvent(EVENT_LIMITDECOMP);
      }
    }
  }
}

void SELF::startSequenceQuery()
{
    gotStartQuery=true;
    turnSignalOn();
}

void SELF::returnStartQueryAnswer(bool decompressed)
{
  gotStartQuery=false;
  turnSignalOff();
  sendMotorStatus(decompressed);
}

bool SELF::stopSignalElligible()
{
  return(signalOn && millis()-tempSignalTime>=LIMITREADINGTIME);
}

void SELF::update()
{
  if (gotTrigger)
  {
    IVR_RPM();
    gotTrigger = false;
  }

  if(gotLimitQuery)
  {
    if(limitSwitchOperated || stopSignalElligible())
      operateOnLimitQuery(isDecompressed());
  }
  else if(gotStartQuery)
  {
    if(limitSwitchOperated || stopSignalElligible())
      returnStartQueryAnswer(isDecompressed());
  }

  if(queryingMachineState)
  {
      if(stopSignalElligible())
      {
        setMachineStatus();        
      }
  }
  checkAndReportLimitEvent();
  checkNoRPM();
}