#include "S_EEPROM.h"

void S_EEPROM::saveHighRPMSettings(unsigned short int HRPM)
{
  RPM=HRPM;
  EEPROM.put(RPMAddress, RPM);
}

void S_EEPROM::saveCompRPMSettings(unsigned short int CRPM)
{
  COMPRPM=CRPM;
  EEPROM.put(compRPMAddress,COMPRPM);  
}

void S_EEPROM::saveDecompRPMSettings(unsigned short int DRPM)
{
  DECOMPRPM=DRPM;
  EEPROM.put(decompRPMAddress, DECOMPRPM);
}

void S_EEPROM::saveMotorLowSettings(unsigned short int mLow)
{
  MOTORLOW=mLow;
  EEPROM.put(motorLowAddress,MOTORLOW);  
}

void S_EEPROM::saveMotorHighSettings(unsigned short int mHigh)
{
  MOTORHIGH=mHigh;
  EEPROM.put(motorHighAddress,MOTORHIGH);
}

void S_EEPROM::saveTempSettings(unsigned short int temp)
{
  EEPROM.put(highTempAddress,temp);
  EEPROM.get(highTempAddress,HIGHTEMP);
}

S_EEPROM::S_EEPROM()
{
  numbersCount = 0;
}

byte S_EEPROM::checkExists(String number)
{
  if (numbersCount > 0)
  {
    if (primaryNumber == number)
    {
      return 0;
    }

    for (byte i = 0; i < numbersCount - 1; i++)
    {
      if (secondary[i] == number)
      {
        return i + 1;
      }
    }
  }
  return 0xFF;
}

bool S_EEPROM::addNumber(String number, bool admin)
{
  if (number.length() == 10)
  {
    if (numbersCount == 5)
      return false;
    else
    {
      if (checkExists(number) == 0xFF)
      {
        if (numbersCount == 0)
          primaryNumber = number;
        else if (numbersCount < 5)
          secondary[numbersCount - 1] = number;

        numbersCount++;
        updateNumberChanges();
        return true;
      }
    }
  }
  return false;
}

bool S_EEPROM::removeNumber(String number, bool admin)
{
  if (number.length() == 10)
  {
    if (numbersCount < 2)
      return false;
    else
    {
      byte loc=checkExists(number);
      if(loc!=0xFF && loc!=0x00)
      {
          secondary[loc - 1] = "";
          for (byte i = loc; i < numbersCount; i++)
          {
            secondary[i - 1] = secondary[i];
          }
          numbersCount--;
          updateNumberChanges();
          return true;        
      }
    }
  }
  return false;
}

void S_EEPROM::updateNumberChanges()
{
  EEPROM.put(numbersCountAddress, numbersCount);
  if (numbersCount > 0)
  {
    write_StringEE(mobileNumberAddress, primaryNumber);
    for (byte temp = 1; temp < numbersCount; temp++)
    {
      write_StringEE(mobileNumberAddress + (11 * temp), secondary[temp - 1]);
    }
  }
}

void S_EEPROM::loadRPMSettings()
{
  EEPROM.get(RPMAddress,RPM);
  EEPROM.get(compRPMAddress,COMPRPM);
  EEPROM.get(decompRPMAddress,DECOMPRPM);

  if (RPM == 0xFFFF)
    saveHighRPMSettings(700);
  if(COMPRPM==0xFFFF)
    saveCompRPMSettings(160);
  if(DECOMPRPM==0xFFFF)
    saveDecompRPMSettings(130);
}

void S_EEPROM::loadTempSettings()
{
  EEPROM.get(highTempAddress,HIGHTEMP);
  if(HIGHTEMP==0xFFFF)
    saveTempSettings(50);
}

void S_EEPROM::loadMotorSettings()
{
  EEPROM.get(motorLowAddress,MOTORLOW);
  EEPROM.get(motorHighAddress,MOTORHIGH);
  if(MOTORLOW==0xFFFF)
    saveMotorLowSettings(65);
  if(MOTORHIGH==0xFFFF)
    saveMotorHighSettings(348);
}

