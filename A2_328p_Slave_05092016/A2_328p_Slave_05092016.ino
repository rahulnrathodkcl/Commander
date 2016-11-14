//This IC would be responsible for the communication with all the sensors.
//It takes the commands from the Main IC, which is the Master For the SPI.
//This software is designed to be used as the slave SPI device.
//It notifies the master of any event occured from reading of any sensor.
//Also, it answers the master's question by checking the data of the sensor.


#include "Definitions.h"
#include "SELF.h"
#include "SMOTOR_R.h"
#include "TEMP.h"
#include "S_EEPROM.h"
#include "slaveSPI.h"

//#include <avr/sleep.h>

//#define MaximumTemperature 55.0
//#define MinimumMotorVoltage 0.30
//#define MaximumMotorVoltage 1.70
//#define TemperatureWaitTime 300 //x100 mS

#ifdef use_diesel
  #define SEN_DSL A1
  #define PWR_DIESEL 27

  #define MinimumDieselVoltage 0.28  //x1024/5.0 
  #define MaximumDieselVoltage 1.68   //x1024/5.0
  #define DieselSensitivty 0.10
  #define DieselMachineOffVoltage 0.30
  #define DieselReserveVoltage 0.45
  #include "DSL.h"

  void registerDieselLow()
  {
    ////Serial.println("Diesel in Reserve");
    //sim1.registerEvent('L',true,true);
    //lastImmediateEvent='L';
  }
  
  void registerDieselRate()
  {
    //Serial.println("Diesel getting low very rapidly..");
    //Serial.println("Diesel Rate increased...");
    //sim1.registerEvent('R',true,false);
  }
  
  void turnMachineOffDieselLow()
  {
      //Serial.println("Machine is turning Off Due to Very Low Diesel ");
  }

  DSL dsl1(SEN_DSL, PWR_DIESEL, MaximumDieselVoltage, MinimumDieselVoltage,DieselMachineOffVoltage,DieselReserveVoltage, DieselSensitivty, &Serial);

#endif

void selfGotStarted();
void selfGotStopped();
void startSequenceQuery();
void setMotorStatus(bool);
byte initialized;

bool gotMotorStatus;
bool waitingForStatus;
bool gotQuery;
bool motorDecompressed;

unsigned long securityReadingTemp=0;
bool checkingSecurity=false;
byte securityCnt=0;
bool securityAlarmed=false;
String str;
bool requestSensorReadings=true;

TEMP temp1;
SMOTOR_R smotor1;
S_EEPROM eeprom1;
SELF self1(setMotorStatus,selfGotStarted,selfGotStopped);
slaveSPI spi1(startSequenceQuery,&eeprom1);

#ifdef CHK_BATTERY  
  #include "BATTERY.h"
  BATTERY batteryLevel;
#endif

bool securityCheckElligible()
{
    return(!checkingSecurity && millis()-securityReadingTemp>1000);
}

void triggerSecurityCheck()
{
  checkingSecurity=true;
  digitalWrite(PIN_SECLIMITPWR,HIGH);
  securityReadingTemp=millis();
//  Serial.println("SEC CHK TRIGGER");
}

bool securityLimitReadingElligible()
{
  return(checkingSecurity && (millis()-securityReadingTemp)>=100);
}

void stopSecurityCheck()
{
  checkingSecurity=false;
  digitalWrite(PIN_SECLIMITPWR,LOW);
//  Serial.println("STOP SEC CHK");
}

void startSequenceQuery()
{
    gotMotorStatus=false;
    waitingForStatus=false;
    gotQuery=true;
    self1.startSequenceQuery();
}

void setMotorStatus(bool canStart)
{
  gotMotorStatus=true;
  motorDecompressed=canStart;
}

void sendResponseAsPerMotorState()
{
      if(motorDecompressed)
        spi1.sendData(A_STARTYES);
      else
        spi1.sendData(A_STARTCOMPRESS);
}

void operateOnQuery()
{ 
  gotQuery=false;
  if(temp1.checkTempLimitReached())
  {
    spi1.sendData(A_STARTTEMP);
    return;
  }

  if(self1.getMachineStatus())
    spi1.sendData(A_STARTON);
  else
  {
    if(gotMotorStatus)
      sendResponseAsPerMotorState();
    else
      waitingForStatus=true;
  }
}

void selfGotStarted()
{
    #ifdef use_diesel
    dsl1.setSensorOperation(false);
    #endif
    //smotor1.setSelfOperationStatus(true);
}

void selfGotStopped()
{
    #ifdef use_diesel
    dsl1.setSensorOperation(true);
    #endif
    //smotor1.setSelfOperationStatus(false);
}

//void settings()
//{
//  Serial.println("settings");
//}


/*ISR(TIMER3_OVF_vect)
{
  //TIMSK3 = (TIMSK3 & 0xFE);   //disable the Timer INterrupt
  //self1.NoRPM();

  //Serial.println("Got A Timer OverFLow Event..");
  
  //digitalWrite(LED, !digitalRead(LED));
  //  TCNT3 = 0x48E4;       //initialize the counter
}*/

