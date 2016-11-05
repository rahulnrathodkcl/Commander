#include "SELF_R.h"
#include <Arduino.h>

SELF_R::SELF_R(byte SRPM, byte SELFPIN, byte COMPPIN, byte DECOMPPIN, byte PWRPIN, byte SMOTORPIN, byte RELALTERNATOR,int compRPM, int decompRPM, int SOnTime, int SOffTime, HardwareSerial *serial,float cvalue,float dvalue)
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
  SIG_MOTOR = SMOTORPIN;
  REL_ALTERNATOR=RELALTERNATOR;
  compressionValue = cvalue;
  decompressionValue = dvalue;

  pinMode(REL_SELF, OUTPUT);
  pinMode(REL_COMPRESS, OUTPUT);
  pinMode(REL_DECOMPRESS, OUTPUT);
  pinMode(PWR_CMOTOR, OUTPUT);
  pinMode(SIG_MOTOR,INPUT);
  pinMode(REL_ALTERNATOR,OUTPUT);
  
  
  digitalWrite(REL_COMPRESS,HIGH);
  digitalWrite(REL_DECOMPRESS,HIGH);
  digitalWrite(REL_SELF,HIGH);
  digitalWrite(REL_ALTERNATOR,HIGH);
  
  cntforoff = 0;
  cntforon = 0;

  firedRPMEvent=false;

  turnMachineOff();
  gotTrigger = false;

  selfOnTime = SOnTime;
  selfOffTime = SOffTime;
  selfOn = false;
  triggerSelfOn = false;

  averageTime = 300;
  averageWaitTime = 0;
  //signalWaitTime = 10;
//  signalWait = 0;

  RPM = 0;
  lastrise = 0;

  motorCompressWaitTime = 1; //x100
  motorCompressWait = 0;
  waitForCompression = false;

  didDecompress = false;


  operateTime=9;//x100
  tempTime=0;
  operating=false;
  compressing=false;
  decompressing=false;
  
  ignoreAvgReadings = false;

  void(*RPMChange)();
  void(*MachineSwitchedOff)();

  currentrise = 0;

  _NSerial = serial;
  _NSerial->begin(115200);
}

void SELF_R::IVR_RPM()
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
    
    if(!firedRPMEvent && !ignoreAvgReadings && ARPM != 0 && tempRPM > (ARPM + 100))
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

void SELF_R::compress()
{
    if(!operating)
    {
      if(!isCompressed())
      {
        tempTime=millis();
        
        operating=true;
        digitalWrite(REL_ALTERNATOR,LOW);
        digitalWrite(REL_COMPRESS,LOW);
        digitalWrite(REL_DECOMPRESS,HIGH);
        compressing=true;
        turnMachineOn();
        _NSerial->print("Compressing At : ..");
        _NSerial->println(millis());
        
        averageWaitTime = millis();
        ignoreAverageReadings();
      }
    }
}

bool SELF_R::isCompressed()
{
  if (getAnalogInput() <= compressionValue)
    return true;
  else
    return false;
}

bool SELF_R::isDecompressed()
{
  if (getAnalogInput() >= decompressionValue)
    return true;
  else
    return false;
}

void SELF_R::stopMotor()
{
  tempTime=0;
  Serial.println("Motor Off..");
  operating = false;
  digitalWrite(REL_COMPRESS, HIGH);
  digitalWrite(REL_DECOMPRESS, HIGH);
  decompressing = false;
  compressing = false;
}

float SELF_R::requestMotorReading()
{
  return getAnalogInput();
}

float SELF_R::getAnalogInput()
{
  unsigned long temp;
  float t2;
  temp = 0;

  digitalWrite(PWR_CMOTOR, HIGH);
  analogRead(SIG_MOTOR);
  for (int i = 0; i < 30; i++)
  {
    temp += analogRead(SIG_MOTOR);
  }

  t2 = temp / 6144.0; //t2=temp/30*5/1024;
  digitalWrite(PWR_CMOTOR, LOW);
  Serial.println(t2);
  return t2;
}

bool SELF_R::timeOut()
{
  return(operating && (millis()-tempTime)>=(operateTime*100));
}

bool SELF_R::checkOperatingElligible()
{
  if (compressing)
  {
    return (!isCompressed());
  }
  else if (decompressing)
  {
    return (!isDecompressed());
  }
  return true;
}

