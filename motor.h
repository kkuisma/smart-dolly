#ifndef  __MOTOR_H
#define __MOTOR_H

#include <SoftwareSerial.h>
#include "connected_device.h"

namespace MotorProperties {
	const byte StepsPerUnit 	= 'A';
	const byte Speed 			= 'B'; // persistent
	const byte Ramp 			= 'C'; // persistent
	const byte PostMoveTime		= 'D'; // persistent
	const byte Direction 		= 'E'; // persistent
}

class Motor : public ConnectedDevice
{
	public:
		Motor(Stream* conn, int step, int dir, int enable, int ms1, int ms2, int ms3, int sleep, int reset);
		virtual bool get(char property);
		virtual bool set(char property, String value);
		virtual bool getResponse(char property, String value);
		virtual bool setResponse(char property, String value);

		enum Direction { LEFT = 0, RIGHT = 1 };

		char objectType() { return TargetObjects::Motor; }
		void run();
		void prepareMove();
		unsigned long getStepsPerMove() const { return stepsPerMove; }
		unsigned long getPostMoveTime() const { return postMoveTime.get(); }
		unsigned long getMotorStepped() const { return motorStepped; }
		unsigned int getMovePerCycle()	const { return movePerCycle; }
		unsigned int getMovePerStep()   const { return movePerStep; } // in tens of um (0.1 um) !!!
		unsigned long getStepsPerMm()   const { return motorStepsPerMm.get(); }
		void setDirection(Direction d) 	{ direction = d; setDirection(); }
		void setMovePerCycle(unsigned long newVal);
		void setMicroSteps(unsigned int newMicroSteps)
		{
			if(newMicroSteps == 16 || newMicroSteps == 8 || newMicroSteps == 4 || newMicroSteps == 2 || newMicroSteps == 1)
			{
				microsteps = newMicroSteps; 
				calcMotorParams();
			}
		}
		void enableMotor()	{ digitalWrite(enablePin, LOW); }
		void disableMotor()	{ digitalWrite(enablePin, HIGH); }
		void init();

	private:
		TransientProperty motorStepsPerMm;
		PersistentProperty speed;
		PersistentProperty ramp;
		PersistentProperty postMoveTime;
		PersistentProperty direction;

		unsigned int microsteps;     		// when all ms1, ms2 and ms3 are HIGH
		unsigned long usedMotorRamp;      	// steps
		unsigned int stepsPerRev;

		unsigned int motorPulse;
		unsigned long motorStepped;
		unsigned int motorPulseWidthMin;    // microseconds 
		unsigned int motorPulseWidthMax;    // microseconds
		unsigned int delayDelta;
		unsigned int movePerCycle;     		// millimeters to move per cycle
		unsigned long stepsPerMove;
		unsigned int movePerStep;			// in tens of um (0.1 um) !!!

		const int stepPin;
		const int dirPin;
		const int enablePin;
		const int sleepPin;
		const int resetPin;
		unsigned int calculateRampDelay();
		void calculateMotorPulseWidth();
		void setDirection();
		void calcMotorParams();
	
};

#endif