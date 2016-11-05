
//This program is to be used for master communication IC
//It would be loaded on to the communications IC which would
//be responsible for the communication with SIM900 and also
//to drive the motors and start self.
//It uses the SPI bus to communicate with the sensors IC.


//Setting For Compression Decompression Motor IS 23.4kOhms AND 1.71 Voltage(INPUT 5V) when decompression cannot be achieved due to rod in between.

//#define use_diesel

//#define use_mega
#include "Defintions.h"
#include "masterSPI.h"
#include "SIM.h"
#include "SELF.h"
#include "SMOTOR_R.h"
#include "S_EEPROM.h"

//_________________________________DIESEL_______________________
//#ifdef use_diesel
//  #define SEN_DSL A1
//  #define PWR_DIESEL 27
//
//  #define MinimumDieselVoltage 0.28  //x1024/5.0 
//  #define MaximumDieselVoltage 1.68   //x1024/5.0
//  #define DieselSensitivty 0.10
//  #define DieselMachineOffVoltage 0.30
//  #define DieselReserveVoltage 0.45
//
//  void registerDieselLow()
//  {
//    //USART1->println("Diesel in Reserve");
//    //sim1.registerEvent('L',true,true);
//    //lastImmediateEvent='L';
//  }
//  
//  void registerDieselRate()
//  {
//    //USART1->println("Diesel getting low very rapidly..");
//    ////USART1->println("Diesel Rate increased...");
//    //sim1.registerEvent('R',true,false);
//  }
//  
//  void turnMachineOffDieselLow()
//  {
//      //USART1->println("Machine is turning Off Due to Very Low Diesel ");
//      triggerStopMachine();
//      //stopMachine();
//  }
//
//#endif
//______________________________________________________________________________

bool inform;
bool tempLimitReached=false;
bool batteryLow=false;
bool securityAlarmed=false;
bool gotCannotTurnOffMachineEvent=false;
bool gotRPMIncreasedEvent=false;
bool gotSwitchedOffEvent=false;
bool gotStartedEvent=false;


void registerRPMIncreased();
void registerRPMDecreased();
void cannotTurnOffMachine();
void speedMotorOperated(bool);
void motorCommandStatus(bool);
void selfCommandStatus(char);
void informSIM(char,bool b=false);

S_EEPROM eeprom1;

#ifndef disable_debug
  SoftwareSerial s1(5,6);
  #ifdef software_SIM
    SIM sim1(&Serial,&s1,PIN_SIMSLEEP);
    SELF self1(&Serial);
    SMOTOR_R smotor1(&Serial,registerRPMIncreased, registerRPMDecreased,speedMotorOperated,cannotTurnOffMachine,motorCommandStatus);
    masterSPI masterSPI1(&Serial,&tempLimitReached,&smotor1.eventOccured,&smotor1.eventByte,&self1.eventOccured,&self1.eventByte,&batteryLow,&securityAlarmed);
    HardwareSerial* USART1=&Serial;
  #else
    SIM sim1(&s1,&Serial,PIN_SIMSLEEP);
    SELF self1(&s1);
    SMOTOR_R smotor1(&s1,registerRPMIncreased, registerRPMDecreased,speedMotorOperated,cannotTurnOffMachine,motorCommandStatus);
    masterSPI masterSPI1(&s1,&tempLimitReached,&smotor1.eventOccured,&smotor1.eventByte,&self1.eventOccured,&self1.eventByte,&batteryLow,&securityAlarmed);
    SoftwareSerial* USART1=&s1;
  #endif
#else
  SIM sim1(&Serial,PIN_SIMSLEEP);
  SELF self1;
  SMOTOR_R smotor1(registerRPMIncreased, registerRPMDecreased,speedMotorOperated,cannotTurnOffMachine,motorCommandStatus);
  masterSPI masterSPI1(&tempLimitReached,&smotor1.eventOccured,&smotor1.eventByte,&self1.eventOccured,&self1.eventByte,&batteryLow,&securityAlarmed);
#endif  

byte initialized;
bool gotMachineOffCommand;
char lastImmediateEvent;

void triggerStopMachine(bool b=false);

void registerRPMIncreased()
{
  //sim1.registerEvent('I', false, false);
}

