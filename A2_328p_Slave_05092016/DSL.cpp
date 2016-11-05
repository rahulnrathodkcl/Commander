#include "DSL.h"
//vERSION 2 dATED 30052016

//750ml=12.90 sec		== 58ml/sec

DSL::DSL(byte DSLPIN, byte PWRDSLPIN, float MaxReading, float MinReading, float offLevelReading , float reserveLevelReading,double sensitivity, HardwareSerial *serial)
{
  SEN_DSL = DSLPIN;
  PWR_DSL = PWRDSLPIN;

  maxDiesel = MaxReading;
  minDiesel = MinReading;

  pinMode(PWR_DSL, OUTPUT);

  SEN_ON = false;
  sensorOperation=true;

  readingCnt = 200;
  dieselEmptyRate = 0;

  generatedDieselLowAlarm = false;
  generatedDieselRateAlarm = false;

  reserveLevel=reserveLevelReading ;
  offLevel=offLevelReading;

  readingWaitTime = 150; //x100 ms
  readingWait = 0;
  CDiesel = 0;
  lastReading = 0;
  lastRateReading = 0;
  lastRateReadingTime = 0;
  this->sensitivity = sensitivity; //(maxDiesel-minDiesel)/80;
  _Serial = serial;
  _Serial->begin(115200);

  //sensorConnected=false;

  //isSensorConnected();
sensorConnected=true;
}

void DSL::isSensorConnected()
{
  turnSensorOn();
  //digitalWrite(PWR_DSL,HIGH);
  getReading();
  if(CDiesel==0.0 || CDiesel >= 3.0)
  {
    _Serial->println("Diesel Sensor Not Found..");
    sensorConnected=false;  
  }
  else
  {
    _Serial->println("Found Diesel Sensor..");    
    sensorConnected=true;
  }
 
  turnSensorOff();
  //digitalWrite(PWR_DSL,LOW);
}

void DSL::setCallBackFunctions(void (*funcDieselRate)(), void (*funcDieselLow)(),void (*funcMachineOff)())
{
  fdieselRate = *funcDieselRate;
  fdieselLow = *funcDieselLow;
  fMachineOff= *funcMachineOff;
}

bool DSL::readingElligible()
{
  return (!SEN_ON && ((millis()-readingWait) >= (readingWaitTime * 100)));
}

void DSL::turnSensorOff()
{
  digitalWrite(PWR_DSL, LOW);
  SEN_ON = false;
  readingWait = millis();
}

void DSL::checkDieselLow()
{
  float ETA = calculateETA();

  if (!generatedDieselLowAlarm && (CDiesel < (reserveLevel+sensitivity) || (ETA <= 60 && ETA != -300)))
  {

    //_Serial->println("Diesel is low...");
    generatedDieselLowAlarm = true;
    fdieselLow();
    //Raise diesel reserve alarm
  }
  else if(CDiesel <= offLevel)
  {
    fMachineOff();
  }
  else if (CDiesel >= (minDiesel + reserveLevel + sensitivity))
  {
    generatedDieselLowAlarm = false;
  }
}

void DSL::setSensorOperation(bool temp)
{
    sensorOperation=temp;
    turnSensorOff();
}

float DSL::calculateETA()		//return time in minutes
{
  if (CDiesel == 0 || dieselEmptyRate == 0 || CDiesel < minDiesel)
    return -300;

  float temp = ((CDiesel - minDiesel) / sensitivity) * (dieselEmptyRate / 60000);
  return temp;

}

void DSL::setDieselEmptyRate()
{
  if ((lastRateReading - CDiesel) > sensitivity)
  {
    dieselEmptyRate = millis() - lastRateReadingTime;
    lastRateReading = CDiesel;
    lastRateReadingTime = millis();
  }
}

bool DSL::checkDieselRate()
{
  if ((lastReading - CDiesel) > sensitivity && !generatedDieselRateAlarm)
  {
    if(!generatedDieselRateAlarm)
    {
      fdieselRate();
      generatedDieselRateAlarm = true;
    }
    return true;
  }
  else if ((lastReading - CDiesel) < sensitivity)
  {
    generatedDieselRateAlarm = false;
    return false;
  }
}

void DSL::turnSensorOn()
{
  SEN_ON = true;
  digitalWrite(PWR_DSL, HIGH);
  //	readingoffWait=millis();
}

void DSL::getReading()
{
  long int temp = 0;
  analogRead(SEN_DSL); //throw away first reading...
  int s;

  long int max=0,min=1024;
  for (int i = 0; i < 10; i++)
  {
    s = analogRead(SEN_DSL);
    if(s<min)
      min=s;
    else if(s>max)
      max=s;
    temp += s;
  }
  turnSensorOff();

  lastReading = CDiesel;

  //I=(5.0-V2)/R1;
  //R2=V2/I;
  _Serial->print("Max Reading : ");
  _Serial->println(max);

  _Serial->print("Min Reading : ");
  _Serial->println(min);

  _Serial->print("Avg Reading : ");
  _Serial->println((max+min)/2.0,DEC);
  
  _Serial->print("Avg Reading : ");
  _Serial->println(((max+min)/2.0)/204.8,DEC);

  _Serial->print("Diesel RAW Reading .. :");
  _Serial->println(temp/10.0, DEC);

  CDiesel = temp / 2048.0;//20480.0; //100.0;

  _Serial->print("Diesel Volt Reading .. :");
  _Serial->println(CDiesel, DEC);
  //V2 = (5.0 / 1024.0) * CDiesel;

  if(!sensorOperation)
  {
    return;
  }
  
  if (lastRateReading == 0 || (lastRateReading + sensitivity < CDiesel)) //|| (millis()-lastRateReadingTime)>=3600000)
  {
    lastRateReading = CDiesel;
    lastRateReadingTime = millis();
  }
  else
  {
    if (lastReading != 0 )
      if(checkDieselRate())
      {
        setDieselEmptyRate();
      }    
    checkDieselLow();
  }

}

void DSL::update()
{
  if(sensorConnected)
  {
    if (readingElligible())
      {
       turnSensorOn();
      } 

    if (SEN_ON && sensorOperation)
    {
      _Serial->println("getReading");
      getReading();  
    }
  }
}