void SELF_R::decompress()
{
    if(!operating)
    {
      if(!isDecompressed())
      {
        tempTime=millis();
        operating=true;      
        digitalWrite(REL_COMPRESS,HIGH);
        digitalWrite(REL_DECOMPRESS,LOW);
        decompressing=true;
        turnMachineOff();
        didDecompress = true;
      }
    }
}

bool SELF_R::tryDecompressing()
{ 
    tempTime=millis();
    decompress();
    
    while(millis()-tempTime<(operateTime*100))
    {
        if(getAnalogInput()>=decompressionValue)
          break;
    }
    
    stopMotor();

    if(checkCMotorStatus()=='D')
      return true;
    else
      return false;

}

bool SELF_R::startSelfElligible()
{
  return (triggerSelfOn && (millis() - selfWaitTime) >= (selfOnTime * 100));
}

void SELF_R::startSelf()
{
  triggerSelfOn = false;
  selfOn = true;
  selfWaitTime = millis();
  _NSerial->print("Self Started AT : ");
  _NSerial->println(selfWaitTime);


  digitalWrite(REL_ALTERNATOR,LOW);
  digitalWrite(REL_SELF, LOW);
  ignoreAverageReadings();
  startingSelf();
}

bool SELF_R::stopSelfElligible()
{
  return (selfOn && ((millis() - selfWaitTime) >= (selfOffTime * 100)));
}

void SELF_R::stopSelf()
{
  selfOn = false;
  digitalWrite(REL_SELF, HIGH);
  stoppingSelf();
}

void SELF_R::checkNoRPM()
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
      digitalWrite(REL_ALTERNATOR,HIGH);
      if (didDecompress)
      {
      // the machine has been stopped
        MachineSwitchedOff();
        didDecompress = false;
      }
    }
  }
}


char SELF_R::checkCMotorStatus()
{
    float x = getAnalogInput();
    {
        if(x<=compressionValue)
          return 'C';
        else if(x>=decompressionValue)
          return 'D';
        else
          return 'U'; 
    }

}

bool SELF_R::triggerSelf()
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

bool SELF_R::averageRPMElligible()
{
  return (getMachineStatus() == true && (millis() - averageWaitTime) >= ((unsigned long)averageTime * 100));
}

void SELF_R::averageRPM()
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

void SELF_R::setCallBackFunctions(void(*funcRPMChange)(), void(*funcMachineOff)(),void (*funcStartSelf)(),void (*funcStopSelf)())
{
  RPMChange = *funcRPMChange;
  MachineSwitchedOff = *funcMachineOff;
  startingSelf=*funcStartSelf;
  stoppingSelf=*funcStopSelf; 
}

bool SELF_R::setMachineStatus()
{
  double tempRPM;
  tempRPM = RPM;

  if (!selfOn)
  {
    checkCMotorStatus();
    if (tempRPM > DRPM && (cMotorStatus == 'C' || cMotorStatus == 'U'))
    {
      turnMachineOn();
      return true;
    }
    else if (tempRPM > 0 && tempRPM < DRPM && cMotorStatus == 'D')
    {
      turnMachineOff();
      return false;
    }
  }
}

double SELF_R::getRPM()
{
  double tempRPM;
  noInterrupts();
  tempRPM = RPM;
  interrupts();
  return tempRPM;
}

double SELF_R::getAverageRPM()
{
  return ARPM;
}

void SELF_R::turnMachineOn()
{
  machineOn = true;
}

void SELF_R::turnMachineOff()
{
  machineOn = false;
}

bool SELF_R::getMachineStatus()
{
  return machineOn;
}

void SELF_R::ignoreAverageReadings()
{
  ignoreAvgReadings = true;
  ignoredReadingsCnt = 0;
  averageWaitTime = millis();
}

void SELF_R::discardRPMEvent()
{
  firedRPMEvent=false;
  ignoreAverageReadings();
  ignoredReadingsCnt=1;
}

void SELF_R::update()
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

  if(operating)
  {
    if(timeOut() || !checkOperatingElligible())
    {
      stopMotor();
    }
  }

  //if(!machineOn)
  //	setMachineStatus();

  /*if (!Power && (millis() - signalWait) > (signalWaitTime * 100))
  {
    turnSignalOff();
    stopDecompression();
    stopCompression();
  }

  if (stopDecompressionElligible())
    stopDecompression();

  if (stopCompressionElligible())
    stopCompression();*/

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
