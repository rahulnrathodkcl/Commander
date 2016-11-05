//Version 2 Dated : 29052016
#include "SIM.h"
#include <SoftwareSerial.h>

#ifdef use_mega

  SIM::SIM(HardwareSerial* serial,HardwareSerial* serial1, byte SLEEPPIN)
  {
    _NSerial=serial;
    _NSerial->begin(115200);
    _SSerial=serial1;
    _SSerial->begin(57600);
    anotherConstructor(SLEEPPIN);
  }
#else

  #ifndef disable_debug
  SIM::SIM(HardwareSerial* serial,SoftwareSerial* serial1, byte SLEEPPIN)
  {
    _NSerial=serial;
    _NSerial->begin(19200);
    _SSerial=serial1;
    _SSerial->begin(19200);
    anotherConstructor(SLEEPPIN);
  }
  #else
  SIM::SIM(SoftwareSerial* serial, byte SLEEPPIN)
  {
    _SSerial=serial;
    _SSerial->begin(19200);
    anotherConstructor(SLEEPPIN);
  }
  #endif
#endif

void SIM::clearBRead()
{
  readBefore=false;
  bReadString="";
}

void SIM::bRead()
{
  if(_SSerial->available()>0)
  {
    bReadString=_SSerial->readStringUntil('\n');
    readBefore=true;
  }
}

void SIM::anotherConstructor(byte SLEEPPIN)
{
  clearBRead();
  initialized=false;

  SLEEP_PIN=SLEEPPIN;
  acceptCommandsTime=200;
  commandsAccepted=false;
  
  pinMode(SLEEP_PIN,OUTPUT);
  rejectCommands();

  soundPlayNumber = 3;
  soundWaitTime = 5;
  soundPlayedNumber = 0;
  bplaySound = false;


  actionType = 'N';
  responseWaitTime = 900;
  makeResponse = false;

  callCutWaitTime = 250;
  nr = 0;
  responseSetting = 'C';
  currentStatus = 'N';
  currentCallStatus = 'N';
  gotSettingTone = false;
  callAccepted = false;
  immediateEvent=false;
  sendImmediateResponse=false;
  freezeIncomingCalls=false;  
}

#ifdef use_mega
  void SIM::setDTMFFunctions(char (*p1)(),char (*p2)(),char (*p3)(),char (*p4)())
  {
   f1=*p1;
   f2=*p2;
   f3=*p3;
   f4=*p4;
  }
#else
  void SIM::setDTMFFunctions(char (*p1)(),char (*p2)(),char (*p3)())//,char (*p4)(),void (*p5)())
  {
   f1=*p1;
   f2=*p2;
   f3=*p3;
  }
#endif

bool SIM::initialize()
{
  byte attempts=0;
  try_again:
  if (sendBlockingATCommand("AT\r\n"))
  {
    #ifndef disable_debug
    _NSerial->println("AT OK");
    #endif 
    if (sendBlockingATCommand("AT+DDET=1\r\n"))
    {
      #ifndef disable_debug
      _NSerial->println("DTMF OK");
      #endif
      if (sendBlockingATCommand("AT+CLIP=1\r\n"))
      {
        #ifndef disable_debug
        _NSerial->println("CLIP OK");
        #endif
        if (sendBlockingATCommand("AT+CLCC=1\r\n"))
        {
          #ifndef disable_debug
          _NSerial->println("CLCC OK");
          _NSerial->println("INIT OK");
          #endif
          initialized=true;
          return true;
        }
      }
    }
  }
  if(!initialized && attempts==0)
  {
    attempts++;
    goto try_again;  
  }
  return false;
}

bool SIM::isNumber(String &str)
{
//+CLCC: 1,1,4,0,0,"+917041196959",145,""
//+CLIP: "+917041196959",145,"",,"",0
return (stringContains(str, "+CLIP: \"", 11, 21));
  //return stringContains(str, "+CLCC: 1,1,4,0,0,\"", 21, 31);
}