void gotImmediateResponse(bool temp)
{
  if (temp && self1.machineOn)
  {
    #ifndef disable_debug
      USART1->print("MachineOff_I Event:");
      USART1->println(lastImmediateEvent);
    #endif
    triggerStopMachine();
  }
  
  lastImmediateEvent = 'N';
}

void cannotTurnOffMachine()
{
  #ifndef disable_debug
    USART1->println("Cannot Off");  
  #endif
  gotCannotTurnOffMachineEvent=true;
}

void reportCannotTurnOffMachine()
{
  if(sim1.registerEvent('Z', true, false))
    gotCannotTurnOffMachineEvent=false;
}

void motorCommandStatus(bool motorStatus)   //inc dec stopMachine command Status
{    
      if(smotor1.checkCurrentOperation()=='S')
      {
        if(motorStatus)
          gotMachineOffCommand=true;

        if(!inform)
          return;
      }

    if(motorStatus)
      informSIM('D');
    else
      informSIM('L');
}

void speedMotorOperated(bool motorStatus)   //true = on, so stop diesel ratings
{
    #ifdef use_diesel
      dsl1.setSensorOperation(!motorStatus);
    #endif
}

void maxTempLimitReached()
{
  if(eeprom1.machineOn)
  {
    if(sim1.registerEvent('T', true,true))
    {
      lastImmediateEvent = 'T';
      tempLimitReached=false;
    }
  }
  else
    tempLimitReached=false;
}

void batteryLevelLow()
{
  if(sim1.registerEvent('X',true,false))
    batteryLow=false;
}

void reportSecurityBreach()
{
  if(sim1.registerEvent('Y',true,false))
  {
    USART1->print("SEC");
    USART1->println(" Event");
    securityAlarmed=false;
  }
}

void informSIM(char b,bool selfStatus)
{
  if(!selfStatus)
    sim1.speedMotorStatus(b);
  else
    sim1.setSelfStatus(b);
  inform=false;
}

void registerRPMDecreased()
{
  //sim1.registerEvent('D', false, false);
}

void triggerIncreaseRPM(byte steps=1)
{
  #ifndef disable_debug
    USART1->print("INC");
    USART1->println("RPM");
  #endif  
  smotor1.triggerIncreaseRPM(steps);
}

void triggerDecreaseRPM(byte steps=1)
{
  #ifndef disable_debug
    USART1->print("DEC");
    USART1->println("RPM");
  #endif  
  smotor1.triggerDecreaseRPM(steps);
}

void triggerStopMachine(bool b)
{
  inform=b;
  #ifndef disable_debug
    USART1->println("STOP");
  #endif  
  if(self1.machineOn)
  {
    smotor1.triggerSwitchOff();
  }
  else
  {
  #ifndef disable_debug
    USART1->println("OFF ");
  #endif  
    if(inform)
      informSIM('O');
  }
}

void selfCommandStatus(char selfStatus)
{
//  #ifndef disable_debug
//    USART1->print("SELF");
//    USART1->print(" STATUS:");
//    USART1->println(selfStatus);
//    USART1->print("INFORM:");
//    USART1->println(inform);
//  #endif
  if(inform)
    informSIM(selfStatus,true);
}

void triggerStartMachine(bool b=false)
{
  inform=b;
  #ifndef disable_debug
    USART1->println("START");
  #endif  
  self1.triggerSelf();
}

void settings()
{  
//  #ifndef disable_debug
//    USART1->println("settings");
//  #endif  
}

void rpmIncreased()
{
  #ifndef disable_debug
    USART1->print("RPM ");
    USART1->println("INC EVENT");  
  #endif  
    gotRPMIncreasedEvent=true;
    
}

void reportRPMIncreasedEvent()
{
  if(sim1.registerEvent('F', true, true))
  {
    lastImmediateEvent = 'F';
    gotRPMIncreasedEvent=false;
  }
}

void machineSwitchedOn()
{
  //lastImmediateEvent='N';
  gotStartedEvent=true;
}

void reportStartEvent()
{
  if(sim1.registerEvent('C', true, false))
    gotStartedEvent=false;        

}
void machineSwitchedOff()
{
  if (gotMachineOffCommand)
  {
    smotor1.backOff();
  }  
  gotMachineOffCommand=false;
  gotSwitchedOffEvent=true; 
}