void FIVR_RPM()
{
  //int x = digitalRead(eeprom1);
  //if (x == 0)
  {
    self1.lastrise = self1.currentrise;
    self1.currentrise = millis();
    self1.gotTrigger = true;
    //TIMSK3 |= (1 << TOIE3); //enable timer 3 
    //TCNT3 = 0x48E4;   //intialize the counter for timer 3
    //Serial.println(TCNT3);
    //self1.setTimer();
  }
}

void FIVR_SDECOMPRESS()
{
  self1.IVR_SDECOMPRESS();
  //self1.sDecompress=true;
}

void printNumbers()
{
  /*#ifndef disable_debug
    Serial.print("high RPM: \t");
    Serial.println(eeprom1.RPM);

    Serial.print("comp rpm: \t");
    Serial.println(eeprom1.COMPRPM);
    Serial.print("Decomp rpm: \t");
    Serial.println(eeprom1.DECOMPRPM);
    Serial.print("M LOW: \t");
    Serial.println(eeprom1.MOTORLOW);
    Serial.print("M HIGH: \t");
    Serial.println(eeprom1.MOTORHIGH);
    Serial.print("TEMP: \t");
    Serial.println(eeprom1.HIGHTEMP);
    Serial.print("force start: \t");
    Serial.println(eeprom1.FORCESTART);
  #endif*/
}

void setup() {
  // put your setup code here, to run once:
  
  //Serial.begin(19200);
  

  pinMode(PIN_SECLIMITSIG,INPUT);
  pinMode(PIN_SECLIMITPWR,OUTPUT);
  digitalWrite(PIN_SECLIMITPWR,LOW);

  eeprom1.loadAllData();
  //eeprom1.saveTempSettings((unsigned short int)45);

  //printNumbers();

  self1.setSPI(&spi1);
  smotor1.setSPI(&spi1);
  temp1.setSPI(&spi1);

  self1.setEEPROM(&eeprom1);
  temp1.setEEPROM(&eeprom1);
  smotor1.setEEPROM(&eeprom1);

  #ifdef CHK_BATTERY
    spi1.setObjectReference(&self1,&smotor1,&temp1,&batteryLevel);
  #else
    spi1.setObjectReference(&self1,&smotor1,&temp1);
  #endif
  //spi1.setInformFunction(self1->informSelf,self1->informLimit,smotor1->inform);
  //spi1.setQueryFunction(startSequenceQuery,temp1.query,smotor1.query,self1.limitQuery,self1.machineQuery);

  initialized = false;

  #ifdef use_diesel
    dsl1.setCallBackFunctions(registerDieselRate, registerDieselLow,turnMachineOffDieselLow);
  #endif

  attachInterrupt(digitalPinToInterrupt(PIN_RPMSEN), FIVR_RPM, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_DECOMP), FIVR_SDECOMPRESS, CHANGE);
  

  //attachInterrupt(digitalPinToInterrupt(SIG_COMPRESS), FIVR_SCOMPRESS, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(SIG_FAST),FIVR_FAST,CHANGE);
  //attachInterrupt(digitalPinToInterrupt(SIG_SLOW),FIVR_SLOW,CHANGE);
  //FIVR_RPM();
  /*if(digitalRead(PIN_RPMSEN)==HIGH)
    {
      FIVR_RPM();
    }*/
}

ISR (SPI_STC_vect)
{
  spi1.receiveInterrupt();
}  // end of interrupt routine SPI_STC_vect


unsigned long lastTime=0;
void loop() {

  if (!initialized && millis() >= 500)
  {
    if(requestSensorReadings)
    {
      self1.triggerMachineStatus();
      temp1.init();    
      #ifdef CHK_BATTERY
        batteryLevel.checkInitialBatteryLevel();
      #endif
      requestSensorReadings=false;    
    }
    if(millis()>2500)
    {
      temp1.init();
      initialized = true;
    }
    return;
  }

  if(securityCheckElligible())
  {
      triggerSecurityCheck();
  }
  if(securityLimitReadingElligible())
  {
    //Serial.println(analogRead(PIN_SECLIMITSIG));
    if(digitalRead(PIN_SECLIMITSIG)==HIGH)
    {
      if(!securityAlarmed)
      { 
        if(securityCnt<2)
        {
            securityCnt++;
        }
        else
        {
          securityAlarmed=spi1.sendData(EVENT_SECURITY);
          if(securityAlarmed)
            securityCnt=0;
          //securityAlarmed=true;
        }
      }
    }
    else
    {
        securityAlarmed=false;
    } 
    stopSecurityCheck();
  }
  
  if(gotQuery)
    operateOnQuery();
  
  if(waitingForStatus && gotMotorStatus)
  {
     sendResponseAsPerMotorState();
     gotMotorStatus=false;
     waitingForStatus=false;
  } 

  self1.update();
  smotor1.update();
  temp1.update();
  spi1.update();
  #ifdef CHK_BATTERY
    batteryLevel.update();
  #endif 

  /*if(millis()-lastTime>5000)
  {
    //printNumbers();
    //Serial.print("Current Temp Setting:");
    //Serial.println(eeprom1.HIGHTEMP);
    lastTime=millis();
  }*/

  //#ifdef use_diesel
  //  dsl1.update();
  //#endif
}
