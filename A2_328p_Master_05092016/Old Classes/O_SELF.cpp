#include "SELF.h"
#include <Arduino.h>

SELF::SELF(byte SRPM, byte SELFPIN, byte COMPPIN, byte DECOMPPIN, byte PWRPIN, byte SCOMPIN, byte SDCOMPIN, int compRPM, int decompRPM, int SOnTime, int SOffTime, HardwareSerial *serial)
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

  pinMode(REL_SELF, OUTPUT);
  pinMode(REL_COMPRESS, OUTPUT);
  pinMode(REL_DECOMPRESS, OUTPUT);
  pinMode(PWR_CMOTOR, OUTPUT);
  pinMode(SIG_COMPRESS, INPUT_PULLUP);
  pinMode(SIG_DECOMPRESS, INPUT_PULLUP);

  digitalWrite(REL_COMPRESS,HIGH);
  digitalWrite(REL_DECOMPRESS,HIGH);
  digitalWrite(REL_SELF,HIGH);
  
  //Power = true;
  //digitalWrite(PWR_CMOTOR, HIGH);

  cntforoff = 0;
  cntforon = 0;

  firedRPMEvent=false;

  turnMachineOff();
  //machineOn=false;
  cMotorStatus = 'U';
  gotTrigger = false;

  selfOnTime = SOnTime;
  selfOffTime = SOffTime;
  selfOn = false;
  triggerSelfOn = false;

  averageTime = 300;
  averageWaitTime = 0;
  signalWaitTime = 10;
  signalWait = 0;

  RPM = 0;
  lastrise = 0;

  motorCompressWaitTime = 1; //x100
  motorCompressWait = 0;
  waitForCompression = false;

  didDecompress = false;
  throw2Readings = false;
  throwReadingsCnt = 0;

  operatedWaitTime = 7;    //x100
  varStopCompression = false;
  varStopDecompression = false;

  ignoreAvgReadings = false;

  void(*RPMChange)();
  void(*MachineSwitchedOff)();

  currentrise = 0;

  motorCompression = false;
  motorDecompression = false;

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
  /*  if (lastrise == 0)
    {
      return;
      //lastrise = millis();
    }
  */
  if (crise != 0 && lrise != 0 )//&& !(motorCompression || motorDecompression)
  {
    RPM = 60000.0 / (crise - lrise );
    tempRPM = RPM;
    //lastrise = millis();
    //_NSerial->print("RPM:");
    if (!firedRPMEvent && !ignoreAvgReadings && ARPM != 0 && tempRPM > (ARPM + 100) )
      {
        firedRPMEvent=true;
        RPMChange();
      }

    tRPM = tRPM + tempRPM;
    aCNT++;

    if (getMachineStatus() == true)// machineOn==true)
    {
      if (tempRPM <= DRPM)
      {
        //_NSerial->print("Going For Decompression, CURRENT MACHINE STATUS : ");
        //_NSerial->println(getMachineStatus());
        decompress();
      }
    }
    else if (getMachineStatus() == false)
    {
      if (tempRPM >= CRPM)
      {
        //_NSerial->print("Going For Compression, CURRENT MACHINE STATUS : ");
        //_NSerial->println(getMachineStatus());
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
  int inp = digitalRead(SIG_COMPRESS);
  if (inp == 0)
  {
    operatedWait = millis();
    varStopCompression = true;
    cMotorStatus = 'C';
    _NSerial->println("machine in compression...");
  }
}

void SELF::IVR_SDECOMPRESS()
{
  int inp = digitalRead(SIG_DECOMPRESS);
  if (inp == 0)
  {
    operatedWait = millis();
    varStopDecompression = true;
    cMotorStatus = 'D';
    _NSerial->println("machine in decompression");
  }
}

void SELF::stopCompression()
{
  motorCompression = false;
  digitalWrite(REL_COMPRESS, HIGH);
  noInterrupts();
  lastrise = 0;
  currentrise = 0;
  interrupts();
  varStopCompression = false;
}

bool SELF::stopCompressionElligible()
{
  return (varStopCompression && (millis() - operatedWait >= (operatedWaitTime * 100)));
}

void SELF::stopDecompression()
{
  motorDecompression = false;
  digitalWrite(REL_DECOMPRESS, HIGH);
  noInterrupts();
  lastrise = 0;
  currentrise = 0;
  interrupts();
  varStopDecompression = false;
}

bool SELF::stopDecompressionElligible()
{
  return (varStopDecompression && (millis() - operatedWait >= (operatedWaitTime * 100)));
}

void SELF::compress()
{
  turnSignalOn();
  digitalWrite(REL_COMPRESS, LOW);
  turnMachineOn();
  motorCompression = true;
  _NSerial->println("Done Compression");
  averageWaitTime = millis();
  ignoreAverageReadings();
}

void SELF::decompress()
{
  turnSignalOn();
  //_NSerial->println("Starting Decompression...");
  motorDecompression = true;
  digitalWrite(REL_DECOMPRESS, LOW);
  turnMachineOff();
  didDecompress = true;
}

bool SELF::tryDecompressing()
{
  unsigned long t = millis();
  decompress();
  //while(millis()-t<1000 && cMotorStatus!='D')
  while (millis() - t < 1000 && cMotorStatus != 'D')
  {
  }

  _NSerial->print("CURRENT STATUS IN TRY DECOMPRESSING");
  _NSerial->println(cMotorStatus);

  digitalWrite(REL_DECOMPRESS, HIGH);

  if (cMotorStatus == 'U' || cMotorStatus == 'C')
  {
    return false;
  }
  else if (cMotorStatus == 'D')
  {
    return true;
  }
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
    temp = currentrise;
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

char SELF::checkCMotorStatus()
{
  cMotorStatus = 'U';
  turnSignalOn();
  unsigned long t = millis();
  while ((millis() - t) < 1000 && (cMotorStatus != 'D' || cMotorStatus != 'C'))
  {
  }
  return cMotorStatus;
}

bool SELF::triggerSelf()
{
  double tempRPM;

  tempRPM = RPM;
  if (getMachineStatus() == true || tempRPM > 0)
  {
    return false;
  }

  char s = checkCMotorStatus();
  if (s == 'C' || s == 'U')
  {
    if (!tryDecompressing())
    {
      return false;
    }
  }

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

void SELF::setCallBackFunctions(void(*funcRPMChange)(), void(*funcMachineOff)(),void (*funcStartSelf)(),void (*funcStopSelf)())
{
  RPMChange = *funcRPMChange;
  MachineSwitchedOff = *funcMachineOff;
  startingSelf=*funcStartSelf;
  stoppingSelf=*funcStopSelf; 
}

bool SELF::setMachineStatus()
{
  double tempRPM;
  tempRPM = RPM;

  if (!selfOn)
  {
    checkCMotorStatus();
    if (tempRPM > DRPM && (cMotorStatus == 'C' || cMotorStatus == 'U'))
    {
      turnMachineOn();
      //machineOn=true;
      return true;
    }
    else if (tempRPM > 0 && tempRPM < DRPM && cMotorStatus == 'D')
    {
      turnMachineOff();
      //machineOn=false;
      return false;
    }
  }
}

double SELF::getRPM()
{
  double tempRPM;
  noInterrupts();
  tempRPM = RPM;
  interrupts();
  return tempRPM;
}

double SELF::getAverageRPM()
{
  return ARPM;
}

void SELF::turnMachineOn()
{
  machineOn = true;
}

void SELF::turnMachineOff()
{
  machineOn = false;
}

bool SELF::getMachineStatus()
{
  return machineOn;
}

void SELF::turnSignalOn()
{
  digitalWrite(PWR_CMOTOR, LOW);
  signalWait = millis();
  Power = false;
}

void SELF::turnSignalOff()
{
  digitalWrite(PWR_CMOTOR, HIGH);
  signalWait = millis();
  Power = true;
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

  //if(!machineOn)
  //	setMachineStatus();

  if (!Power && (millis() - signalWait) > (signalWaitTime * 100))
  {
    turnSignalOff();
    stopDecompression();
    stopCompression();
  }

  if (stopDecompressionElligible())
    stopDecompression();

  if (stopCompressionElligible())
    stopCompression();

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
