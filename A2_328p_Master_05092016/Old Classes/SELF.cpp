#include "SELF.h"
#include <Arduino.h>

SELF::SELF(byte SRPM, byte SELFPIN, byte COMPPIN, byte DECOMPPIN, byte PWRPIN, byte SCOMPIN, byte SDCOMPIN,byte ALTERPIN, int compRPM, int decompRPM, int SOnTime, int SOffTime, HardwareSerial *serial)
{
  tRPM = 0;
  aCNT = 0;
  CRPM = compRPM;
  DRPM = decompRPM;

  SEN_RPM = SRPM;
  REL_SELF = SELFPIN;
  REL_COMPRESS = COMPPIN;
  REL_DECOMPRESS = DECOMPPIN;
  PWR_CMOTOR = PWRPIN;
  SIG_COMPRESS = SCOMPIN;
  SIG_DECOMPRESS = SDCOMPIN;
  REL_ALTERNATOR=ALTERPIN;

  pinMode(REL_SELF, OUTPUT);
  pinMode(REL_COMPRESS, OUTPUT);
  pinMode(REL_DECOMPRESS, OUTPUT);
  pinMode(REL_ALTERNATOR,OUTPUT);
  
  pinMode(PWR_CMOTOR, OUTPUT);
  pinMode(SIG_COMPRESS, INPUT_PULLUP);
  pinMode(SIG_DECOMPRESS, INPUT_PULLUP);
  

  //pinMode(SEN_RPM, INPUT_PULLUP);

  digitalWrite(REL_COMPRESS,HIGH);
  digitalWrite(REL_DECOMPRESS,HIGH);
  digitalWrite(REL_SELF,HIGH);
  digitalWrite(REL_ALTERNATOR,HIGH);
  
  HRPMCnt=0;
  firedRPMEvent=false;

  turnMachineOff();
  turnSignalOff();
   
  gotTrigger = false;

  selfOnTime = SOnTime;
  selfOffTime = SOffTime;
  selfOn = false;
  triggerSelfOn = false;

  averageTime = 300;
  averageWaitTime = 0;
  //signalWaitTime = 10;
  //signalWait = 0;

  RPM = 0;
  lastrise = 0;
  currentrise = 0;

  motorCompressWaitTime = 1; //x100
  motorCompressWait = 0;
  waitForCompression = false;

  didDecompress = false;
  didCompress=false;

  sCompress=false;
  sDecompress=false;

  compressing=false;
  decompressing=false;
  switchChanged=false;
  limitSwitchWaitTime=50;   //without x100
  operateWaitTime = 15;    //x100
  switchChangedOffTime = 9; //x100

  ignoreAvgReadings = false;

  void(*RPMChange)();
  void(*MachineSwitchedOff)();
  _NSerial = serial;
  _NSerial->begin(115200);
}

void SELF::IVR_RPM()
{
  double tempRPM = 0;

  double crise,lrise;
  noInterrupts();
  crise=currentrise;
  lrise=lastrise;
  interrupts();  
  if (crise != 0 && lrise != 0 )//&& !(motorCompression || motorDecompression)
  {
    RPM = 60000.0 / (crise - lrise );
    tempRPM = RPM;
    
    if(didCompress)
    {
      if(tempRPM>CRPM && startCnt<20)
        startCnt++;
      else if(startCnt>=20)
      {
        didCompress=false;
        MachineSwitchedOn();
      }
    }

    if (!firedRPMEvent && !ignoreAvgReadings && ARPM != 0)
      {
        if(tempRPM > (ARPM + 100))
        {
          HRPMCnt++;
          if(HRPMCnt>10)
          {
            HRPMCnt=0;
            firedRPMEvent=true;
            RPMChange();
          }
        }
        else
          HRPMCnt=0;
      }

    tRPM = tRPM + tempRPM;
    aCNT++;

    if (getMachineStatus() == true)// machineOn==true)
    {
      if (tempRPM <= DRPM)
      {
        decompress();
      }
    }
    else if (getMachineStatus() == false)
    {
      if (tempRPM >= CRPM)
      {
        if (selfOn)
        {
          stopSelf();
          _NSerial->println("Self Off.");
        }
        //turnMachineOn();
        //compress();
        waitForCompression = true;
        motorCompressWait = millis();
      }
    }
    _NSerial->println(tempRPM);
  }
}

void SELF::IVR_SCOMPRESS()
{
  if(digitalRead(SIG_COMPRESS)==LOW)
  {  
    limitSwitchOperated=true;
    limitSwitchOperatedAt=millis();
  }
}