bool SIM::checkNumber(String number)
{
  /*String number;
    while (stringContains(number, "+CLIP: \"", 11, 21) == false)
    {
    if (SIM.available() > 0)
      number = SIM.readStringUntil('\n');
    }
    //number = str.substring(8, 21);
    _NSerial->println(number);
  */

  /*int ind = 0;
    if (CurEPSettings.n_mobiles != 0)
    {
    while (ind < CurEPSettings.n_mobiles)
    {
      _NSerial->print(number); _NSerial->print("=="); _NSerial->println(CurEPSettings.phones[ind]);

      if (number == CurEPSettings.phones[ind])
        return true;
      ind++;
    }
    return false;
    }
    else
    return true;
  */
  /*if (number == "9825193202")
    return true;
  else if (number == "7777931333")
    return true;
  else if (number == "9879128308")
    return true;
  else if (number == "7779021943")
    return true;
  //else if (number == "7698678761")
    //return true;
    */ 
  #ifndef disable_debug
    _NSerial->print("Number:");
    _NSerial->println(number);
  #endif

  if (number == "7041196959")
    return true;
  else if (number == "9879950660")
    return true;
  else if (number == "7698439201")
    return true;
  else if (number == "9173191310")
    return true;
  else if (number == "9427664137")
    return true;
  else
    return false;
}

void SIM::acceptCommands()
{
  if(!commandsAccepted)
  {
    #ifndef disable_debug
      _NSerial->print("commandsAccepted:");
      _NSerial->println(millis());
    #endif
    //pinMode(SLEEP_PIN,OUTPUT);
    digitalWrite(SLEEP_PIN,LOW);
    commandsAccepted=true;
    tempAcceptCommandTime=millis();  
    while(millis()-tempAcceptCommandTime<=100)
    {}
  }
  else
  {
    tempAcceptCommandTime=millis();  
  }
}

void SIM::rejectCommands()
{
  #ifndef disable_debug
    _NSerial->print("Commands rejected:");
    _NSerial->println(millis());
  #endif
  digitalWrite(SLEEP_PIN,HIGH);
  //pinMode(SLEEP_PIN,INPUT);
  commandsAccepted=false;
}
void SIM::sendCommand(char cmd,bool newline=false)
{
  acceptCommands();
    if(!newline)
    _SSerial->print(cmd);
    else
    _SSerial->println(cmd);
}

void SIM::sendCommand(String cmd,bool newline=false)
{
  acceptCommands();
  if(cmd=="")
    _SSerial->println();
  else
  {
    if(!newline)
      _SSerial->print(cmd);
    else
      _SSerial->println(cmd);
  }
}

bool SIM::sendBlockingATCommand(String cmd)
{
  //_NSerial->print(cmd); 
  sendCommand(cmd);
  //_SSerial->print(cmd);
  #ifndef disable_debug
  _NSerial->print(cmd);
  #endif

  unsigned long t = millis();
  String str;
  while (millis() - t < 3000)
  {
    if (_SSerial->available() > 0)
    {
      str = readString();
      #ifndef disable_debug
      _NSerial->println(str);
      #endif

      if (matchString(str, "OK\r") == true)
        return true;
      else if (matchString(str, "ERROR\r") == true)
        return false;
    }
  }
  return false;
}

String SIM::readString()
{
  String str = "";
  if (_SSerial->available() > 0)
  {
    acceptCommands();
    str = _SSerial->readStringUntil('\n');
    #ifndef disable_debug
    _NSerial->println(str);
    #endif
  }
  return str;
}

bool SIM::matchString(String m1, String m2)
{
  return (m1 == m2);
}

bool SIM::stringContains(String &sstr, String mstr, int sstart, int sstop)
{
  if (sstr.startsWith(mstr))
  {   
        sstr = sstr.substring(sstart, sstop);
    return true;
  }
  return false;
}


bool SIM::isRinging(String str)
{
  return (str == "RING\r");
}

