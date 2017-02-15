#ifndef S_EEPROM_h
#define S_EEPROM_h

#include <Arduino.h>
#include <String.h>
#include <EEPROM.h>
#include "Definitions.h"

class S_EEPROM
{
    void loadRPMSettings();
    void loadMotorSettings();
    void loadTempSettings();
    void loadForceStartSettings();
    void loadNumbers();
    void clearLoadedNumbers();

    
    void updateNumberChanges();
    bool write_StringEE(int Addr, String input);
    String read_StringEE(int Addr, int length);
    bool eeprom_read_string(int addr, char* buffer, int bufSize);
    bool eeprom_write_string(int addr, const char* str);
    bool eeprom_is_addr_ok(int addr);
    bool eeprom_write_bytes(int startAddr, const byte* array, int numBytes);

  public:
    byte numbersCount;
    bool machineOn;
    bool gotMasterQuery;
    
    unsigned short int RPM;
    unsigned short int COMPRPM;
    unsigned short int DECOMPRPM;
    unsigned short int MOTORLOW;
    unsigned short int MOTORHIGH;
    unsigned short int HIGHTEMP;
    byte FORCESTART;
    
    String primaryNumber;
    String secondary[4];

    S_EEPROM();
    void saveHighRPMSettings(unsigned short int);
    void saveCompRPMSettings(unsigned short int);
    void saveDecompRPMSettings(unsigned short int);
    void saveMotorLowSettings(unsigned short int);
    void saveMotorHighSettings(unsigned short int);
    void saveForceStartSettings(bool);
    
    void saveTempSettings(unsigned short int temp);

    byte checkExists(String number);
    
    void loadAllData();

    bool addNumber(String number, bool admin=false);
    bool removeNumber(String number, bool admin=false);
    void clearNumbers(bool admin);
};
#endif