void SELF::IVR_SDECOMPRESS()
{
  if(digitalRead(SIG_DECOMPRESS)==LOW)
  {  
    limitSwitchOperated=true;
    limitSwitchOperatedAt=millis();
  }
}

void SELF::setEEPROM(S_EEPROM* e1);
{
    eeprom1=e1;
    //eeprom1->loadAllData();
}

bool SELF::isDecompressed()
{
  return (!digitalRead(SIG_DECOMPRESS));
}

bool SELF::isCompressed()
{
  return (!digitalRead(SIG_COMPRESS));
}

bool SELF::stopMotorElligible()
{
  return(operating && (millis()-operateTime>(operateWaitTime*100) || (switchChanged && millis()-tempSwitchTime>(switchChangedOffTime*100))));
}

void SELF::stopMotor()
{
  operating=false;
  digitalWrite(REL_COMPRESS,HIGH);
  digitalWrite(REL_DECOMPRESS,HIGH);

  compressing=false;
  decompressing=false;
  switchChanged=false;

  turnSignalOff();

  Serial.println("Motor Off..");
}

void SELF::compress()
{
  if(!operating)
  {
    turnSignalOn();    
    operating=true;
    compressing = true;
   
    digitalWrite(REL_COMPRESS, LOW);
    digitalWrite(REL_DECOMPRESS,HIGH);
    operateTime=millis();
    
    didCompress=true;
    turnMachineOn();
    averageWaitTime = millis();
    ignoreAverageReadings();  
  }
}

void SELF::decompress()
{

  if(!operating)
  {
    turnSignalOn();
    operating=true;
    decompressing=true;

    digitalWrite(REL_DECOMPRESS, LOW);
    digitalWrite(REL_COMPRESS,HIGH);
    operateTime=millis();

    turnMachineOff();
    didDecompress = true;
  }
}

bool SELF::tryDecompressing()
{
  bool ret;
  unsigned long t = millis();
  decompress();
  while (millis() - t < 1000)
  {
    if(limitSwitchReadingElligible() && isDecompressed())
    {
      break;
    }
  } 
  ret=isDecompressed();
  stopMotor();

  _NSerial->print("Decompress Done ? = ");
  _NSerial->println(ret);
  return ret;
}

bool SELF::startSelfElligible()
{
  return (triggerSelfOn && (millis() - selfWaitTime) >= (selfOnTime * 100));
}

void SELF::startSelf()
{
  triggerSelfOn = false;
  selfOn = true;
  selfWaitTime = millis();
  digitalWrite(REL_SELF, LOW);
  ignoreAverageReadings();
  startingSelf();
}

bool SELF::stopSelfElligible()
{
  return (selfOn && ((millis() - selfWaitTime) >= (selfOffTime * 100)));
}

void SELF::stopSelf()
{
  selfOn = false;
  digitalWrite(REL_SELF, HIGH);
  stoppingSelf();
  _NSerial->print("Self Run For : ");
  _NSerial->println(millis()-selfWaitTime);  
}

void SELF::checkNoRPM()
{
  unsigned long temp,cTime;
  double tempRPM;

  //noInterrupts();
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

  //interrupts();

  if (temp!=0 && (cTime - temp)>= 3000)
  {
    if (tempRPM != 0)
    {
    _NSerial->print("current rise : ");
    _NSerial->println(temp);
    _NSerial->print("millis() : ");
    _NSerial->println(cTime);
    _NSerial->print("Value : ");
    _NSerial->println(cTime-temp);
      RPM = 0;
      turnMachineOff();
      if (didDecompress)
      {
      // the machine has been stopped
        MachineSwitchedOff();
        didDecompress = false;
      }
    }
  }
}

bool SELF::triggerSelf()
{
  double tempRPM;

  tempRPM = RPM;
  if (getMachineStatus() == true || tempRPM > 0)
  {
    return false;
  }

  turnSignalOn();
  while(millis()-tempSignalTime<50)
  {  }

  if(!isDecompressed())
  {
    _NSerial->println("Not in Decompression.. Trying to Decompress");
    if(!tryDecompressing())
    {
      _NSerial->println("Cannot Decompress");
      return false;
    }
  }

  _NSerial->println("Done Self is going to Start..");
  turnSignalOff();
  triggerSelfOn = true;
  selfWaitTime = millis();
  return true;

}

bool SELF::averageRPMElligible()
{
  return (getMachineStatus() == true && (millis() - averageWaitTime) >= ((unsigned long)averageTime * 100));
}

