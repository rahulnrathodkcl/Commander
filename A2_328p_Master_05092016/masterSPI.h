#ifndef masterSPI_h
#define masterSPI_h

#include <Arduino.h>
#include "Defintions.h"

class masterSPI
{
	bool *tempLimit;
	bool *smotorEventOccured;
	byte *smotorEventByte;
	bool *selfEventOccured;
	byte *selfEventByte;
	#ifdef CHK_BATTERY
		bool *batteryLow;
	#endif
	bool *securityEvent;

	volatile bool engaged;
	volatile byte *answer;
	volatile bool *dataReceived;
	volatile byte receivedByte;

	void sendByte(byte data);
	byte receiveByte();

	#ifndef disable_debug
		#ifdef software_SIM
			HardwareSerial *_Serial;
		#else
			SoftwareSerial *_Serial;
		#endif
	#endif
	public:

		#ifndef disable_debug
			#ifdef software_SIM
				masterSPI(HardwareSerial *serial1,bool *,bool *,byte *,bool *,byte *,bool *,bool *);
			#else
				masterSPI(SoftwareSerial *serial1,bool *,bool *,byte *,bool *,byte *,bool *,bool *);
			#endif
		#else
			masterSPI(bool *,bool *,byte *,bool *,byte *,bool *,bool *);
		#endif
			
		void anotherConstructor(bool *,bool *,byte *,bool *,byte *,bool *,bool *);
		template <typename T> unsigned int SPI_writeAnything (const T& value);
		void ISR_SlaveReady();
		
		bool sendSettings(byte,float);
		bool queryState(byte ,byte *,bool *);
		bool inform(byte);
		void update();
};
#endif