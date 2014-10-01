#ifndef  __DOLLY_H
#define __DOLLY_H

#include <SoftwareSerial.h>
#include "connected_device.h"
#include "persistent_property.h"

const float LowBatteryLimit = 10.5; // Volts

// protocol definitions
// dolly properties
namespace DollyProperties {
	const byte State  			= 'A';
	const byte Ping  			= 'B';
	const byte Interval  		= 'C'; // persistent
	const byte TotalTime  		= 'D'; // persistent
	const byte ElapsedTime  	= 'E';
	const byte TotalShots  		= 'F'; // persistent
	const byte ShotsTaken  		= 'G';
	const byte MaxDistance  	= 'H'; // persistent
	const byte TotalDistance  	= 'I'; // persistent
	const byte CurrentPosition 	= 'J';
	const byte MotionPerCycle 	= 'K'; // persistent
	const byte RightLimit		= 'L';
	const byte LeftLimit 		= 'M';
	const byte Measuring		= 'N';
	const byte Homing			= 'O';
	const byte BatVoltage   	= 'P';
}

class Dolly : public ConnectedDevice
{
	public:
		Dolly(Stream* conn, int rightLimit, int leftLimit, int batteryVoltageMeas);
		virtual bool get(char property);
		virtual bool set(char property, String value);
		virtual bool getResponse(char property, String value);
		virtual bool setResponse(char property, String value);

		char objectType() { return TargetObjects::Dolly; }
		void ping();
		void runCycle();
		void init();

	private:
		enum State {
			Stopped			= 0,
			Running 		= 1,
			NotConnected	= 0xffff
		};

		enum CyclePhase { 
			Waiting 		= 0,
			Focus 			= 1,
			Exposure 		= 2,
			PostExposure	= 3,
			Move 			= 4,
			PostMove 		= 5
		} cyclePhase;
		
		enum SpeedMode {
			Normal 			= 0,
			Fast 			= 1
		};

		enum Mode { Sms, Continuous } mode;

		TransientProperty  state;
		TransientProperty  pingPong;
		PersistentProperty interval;
		PersistentProperty totalTime;
		TransientProperty  elapsedTime;
		PersistentProperty totalShots;
		TransientProperty  shotsTaken;
		PersistentProperty maxDistance;
		PersistentProperty totalDistance;
		TransientProperty  currentPosition;
		PersistentProperty motionPerCycle;
		TransientProperty  rightLimitSwitch;
		TransientProperty  leftLimitSwitch;
		TransientProperty  measuring;
		TransientProperty  homing;
		TransientProperty  batVoltage;

		const int limitPinRight;
		const int limitPinLeft;
		const int batteryVoltageMeasPin;

		unsigned long previousPingTime;
		unsigned long pingInterval;   // milliseconds
		long cycleWaitTime;
		unsigned long previousCycleTime;
		unsigned long cycleStartTime;

		void start();
		void stop();
		void printCyclePhase(); // for debug only!

		bool limit1Reached();
		bool limit2Reached();

		void figureOutMaxDistance();
		void goHome(SpeedMode speed);
		void getOffFromLimit();
		void fastSpeed();
		void normalSpeed();
		bool checkNeedForPing();

		unsigned long startTime;
		unsigned long lastElapsedTimeSent;
};

#endif