bool SIM::isDTMF(String &str)
{
  return stringContains(str, "+DTMF:", 6, 7);
}

bool SIM::isCut(String str)
{

  if(currentStatus=='I' && (currentCallStatus=='I' || currentCallStatus=='O'))
  {
    #ifndef disable_debug
    _NSerial->print("String Received : ");
    _NSerial->print(str);
    _NSerial->println("String Ended Here");
    #endif
  }

  if(matchString(str, "NO CARRIER\r"))
  {
    #ifndef disable_debug
    _NSerial->println("No Carrier Detected");
    #endif
    return true;
  }
  else if(matchString(str, "BUSY\r"))
  {
    #ifndef disable_debug
    _NSerial->println("Busy Detected");
    #endif
    return true;
  }
  else if(matchString(str, "NO ANSWER\r"))
  {
    #ifndef disable_debug
    _NSerial->println("No Answer Detected");
    #endif
    return true;
  }
  else if(matchString(str, "ERROR\r"))
  {
    #ifndef disable_debug
    _NSerial->println("ERROR Detected");
    #endif
    return true;
  }
  return false;

  //return (matchString(str, "NO CARRIER\r") || matchString(str, "BUSY\r") || matchString(str, "NO ANSWER\r") || matchString(str, "ERROR\r"));
}

bool SIM::isSoundStop(String str)
{
  return (matchString(str, "AMR_STOP\r"));
}

char SIM::callState(String str)
{
  //0:  call accepted
  //3:  call made
  //6:  call ended
  #ifndef disable_debug
  _NSerial->print("str:");
  _NSerial->println(str);
  #endif

  if (stringContains(str, "+CLCC: 1,0,3", 11, 12))
  {
    return 'R'; //call made
  }
  else if (stringContains(str, "+CLCC: 1,0,0", 11, 12))
  {
    return 'I'; //call accepted
  }
  else if (stringContains(str, "+CLCC: 1,0,6", 11, 12))
  {
    return 'E'; //call ended
  }
  else
  {
    return 'N';
  }
}

void SIM::makeCall()
{
  //_NSerial->println("Call Made");
  //hangup_call();
  //if (!send_AT_command("ATD+91" + CurEPSettings.phones[0] + ";"))
  acceptCommands();
  _SSerial->flush();
 
  /*if (!sendBlockingATCommand("ATD+917698439201;"))
  {
    endCall();
    if(immediateEvent && sendImmediateResponse)
      sendImmediateFeedback(true);
    if (responseSetting == 'A')
      sendSMS();
  }
  else
  */
  {
    //sendBlockingATCommand("ATD+917698439201;");
    sendCommand("ATD+917698439201;",true);
    //_SSerial->println("ATD+917698439201;");
    double temp=millis();
    while(millis()-temp<100)
    {    }
    sendCommand("",true);
    //_SSerial->println();
    //_SSerial->flush();
    #ifndef disable_debug
    _NSerial->println("Call Made");
    #endif
    callCutWait = millis();
    currentStatus = 'R';
    currentCallStatus = 'O';
  }
}

void SIM::endCall()
{
  nr = 0;
  _SSerial->flush();
  sendCommand("AT+CHUP",true);
  //_SSerial->println("AT+CHUP");
  double temp=millis();
  while(millis()-temp<100)
  {  }
  sendCommand("",true);
  //_SSerial->println();
  _SSerial->flush();
  /*_SSerial->println("ATH");
  _SSerial->println("ATH0");
  */
  //_SSerial->println("ATH");

  //sendBlockingATCommand("ATH");
  freezeIncomingCalls=false;
  gotSettingTone = false;
  soundPlayedNumber = 0;
  callAccepted = false;
  currentStatus = 'N';
  currentCallStatus = 'N';
  #ifndef disable_debug
  _NSerial->println("Call End..");
  #endif
  
  if(immediateEvent)
    sendImmediateFeedback(true);
  //callended = true;
}

