
#include "slaveSPI.h"
#include "Definitions.h"

slaveSPI::slaveSPI(void (*startSequenceQuery)(),S_EEPROM *eeprom1)
{
	// _Serial=s1;
	// _Serial->begin(19200);
	this->startSequenceQuery=*startSequenceQuery;
	//
	this->eeprom1=eeprom1;
	pinMode(PIN_INTERRUPTMASTER,OUTPUT);
	SPCR |= bit (SPE);	//enable SPI slave mode
  	pinMode(MISO, OUTPUT);

	PCICR |= (1 <<PCIE0);
	PCMSK0 |= (1<<PCINT2);
	SPI.attachInterrupt();
}

#ifdef CHK_BATTERY
	void slaveSPI::setObjectReference(SELF *self1,SMOTOR_R *smotor1,TEMP *temp1,BATTERY *batteryLevel)
	{
		this->temp1=temp1;
		this->smotor1=smotor1;
		this->self1=self1;	
		this->batteryLevel=batteryLevel;
	}

#else
	void slaveSPI::setObjectReference(SELF *self1,SMOTOR_R *smotor1,TEMP *temp1)
	{
		this->temp1=temp1;
		this->smotor1=smotor1;
		this->self1=self1;	
	}
#endif
// void slaveSPI::setInformFunction(void(*iSelf)(byte),void (*iLimit)(byte),void (*ismotor)(byte))
// {
// //	informSelf=*iSelf;
// //	informLimit=*iLimit;
// //	informSmotor=*ismotor;
// }

// void slaveSPI::setQueryFunction(void(*startSeq)(),void(*Temp)(),void (*smotor)(),void(*selfLimit)(),void (*selfMachine)())
// {
// //	startSequenceQuery=*startSeq;
// //	tempQuery=*temp;
// //	smotorQuery=*smotor;
// //	selfLimitQuery=*selfLimit;
// //	selfMachineQuery=*selfMachine;
// }

template <typename T> unsigned int slaveSPI::SPI_readAnything(T& value)
{
	byte * p = (byte*) &value;
	unsigned int i;
	for (i = 0; i < sizeof value; i++)
		*p++ = SPI.transfer (0);
	return i;
}  

void slaveSPI::receiveInterrupt()
{
	byte temp = SPDR;
	if(temp==Q_EVENT)
	{
		digitalWrite(PIN_INTERRUPTMASTER,LOW);
		// _Serial->println("Querying For Data");
		// _Serial->println(SPDR);	
		return;
	}

	byte temp2=temp>>4;
	
	if(temp2==CHKINCOMING_SETTING)
	{
		SPI_readAnything(settingData);

		settingByte=temp;
		settingReceived=true;
	}
	else
	{
		spiDataReceived=true;
		spiData=SPDR;
		digitalWrite(PIN_INTERRUPTMASTER,LOW);
	}
}  // end of interrupt routine SPI_STC_vect

void slaveSPI::operateOnSPIData()
{

	// _Serial->print("Data Received:");
	// _Serial->println(spiData);

	spiDataReceived=false;
	byte temp=spiData >> 4;
	switch(temp)
	{
		case CHKINCOMING_QUERY:	//query
			switch(spiData)
			{
				case Q_TEMP:
					temp1->query();
					break;
				case Q_LIMIT:
					self1->limitQuery();
					break;
				case Q_MACHINE:
					self1->machineQuery();
					break;
				case Q_MOTOR:
					smotor1->query();
					break;
				case Q_STARTSEQUENCE:
					startSequenceQuery();
					break;
				case Q_SLAVEEXISTENCE:
					slaveExistenceQuery=true;
					break;
				case Q_BATTERY:
					batteryLevel->query();
					break;
				//#ifdef CHK_BATTERY
				//#endif
			}
		break;
		case CHKINFORM_SELF:
			self1->informSelf(spiData);
			#ifdef CHK_BATTERY
			batteryLevel->inform(spiData);
			#endif
		break;
		case CHKINFORM_LIMIT:
			self1->informLimit(spiData);
		break;
		case CHKINFORM_MOTORRP:
			smotor1->inform(spiData);
		break;
	}
}

void slaveSPI::operateOnSetting()
{
	settingReceived=false;
	unsigned short int temp=settingData;
	switch(settingByte)
	{
		case S_HIGHRPM:
			eeprom1->saveHighRPMSettings(temp);
		break;
		case S_COMPRPM:
			eeprom1->saveCompRPMSettings(temp);
		break;
		case S_DECOMPRPM:
			eeprom1->saveDecompRPMSettings(temp);
		break;
		case S_HIGHTEMP:
			eeprom1->saveTempSettings(temp);
			break;
		case S_MOTORRHIGH:
			eeprom1->saveMotorHighSettings(temp);
		break;
		case S_MOTORRLOW:
			eeprom1->saveMotorLowSettings(temp);
		break;
		case S_FORCESTART:
			eeprom1->saveForceStartSettings((byte)temp);
		break;
	}
}

void slaveSPI::sendData(byte data)
{
	SPDR=data;
	digitalWrite(PIN_INTERRUPTMASTER,HIGH);
}

void slaveSPI::update()
{
	if(slaveExistenceQuery)
	{
		slaveExistenceQuery=false;
		// _Serial->println("Got Master Init Query..");
		sendData(A_SLAVEEXISTS);
	}

	if(spiDataReceived)
		operateOnSPIData();

	if(settingReceived)
		operateOnSetting();

}
//void slaveSPI::answerQuery(byte data)
//{
//	sendData(data);
//}