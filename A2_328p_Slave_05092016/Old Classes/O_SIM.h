//Version 2 Dated 29052016
#ifndef SIM_h
#define SIM_h
#include <Arduino.h>
#include <HardwareSerial.h>

class SIM
{
  private:
    bool initialized;
    
    byte soundPlayNumber;
    byte soundWaitTime; //x100 = mSec
    byte soundPlayedNumber;
    unsigned long soundWait;
    bool bplaySound;
    char playFile;

    byte callCutWaitTime;  //x100 = mSec
    unsigned long callCutWait;

    char currentStatus;
    char currentCallStatus;

    byte nr;
    char responseSetting;
    bool gotSettingTone;
    bool callAccepted;

    byte responseWaitTime;
    unsigned long responseWait;
    bool makeResponse;
    char actionType;

    bool immediateEvent;
    bool sendImmediateResponse;
    char (*f1)();
    char (*f2)();
    char (*f3)();
    char (*f4)();
    void (*f5)();
    void (*immediateFeedback)(bool);

    bool isNumber(String &str);
    bool checkNumber(String);
    bool sendBlockingATCommand(String);
    String readString();
    bool matchString(String, String);
    bool stringContains(String &sstr, String mstr, int sstart, int sstop);
    bool isRinging(String);
    bool isDTMF(String &str);
    bool isCut(String);
    bool isSoundStop(String);
    char callState(String);
    void makeCall();
    void endCall();
    void acceptCall();
    void sendSMS();
    void operateDTMF(String str);
    void operateRing();
    bool playSoundElligible();
    void triggerPlaySound();
    void playSoundAgain(String);
    void playSound(char c,bool x=false);
    void stopSound();
    bool callTimerExpire();
    bool responseActionElligible();
    void makeResponseAction();
    void sendImmediateFeedback(bool);
    HardwareSerial* _SSerial;
    HardwareSerial* _NSerial;

  public:
    SIM(HardwareSerial* serial, HardwareSerial* serial1);
    bool initialize();
    void registerEvent(char eventType, bool immediate,bool getResponse);
    void setDTMFFunctions(char (*p1)(),char (*p2)(),char (*p3)(),char (*p4)(),void (*p5)());
    void setCallBackFunctions(void (*ImdEvent)(bool));
    void update();
};
#endif