void SIM::acceptCall()
{
  callAccepted = true;
  _SSerial->flush();
  sendCommand("ATA",true);
  _SSerial->flush();
  //_SSerial->println("ATA");
  currentStatus = 'I';
  currentCallStatus = 'I';
  gotSettingTone = false; 
  playSound('M',true);
  //_NSerial->println("ATA");
}

void SIM::sendSMS()
{
  _SSerial->flush();
  String responseString = "";
  switch (actionType)
  {
    case 'O':
      responseString = "Machine OFF";
      break;
    case 'I':
      responseString = "RPM HIGH";
      break;
    case 'D':
      responseString = "RPM LOW";
      break;
  }
  /*if (action_type == 'O')
    else if (action_type == 'I')
    else if (action_type == 'D')
  */

  //delay(350);
  #ifndef disable_debug
  _NSerial->println("Start SMS");
  #endif
  if (sendBlockingATCommand("AT+CMGF=1\r"))
  {
    if (sendBlockingATCommand("AT+CMGS=\"+917698439201\""))
    {
      sendCommand(responseString,true);
      //_SSerial->println(responseString);    // message to send
      _SSerial->write(0x1A);
      //_NSerial->println(response_chk_ret_str());
      //delay(500);
      #ifndef disable_debug
      _NSerial->println("Sent SMS");
      #endif
      sendCommand("AT+CMGF=0\r");
      //_SSerial->print("AT+CMGF=0\r"); //Normal Mode
    }
    //_SSerial->println("AT+CMGS=\"+917698439201\""); // recipient's mobile number, in international format
  }
  //_SSerial->println("AT+CMGF=1\r"); // AT command to send SMS message
  //_NSerial->println(response_chk_ret_str());
  //SIM.println("AT+CMGS=\"+91" + CurEPSettings.phones[0] + "\""); // recipient's mobile number, in international format
  //_NSerial->println(response_chk_ret_str());
  //delay(500);
}

void SIM::operateDTMF(String str)
{

  if(immediateEvent)
  {
    if(str=="1")    //yes do the operation
    {
      #ifndef disable_debug
      _NSerial->flush();
      #endif
      _SSerial->flush();
      sendCommand("AT+CHUP",true);
      //_SSerial->println("AT+CHUP");
      _SSerial->flush();
      sendImmediateFeedback(true);
      endCall();
    }
    else if(str=="2")   //no dont do anything
    {
      #ifndef disable_debug
      _NSerial->flush();
      #endif
      _SSerial->flush();
      sendCommand("AT+CHUP",true);
      //_SSerial->println("AT+CHUP");
      _SSerial->flush();
      sendImmediateFeedback(false);
      endCall();
    }
  }
  else
  {
      char r;
      if (str == "4") //"INC RPM"
      {
          r=f1();
          #ifndef disable_debug
          _NSerial->print("INC Status : ");
          _NSerial->print(r);
          _NSerial->println(r=='D');
          #endif
          if(r=='L')
            playSound('3',true);
          else if(r=='O')
            playSound('1',true);
          else if(r=='D')
            endCall();
      }
      else if (str == "3") //DEC RPM
      {
          r=f2();
          #ifndef disable_debug
          _NSerial->print("DEC Status : ");
          _NSerial->print(r);
          _NSerial->println(r=='D');
          #endif
          if(r=='L')
            playSound('4',true);
          else if(r=='O')
            playSound('1',true);
          else if(r=='D')
            endCall(); 
      }
      else if (str == "1") //MACHINE OFF
      {
          r=f3();
          #ifndef disable_debug
          _NSerial->print("OFF Status : ");
          _NSerial->print(r);
          _NSerial->println(r=='D');
          #endif
          if(r=='L')
            playSound('4',true);
          else if(r=='O')
            playSound('1',true);
          else if(r=='D')
            endCall();
      }
      #ifdef use_mega
      else if (str == "2") //MACHINE START
      {
          r=f4();
          #ifndef disable_debug
          _NSerial->print("START Status : ");
          _NSerial->print(r);
          _NSerial->println(r=='D');
          #endif
          if(r=='L')
            playSound('B',true);
          else if(r=='O')
            playSound('2',true);
          else if(r=='T')
            playSound('T',true);
          else
            endCall();
      }    
      #endif
  }
  /*  else if (str == "9")
      {
          f5();
      }
*/
      //endCall();

}