void S_EEPROM::loadNumbers()
{
  EEPROM.get(numbersCountAddress, numbersCount);

  if (numbersCount > 0 && numbersCount != 0xFF)
  {
    primaryNumber = read_StringEE(mobileNumberAddress, 11);
    for (byte temp = 1; temp < numbersCount; temp++)
    {
      secondary[temp - 1] = read_StringEE(mobileNumberAddress + (11 * temp), 10);
    }
  }
  else
  {
    clearLoadedNumbers();
  }
}

void S_EEPROM::clearLoadedNumbers()
{
  numbersCount = 0;
  primaryNumber = "";
  for (byte temp = 0; temp < 4; temp++)
  {
    secondary[temp] = "";
  }
}

void S_EEPROM::saveForceStartSettings(bool temp)
{
  FORCESTART=temp;
  EEPROM.put(forceStartAddress,FORCESTART);
}

void S_EEPROM::loadForceStartSettings()
{
  FORCESTART=EEPROM.get(forceStartAddress,FORCESTART);
  if(FORCESTART==0xFF)
    saveForceStartSettings(true);
}

void S_EEPROM::loadAllData()
{
  loadForceStartSettings();
  loadRPMSettings();
  loadTempSettings();
  loadMotorSettings();
  loadNumbers();
}

void S_EEPROM::clearNumbers(bool admin = false)
{
  word i;
  byte temp;
  if (!admin)
  {
    i = mobileNumberAddress + 11;
    temp = 44;
    numbersCount = 1;
  }
  else
  {
    i = mobileNumberAddress;
    temp = 55;
    numbersCount = 0;
    primaryNumber = "";
  }
  for (int i = 0; i < 4; i++)
    secondary[i] = "";

  for (; i <= temp; i++)
  {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.put(numbersCountAddress, numbersCount);
  //EEPROM_MAX_ADDR 1023
}

bool S_EEPROM::write_StringEE(int Addr, String input)
{
  char cbuff[input.length() + 1]; //Finds length of string to make a buffer
  input.toCharArray(cbuff, input.length() + 1); //Converts String into character array
  return eeprom_write_string(Addr, cbuff); //Saves String
}

String S_EEPROM::read_StringEE(int Addr, int length)
{
  char cbuff[length + 1];
  eeprom_read_string(Addr, cbuff, length + 1);

  String stemp(cbuff);
  return stemp;
}

bool S_EEPROM::eeprom_read_string(int addr, char* buffer, int bufSize) {
  byte ch; // byte read from eeprom
  int bytesRead; // number of bytes read so far

  if (!eeprom_is_addr_ok(addr)) { // check start address
    return false;
  }

  if (bufSize == 0) { // how can we store bytes in an empty buffer ?
    return false;
  }

  // is there is room for the string terminator only, no reason to go further
  if (bufSize == 1) {
    buffer[0] = 0;
    return true;
  }

  bytesRead = 0; // initialize byte counter
  ch = EEPROM.read(addr + bytesRead); // read next byte from eeprom
  buffer[bytesRead] = ch; // store it into the user buffer
  bytesRead++; // increment byte counter

  // stop conditions:
  // - the character just read is the string terminator one (0x00)
  // - we have filled the user buffer
  // - we have reached the last eeprom address
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) {
    // if no stop condition is met, read the next byte from eeprom
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch; // store it into the user buffer
    bytesRead++; // increment byte counter
  }

  // make sure the user buffer has a string terminator, (0x00) as its last byte
  if ((ch != 0x00) && (bytesRead >= 1)) {
    buffer[bytesRead - 1] = 0;
  }

  return true;
}

bool S_EEPROM::eeprom_write_string(int addr, const char* str)
{
  int numBytes; // actual number of bytes to be written
  numBytes = strlen(str) + 1;
  return eeprom_write_bytes(addr, (const byte*)str, numBytes);
}

bool S_EEPROM::eeprom_is_addr_ok(int addr)
{
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

bool S_EEPROM::eeprom_write_bytes(int startAddr, const byte* array, int numBytes)
{
  int i;
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes))
    return false;

  for (i = 0; i < numBytes; i++)
  {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}