void reportSwitchOffEvent()
{
  if(sim1.registerEvent('O', true, false))
  {
    lastImmediateEvent='N';
    gotSwitchedOffEvent=false; 
  }  
}
//void printNumbers()
//{
//  #ifndef disable_debug
//    if (eeprom1.numbersCount > 0)
//    {
//      USART1->println("Numbers:");    
//      USART1->print("1:");
//      USART1->print(eeprom1.primaryNumber);
//      USART1->print("\t");
//      USART1->println(eeprom1.primaryNumber.length());
//  
//        for (int i = 0; i < eeprom1.numbersCount - 1; i++)
//        {
//          USART1->print(i + 2);
//          USART1->print(":");
//          USART1->print(eeprom1.secondary[i]);
//          USART1->print("\t");
//          USART1->println(eeprom1.secondary[i].length());
//        }
//    }
//  #endif
//}

ISR(BADISR_vect)
{
    #ifndef disable_debug
      USART1->println("!!!");
      USART1->println(MCUSR);
    #endif  
}

ISR(PCINT2_vect)
{
  if(digitalRead(SLAVE_READY_INTERRUPT_PIN)==HIGH)
    masterSPI1.ISR_SlaveReady();
  else
  {
    #ifndef disable_debug
      s1.gotInterrupt();
    #endif
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  #ifndef disable_debug
    USART1->begin(19200);
  #endif

  eeprom1.loadAllData();
  
  sim1.setCallBackFunctions(gotImmediateResponse);
  sim1.setDTMFFunctions(triggerIncreaseRPM,triggerDecreaseRPM,triggerStopMachine, triggerStartMachine);

  self1.setCallBackFunctions(rpmIncreased, machineSwitchedOff,machineSwitchedOn,selfCommandStatus);

  sim1.setEEPROM(&eeprom1);
  self1.setEEPROM(&eeprom1);
  
  self1.setSPI(&masterSPI1);
  smotor1.setSPI(&masterSPI1);
  sim1.setSPI(&masterSPI1);
  initialized = false;
  gotMachineOffCommand = false;

  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT23);

}

String str;

void playBuzzer(byte times,byte d)
{
  int temp=d*100;
  for(byte i=0;i<times;i++)
  {
    digitalWrite(PIN_BUZZER,HIGH);
    delay(temp);
    digitalWrite(PIN_BUZZER,LOW);
    if(times>1)
      delay(temp);
  }
}

void loop() {

  if(!initialized)
  {
    if(millis()>=5000)
    {
      if (!sim1.initialize())
      {
        #ifndef disable_debug
          USART1->println("NOT INIT SIM");
        #endif
          playBuzzer(5,3);
      }
      else
      {
        playBuzzer(2,3);
      }

      bool SPIDataReceived;
      byte SPIData;
      unsigned long m=millis();
      if(masterSPI1.queryState(Q_SLAVEEXISTENCE,&SPIData,&SPIDataReceived))
      {
        #ifndef disable_debug
          USART1->println("QS");
        #endif
        while(millis()-m<1000 && !SPIDataReceived)
        { 
        }
        initialized = true;  
        #ifndef disable_debug
          USART1->println(SPIData==A_SLAVEEXISTS);
        #endif
        if(SPIDataReceived)
        {
            if(SPIData==A_SLAVEEXISTS)
            {  
              //self1.queryMachineState();
              playBuzzer(2,3);
              return;
            }
        }
      }
      playBuzzer(5,3);
      return;
    }
  }

  if(tempLimitReached)
    maxTempLimitReached();

  if(gotCannotTurnOffMachineEvent)
    reportCannotTurnOffMachine();

  if(gotRPMIncreasedEvent)
    reportRPMIncreasedEvent();

  if(gotSwitchedOffEvent)
    reportSwitchOffEvent();

  if(gotStartedEvent)
    reportStartEvent();

  if(securityAlarmed)
    reportSecurityBreach();

  if(batteryLow)
    batteryLevelLow();

  #ifndef disable_debug 
  if (USART1->available() > 0)
  {
    str = USART1->readStringUntil('\n');
    if (str == "S\r")
      triggerStartMachine();
    else if (str == "I\r")
      triggerIncreaseRPM(1);
    else if (str == "D\r")
      triggerDecreaseRPM(1);
    else if (str == "A\r")
      triggerStopMachine();
    else
      sim1.operateOnMsg(str,false);
  }
  #endif
  
  masterSPI1.update();
  self1.update();
  smotor1.update();
  sim1.update();
  }