void SIM::operateRing()
{
  nr++;
  if (nr <= 3)
  {
    String str;
    do
    {
      str = readString();
    }
    while (isNumber(str) == false);
    
    if(str.length()>=10)
    {
      if (!checkNumber(str))
      {
        endCall();
      }
    }
  }
  else if(nr==4)
  {
      callCutWait = millis();
      acceptCall();     
  }
}

bool SIM::playSoundElligible()
{
  return (bplaySound == true && ((millis() - soundWait) > (soundWaitTime * 100)));
}

void SIM::triggerPlaySound()
{
  _SSerial->flush();
  sendCommand("AT+CPAMR=\"");
  sendCommand(playFile);
  sendCommand(".amr\",0\r",true);
  _SSerial->flush();
  //_SSerial->print("AT+CPAMR=\"");
  //_SSerial->print(playFile);
  //_SSerial->println(".amr\",0\r");
  soundPlayedNumber++;
  bplaySound = false;
}

void SIM::playSoundAgain(String str)
{
  int noOfTimeSoundPlays=soundPlayNumber;
  
  if(immediateEvent && sendImmediateResponse)
    noOfTimeSoundPlays*=2;

  if (isSoundStop(str))
  {
    if (soundPlayedNumber < noOfTimeSoundPlays)
    {
      if(immediateEvent && sendImmediateResponse)
      {
          if(playFile==actionType)
            playFile='A';
          else
            playFile=actionType;
      }
      playSound(playFile);
    }
    else
    {
        endCall();
    }
  }
}

void SIM::playSound(char actionType,bool init)
{
  _SSerial->flush();
  soundWait = millis();
  bplaySound = true;
  playFile = actionType;
    if(init)
      soundPlayedNumber=0;
}

void SIM::stopSound()
{
  _SSerial->flush();
  sendCommand("AT+CPAMR\r",true);
  //_SSerial->println("AT+CPAMR\r");
}

bool SIM::callTimerExpire()
{
  return (callAccepted && ((millis() - callCutWait) >= (callCutWaitTime*100)));
}

bool SIM::responseActionElligible()
{
  return (makeResponse);  //&& (millis() - responseWait) >= (responseWaitTime * 100));
}

void SIM::makeResponseAction()
{
  makeResponse = false;
  if (immediateEvent ||  responseSetting == 'A' || responseSetting == 'C')
    makeCall();
  else if (responseSetting == 'S')
    sendSMS();
}

void SIM::registerEvent(char eventType, bool immediate,bool getResponse)
{
  actionType = eventType;

  if(!initialized)
  {
    #ifndef disable_debug
    _NSerial->print("No SIM...");    
    #endif
    if(immediate && getResponse)
    {
      immediateFeedback(true);
    }
    return;
  }

  if (!immediate)
  {
    #ifndef disable_debug
    _NSerial->print("Got Event:");
    _NSerial->println(eventType);
    #endif

    responseWait = millis();
    makeResponse = true;
  }
  else
  {
    //endCall();
    //_SSerial->flush();
    freezeIncomingCalls=true;
    acceptCommands();

    #ifndef disable_debug
    _NSerial->print("Immed event:");
    _NSerial->print(eventType);
    _NSerial->print(" immediate: ");
    _NSerial->print(immediate);
    _NSerial->print(" getResponse: ");
    _NSerial->println(getResponse);
    #endif
    
    sendImmediateResponse=getResponse;
    immediateEvent=true;
    makeResponseAction();
  }
}

void SIM::sendImmediateFeedback(bool temp)
{  
    if(sendImmediateResponse)
      immediateFeedback(temp);
  
  immediateEvent=false;
  sendImmediateResponse=false;
}


