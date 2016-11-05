#ifndef DEF_h
#define DEF_h

#define CHK_BATTERY
// SELF

#define LIMITREADINGTIME 25

#define PIN_RPMSEN 3
#define PIN_CMOTORPWR 1
#define PIN_DECOMP 2

#define PIN_INTERRUPTMASTER 7
#define PIN_BATTERYPWR 8
#define PIN_BATTERYSEN A2

//SEC_LIMIT || DIESEL
#define PIN_SECLIMITPWR 0
#define PIN_SECLIMITSIG A5

// SMOTOR
#define PIN_SMOTORPWR A4
#define PIN_SMOTORSIG A3

// TEMP
#define PIN_TEMPSEN 4
#define TEMPWAITTIME 30000L
#define GETTEMPWAITTIME 1500

// EEPROM
#define RPMAddress 0
#define numbersCountAddress 4
#define mobileNumberAddress 8
#define compRPMAddress 80
#define decompRPMAddress 84
#define highTempAddress 90
#define motorHighAddress 94
#define motorLowAddress 98
#define forceStartAddress 102

#define EEPROM_MIN_ADDR 0
#define EEPROM_MAX_ADDR 1023


	#define S_COMPRPM 0x70
	#define S_DECOMPRPM 0x71
	#define S_HIGHRPM 0x72
	#define S_HIGHTEMP 0x73
	#define S_MOTORRHIGH 0x74
	#define S_MOTORRLOW 0x75
	#define S_FORCESTART 0x76


	#define Q_TEMP 0x60
	#define Q_LIMIT 0x61
	#define Q_MOTOR 0x62
	#define Q_MACHINE 0x63
	#define Q_STARTSEQUENCE 0x64
	#define Q_EVENT 0x65
	#define Q_SLAVEEXISTENCE 0x66
	#define Q_BATTERY 0x67
	
	#define A_VALIDTEMP 0x90
	#define A_INVALIDTEMP 0x91
	#define A_LIMITCOMPRESS 0x92
	#define A_LIMITDECOMPRESS 0x93
	#define A_MOTORLOW 0x94
	#define A_MOTORHIGH 0x95
	#define A_MOTORNORMAL 0x96
	#define A_MACHINEON 0x97
	#define A_MACHINEOFF 0x98
	#define A_STARTYES 0x99
	#define A_STARTTEMP 0x9A
	#define A_STARTCOMPRESS 0x9B
	#define A_STARTON 0x9C
	#define A_MOTORPREVIOUSSTATE 0x9D //used in slave
	#define A_SLAVEEXISTS 0x9E
	#define A_BATTERYOK 0x9F
	#define A_BATTERYLOW 0x80
		

	#define I_SELFSTARTED 0x30
	#define I_SELFSTOPPED 0x31
	#define I_LIMITPON 0x40
	#define I_LIMITPOFF 0x41
	#define I_LIMITCOMPRESS 0x42
	#define I_LIMITDECOMPRESS 0x43
	#define I_MOTORRPON 0x50
	#define I_MOTORRPOFF 0x51
	#define I_MOTORSWOFF 0x52
	#define I_MOTORBOFF 0x53
	
	#define EVENT_NORPM 0xAF
	#define EVENT_COMPRPM 0xAA
	#define EVENT_DECOMPRPM 0xAB
	#define EVENT_HIGHRPM 0xAC
	#define EVENT_STARTRPM 0xAD
	#define EVENT_FORCESTARTED 0xAE

	#define EVENT_HIGHTEMP 0xBA
	#define EVENT_LIMITCOMP 0xCA
	#define EVENT_LIMITDECOMP 0xCB
	#define EVENT_MOTORHIGH 0xDA
	#define EVENT_MOTORLOW 0xDB
	#define EVENT_MOTORPREVIOUSSTATE 0xDC
	#define EVENT_BATTERYLOW 0xEA
	#define EVENT_SECURITY 0xFA
 
	// only for master
	#define CHKEVENT_RPM 0x0A
	#define CHKEVENT_TEMP 0x0B
	#define CHKEVENT_LIMIT 0x0C
	#define CHKEVENT_MOTOR 0x0D
	#define CHKEVENT_BATTERY 0x0E
	#define CHKEVENT_SECURITY 0x0F
	#define CHKINCOMING_ANSWER 0x09

	// only for slave
	#define CHKINCOMING_SETTING 0x07
	#define CHKINCOMING_QUERY 0x06
	#define CHKINFORM_SELF 0x03
	#define CHKINFORM_LIMIT 0x04
	#define CHKINFORM_MOTORRP 0x05
#endif