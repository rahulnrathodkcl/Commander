//Version 2 Dated : 29052016
#include "SIM.h"

SIM::SIM(HardwareSerial* serial, HardwareSerial* serial1)
{
  initialized=false;

  _NSerial=serial;
  _SSerial=serial1;
 
  _NSerial->begin(115200);
  _SSerial->begin(115200);

  soundPlayNumber = 3;
  soundWaitTime = 5;
  soundPlayedNumber = 0;
  bplaySound = false;

  actionType = 'N';
  responseWaitTime = 200;
  makeResponse = false;

  callCutWaitTime = 250;
  nr = 0;
  responseSetting = 'A';
  currentStatus = 'N';
  currentCallStatus = 'N';
  gotSettingTone = false;
  callAccepted = false;
  immediateEvent=false;
  sendImmediateResponse=false;
}

  void SIM::setDTMFFunctions(char (*p1)(),char (*p2)(),char (*p3)(),char (*p4)(),void (*p5)())
  {
   f1=*p1;
   f2=*p2;
   f3=*p3;
   f4=*p4;
   f5=*p5;
  }

bool SIM::initialize()
{
  if (sendBlockingATCommand("AT\r\n"))
  {
    _NSerial->println("AT Success");
    if (sendBlockingATCommand("AT+DDET=1\r\n"))
    {
      _NSerial->println("DTMF Success");
      if (sendBlockingATCommand("AT+CLIP=1\r\n"))
      {
        _NSerial->println("CLIP Success");
        if (sendBlockingATCommand("AT+CLCC=1\r\n"))
        {
          _NSerial->println("CLCC Success");
          _NSerial->println("INIT Success");
          initialized=true;
          return true;
        }
      }
    }
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

bool SIM::sendBlockingATCommand(String cmd)
{
  //_NSerial->print(cmd);
  _SSerial->print(cmd);
  _NSerial->print(cmd);
  unsigned long t = millis();
  String str;
  while (millis() - t < 2000)
  {
    if (_SSerial->available() > 0)
    {
      str = readString();
      _NSerial->println(str);
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
    str = _SSerial->readStringUntil('\n');
    _NSerial->println(str);
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
  return (matchString(str, "NO CARRIER\r") || matchString(str, "BUSY\r") || matchString(str, "NO ANSWER\r"));
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
  //callended = false;

  //delay(200);
  //_NSerial->println("Call Made");
  //hangup_call();
  //if (!send_AT_command("ATD+91" + CurEPSettings.phones[0] + ";"))

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
    _SSerial->println("ATD+917698439201;");
    _NSerial->println("Call Made");
    callCutWait = millis();
    currentStatus = 'R';
    currentCallStatus = 'O';
  }

  //SIM.println("ATD+91" + CurEPSettings.phones[0] + ";");
  //SIM.println("ATD+919825193202;");
  //_NSerial->println(response_chk_ret_str());


  //+CLCC: 1,0,3,0,0,"+919173191310",145,""     //call made
  //+CLCC: 1,0,0,0,0,"+919173191310",145,""     //call accepted
  //+CLCC: 1,0,6,0,0,"+919173191310",145,""     //call cut
  /*  String str;
    long x=millis();
    do
    {
      str=read_str();
      if(chk_string_contains(str,"+CLCC: 1,0,3",11,12))
        x=millis();
      if((millis()-x)>20000)
        {
            hangup_call();
            break;
        }
    }
    while(chk_string(str,"NO CARRIER\r")==false && chk_string_contains(str,"+CLCC: 1,0,0",11,12)==false && chk_string_contains(str,"+CLCC: 1,0,6",11,12)==false);

    _NSerial->println("Call Status:" + str);
      if(str=="0")
      {
        x=millis();
        int played=0;
        while((millis()-x)<20000)
        {
          if (play_sound(action_type) == true)
          {

            _NSerial->println("  Voice Played " + action_type);

            do
            {
                str=read_str();
            }
            while(chk_string(str,"AMR_STOP\r")==false && chk_string_contains(str,"+CLCC: 1,0,6",11,12)==false && (millis()-x)<20000);

            if(chk_string(str,"AMR_STOP\r"))
              {
                played++;
                if(played==3)
                  break;
              }
            if(str=="6")
                break;
          }
          else
            break;
        }
      }
        done:
          hangup_call();
          callended=true;
  */


}

void SIM::endCall()
{
  nr = 0;
  _SSerial->println("ATH");
  gotSettingTone = false;
  soundPlayedNumber = 0;
  callAccepted = false;
  _SSerial->flush();
  currentStatus = 'N';
  currentCallStatus = 'N';
  
  if(immediateEvent)
    sendImmediateFeedback(true);
  //callended = true;
}

void SIM::acceptCall()
{
  _SSerial->println("ATA");
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
  _NSerial->println("Starting SMS");
  if (sendBlockingATCommand("AT+CMGF=1\r"))
  {
    if (sendBlockingATCommand("AT+CMGS=\"+917698439201\""))
    {
      _SSerial->println(responseString);    // message to send
      _SSerial->write(0x1A);
      //_NSerial->println(response_chk_ret_str());
      //delay(500);
      _NSerial->println("Sent SMS");
      _SSerial->print("AT+CMGF=0\r"); //Normal Mode
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
      sendImmediateFeedback(true);
      endCall();
    }
    else if(str=="2")   //no dont do anything
    {
      sendImmediateFeedback(false);
      endCall();
    }
    
  }
  else
  {
      char r;
      if (str == "3") //"INC RPM"
      {
          r=f1();
          if(r=='L')
            playSound('3',true);
          else if(r=='O')
            playSound('1',true);
          else
            endCall(); 
      }
      else if (str == "4") //DEC RPM
      {
          r=f2();
          if(r=='L')
            playSound('4',true);
          else if(r=='O')
            playSound('1',true);
          else
            endCall(); 
      }
      else if (str == "1") //MACHINE OFF
      {
          r=f3();
          if(r=='L')
            playSound('4',true);
          else if(r=='O')
            playSound('1',true);
          else
            endCall();
      }
      else if (str == "2") //MACHINE START
      {
          r=f4();
          if(r=='L')
            playSound('B',true);
          else if(r=='O')
            playSound('2',true);
          else if(r=='T')
            playSound('T',true);
          else
            endCall();
      }
      else if (str == "9")
      {
          f5();
      }

      //endCall();
  }
}

void SIM::operateRing()
{
  nr++;
  if (nr == 1)
  {
    String str;
    do
    {
      str = readString();
    }
    while (isNumber(str) == false);
    
    if (!checkNumber(str))
    {
      endCall();
    }
  }
  else if(nr==3)
  {
      callCutWait = millis();
      acceptCall();
      callAccepted = true;
  }
}

/*void SIM::operateRing()
{
  nr++;
  if (nr == 1)
  {
    String str;
    do
    {
      str = readString();
    }
    while (isNumber(str) == false);
    
    if (!checkNumber(str))
    {
      endCall();
    }
  }
  else if (nr == 2)
  {
    callCutWait = millis();
    acceptCall();
    callAccepted = true;
  }
}*/

bool SIM::playSoundElligible()
{
  return (bplaySound == true && ((millis() - soundWait) > (soundWaitTime * 100)));
}

void SIM::triggerPlaySound()
{
  _SSerial->print("AT+CPAMR=\"");
  _SSerial->print(playFile);
  _SSerial->println(".amr\",0\r");
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

  /*while(millis()-m<350)
    {
    }
    _SSerial->println("AT+CPAMR=");
  */
}

void SIM::stopSound()
{
  _SSerial->flush();
  _SSerial->println("AT+CPAMR\r");
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
    _NSerial->print("No SIM Connected");    
    if(immediate && getResponse)
    {
      immediateFeedback(true);
    }
    return;
  }

  if (!immediate)
  {
    _NSerial->print("Got an event : ");
    _NSerial->println(eventType);

    responseWait = millis();
    makeResponse = true;
  }
  else
  {
    _NSerial->print("Got an immediate event : ");
    _NSerial->print(eventType);
    _NSerial->print("     immediate : ");
    _NSerial->print(immediate);
    _NSerial->print("     getResponse : ");
    _NSerial->println(getResponse);
    
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

void SIM::update()
{
  if (currentStatus == 'N')
  {
    if (responseActionElligible())
      makeResponseAction();
    //chk_for_response_play();
  }
  else if (currentStatus == 'I' || currentStatus == 'R')
  {
    if (callTimerExpire())
      endCall();

    if (playSoundElligible())
      triggerPlaySound();
  }

  while (_SSerial->available() > 0)
  {

    String str = readString();
    //_NSerial->println(str);

    if ((currentStatus == 'N' || currentStatus == 'R') && (currentCallStatus == 'N' || currentCallStatus == 'I')) //Ringing Incoming Call
    {
      if (isRinging(str) == true) //  chk_ringing(str) == true)
      {
        currentStatus = 'R';
        currentCallStatus = 'I';
        operateRing();
      }
    }
    else if (currentStatus == 'I' && currentCallStatus == 'I') //IN CALL INCOMING CALL
    {
      if (isCut(str) == true) //chk_cut(str) == true)
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
      /*else if (isSoundStop() && gotSettingTone == false)
        {
        if (soundPlayedNumber == 3)
          endCall();
        else
        {
          soundPlayedNumber++;
          playSound('M');
          //if (playfile() == true)
          //  _NSerial->println("  Voice Played MM");
        }
        }*/
    }
    else if ((currentStatus == 'N' || currentStatus == 'R') && currentCallStatus == 'O')
    {
      //if (stringContains(str, "+CLCC: 1,0,3", 11, 12))
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
        _NSerial->println("Call Accepted");
        currentStatus = 'I';
        currentCallStatus = 'O';
        callAccepted = true;
        callCutWait = millis();
        //delay(100);
        playSound(actionType,true);
        //playSound(Current_Action);

        /*if (playSound(Current_Action))
          {
          soundPlayedNumber = 1;
          _NSerial->println("  Voice Played " + Current_Action);
          }
          else
          soundPlayedNumber = 0;
        */
      }
      //+CLCC: 1,0,3,0,0,"+919173191310",145,""     //call made
      //+CLCC: 1,0,0,0,0,"+919173191310",145,""     //call accepted
      //+CLCC: 1,0,6,0,0,"+919173191310",145,""     //call cut
    }
    else if (currentStatus == 'I' && currentCallStatus == 'O') //IN CALL OUTGOING CALL
    {
      //if (isCut(str) || stringContains(str, "+CLCC: 1,0,6", 11, 12) == true)
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
      /*else if (isSoundStop(str))
        {
        if (soundPlayedNumber == 3)
          endCall();
        else
        {
          soundPlayedNumber++;
          playSound(Current_Action);
          //if ()
          //  _NSerial->println("  Voice Played " + Current_Action);
        }
        }*/
    }
  }
}