void SIM::setCallBackFunctions(void (*func)(bool))
{
  immediateFeedback=*func;
}

bool SIM::rejectCommandsElligible()
{
  return (commandsAccepted && millis()-tempAcceptCommandTime>=(acceptCommandsTime*100));
}

void SIM::checkNetwork(String str)
{
  if(str=="+CPIN: NOT READY\r")
  {
    endCall();
    networkCounterMeasures();
  }
}

void SIM::networkCounterMeasures()
{
  _SSerial->flush();
  sendCommand("AT+CFUN=0\r\n");
  _SSerial->flush();
  unsigned long t=millis();
  while(millis()-t<2)
  {}
  sendCommand("AT+CFUN=1\r\n");
  _SSerial->flush();
  while(millis()-t<2)
  {}
}

void SIM::update()
{

  if(rejectCommandsElligible())
  {
    rejectCommands();
  }

  if (currentStatus == 'N')
  {
    if (responseActionElligible())
      makeResponseAction();
  }
  else if (currentStatus == 'I' || currentStatus == 'R')
  {
    if (callTimerExpire())
      endCall();

    if (playSoundElligible())
      triggerPlaySound();
  }

  while (_SSerial->available() > 0 || readBefore)
  {
    String str;
    if(readBefore)
    {
      str=bReadString;
      readBefore=false;
    }
    else
    {
      str=readString();
    }
    

    if(isRinging(str))
    {
      #ifndef disable_debug
      _NSerial->print("currentStatus:");
      _NSerial->println(currentStatus);
      _NSerial->print("currentCallStatus:");
      _NSerial->println(currentCallStatus);
      #endif
    }
    else
    {
      checkNetwork(str);
    }

    if (!freezeIncomingCalls &&  (currentStatus == 'N' || currentStatus == 'R') && (currentCallStatus == 'N' || currentCallStatus == 'I')) //Ringing Incoming Call
    {
      if (isRinging(str) == true) //  chk_ringing(str) == true)
      {
        currentStatus = 'R';
        currentCallStatus = 'I';
        operateRing();
      }
    }
    else if (!freezeIncomingCalls && currentStatus == 'I' && currentCallStatus == 'I') //IN CALL INCOMING CALL
    {
      if (isCut(str) == true ) //chk_cut(str) == true)
      {
        endCall();
      }
      else if (isDTMF(str) == true) //chk_DTMF(str) == true)
      {
        operateDTMF(str);
      }
      else
      {
        playSoundAgain(str);
      }
    }
    else if ((currentStatus == 'N' || currentStatus == 'R') && currentCallStatus == 'O')
    {
      if (callState(str) == 'R')
      {
        callCutWait = millis();
        currentStatus = 'R';
        currentCallStatus = 'O';
      }
      else if (isCut(str) || callState(str) == 'E') //else if (isCut(str) || stringContains(str, "+CLCC: 1,0,6", 11, 12) == true)
      {
        endCall();
        if(immediateEvent && sendImmediateResponse)
            sendImmediateFeedback(true);
        if (responseSetting == 'A')
            sendSMS();
      }
      else if (callState(str) == 'I') //else if (stringContains(str, "+CLCC: 1,0,0", 11, 12) == true)
      {
        #ifndef disable_debug
        _NSerial->println("Call Accepted");
        #endif
        currentStatus = 'I';
        currentCallStatus = 'O';
        callCutWait = millis();
        callAccepted = true;
        playSound(actionType,true);
      }
    }
    else if (currentStatus == 'I' && currentCallStatus == 'O') //IN CALL OUTGOING CALL
    {
      if (isCut(str) || callState(str) == 'E')
        endCall();
      else if (immediateEvent && sendImmediateResponse && isDTMF(str)) //chk_DTMF(str) == true)
      {
        operateDTMF(str);
      }
      else
      {
        playSoundAgain(str);
      }
    }
  }
  clearBRead();
}