void SELF::averageRPM()
{
  double temp = ARPM;

  if (tRPM != 0)
  {
    ARPM = tRPM / aCNT;
    tRPM = 0;
    aCNT = 0;
  }
  else
    ARPM = 0;

  _NSerial->print("Average RPM : ");
  _NSerial->println(ARPM);

  if (ignoreAvgReadings && ignoredReadingsCnt < 2)
    ignoredReadingsCnt++;
  else
  {
    ignoreAvgReadings = false;
    ignoredReadingsCnt = 0;
  }
  averageWaitTime = millis();
}

void SELF::setCallBackFunctions(void(*funcRPMChange)(), void(*funcMachineOff)(),void (*funcMachineOn)(),void (*funcStartSelf)(),void (*funcStopSelf)())
{
  RPMChange = *funcRPMChange;
  MachineSwitchedOff = *funcMachineOff;
  MachineSwitchedOn = *funcMachineOn;
  startingSelf=*funcStartSelf;
  stoppingSelf=*funcStopSelf; 
}

bool SELF::setMachineStatus()
{
  double tempRPM;
  tempRPM = RPM;

  if(!selfOn)
  {
    turnSignalOn();
    while(millis()-tempSignalTime<50)
    {}
    if (tempRPM > DRPM && isCompressed())
    {
      turnSignalOff();
      turnMachineOn();
      return true;
    }
    else if (tempRPM > 0 && tempRPM < DRPM && isDecompressed())
    {
      turnSignalOff();
      turnMachineOff();
      return false;
    }
  }
}

double SELF::getRPM()
{
  double tempRPM;
  tempRPM = RPM;
  return tempRPM;
}

double SELF::getAverageRPM()
{
  return ARPM;
}

void SELF::turnMachineOn()
{
  startCnt=0;
  machineOn = true;
  digitalWrite(REL_ALTERNATOR,LOW);
}

void SELF::turnMachineOff()
{
  startCnt=0;
  didCompress=false;
  machineOn = false;
  digitalWrite(REL_ALTERNATOR,HIGH);
}

bool SELF::getMachineStatus()
{
  return machineOn;
}

bool SELF::limitSwitchReadingElligible()
{
  return (signalOn && limitSwitchOperated && millis()-limitSwitchOperatedAt>(limitSwitchWaitTime));
}

void SELF::turnSignalOn()
{
  limitSwitchOperated=false;
  signalOn=true;  
  tempSignalTime=millis();
  digitalWrite(PWR_CMOTOR, LOW);
}

void SELF::turnSignalOff()
{
  signalOn=false;
  limitSwitchOperated=false;
  digitalWrite(PWR_CMOTOR, HIGH);
}

void SELF::ignoreAverageReadings()
{
  ignoreAvgReadings = true;
  ignoredReadingsCnt = 0;
  averageWaitTime = millis();
}

void SELF::discardRPMEvent()
{
  firedRPMEvent=false;
  ignoreAverageReadings();
  ignoredReadingsCnt=1;
  //  averageWaitTime=0;
}

void SELF::update()
{
  if (gotTrigger)
  {
    IVR_RPM();
    gotTrigger = false;
  }

  if(sCompress)
  {
    IVR_SCOMPRESS();
    sCompress=false;
  }
  if(sDecompress)
  {
    IVR_SDECOMPRESS();
    sDecompress=false;
  }

  if(operating && limitSwitchReadingElligible())
  {
      if((decompressing && digitalRead(SIG_DECOMPRESS)==LOW )|| (compressing && digitalRead(SIG_COMPRESS)==LOW))
        {
        switchChanged=true;
        tempSwitchTime=millis();
        }
  }

  if(stopMotorElligible())
      stopMotor();


  if (waitForCompression)
  {
    if (millis() - motorCompressWait > (motorCompressWaitTime * 100))
    {
      waitForCompression = false;
      motorCompressWait = 0;
      compress();
    }
  }

  checkNoRPM();

  if (averageRPMElligible())
  {
    _NSerial->println("AVG ELLIGIBLE");
    if (ignoreAvgReadings && ignoredReadingsCnt < 2)
    {
      tRPM = 0;
      aCNT = 0;
      ignoredReadingsCnt++;
    }
    else
    {
      ignoreAvgReadings = false;
      ignoredReadingsCnt = 0;
      averageRPM();
    }
  }

  if (stopSelfElligible())
    stopSelf();

  if (startSelfElligible())
    startSelf();
}
