#ifndef  __CAMERA_H
#define __CAMERA_H

#include <SoftwareSerial.h>
#include "connected_device.h"

namespace CameraProperties {
	const byte ExposureTime 	= 'A'; // persistent
	const byte PostExposureTime = 'B'; // persistent
	const byte FocusTime		= 'C'; // persistent
	const byte ExposureMode 	= 'D'; // persistent
	const byte FocalLength 		= 'E'; // persistent
}

class Camera : public ConnectedDevice
{
	enum ExpMode { Bulb = 0, CameraMode = 1 };

	public:
		Camera(Stream* conn, int exposure, int focus);
		virtual bool get(char property);
		virtual bool set(char property, String value);
		virtual bool getResponse(char property, String value);
		virtual bool setResponse(char property, String value);

		char objectType() { return TargetObjects::Camera; }

		unsigned long getExposureTime() { return exposureTime.get(); }
		unsigned long getPostExposureTime() { return postExposureTime.get(); }
		unsigned long getFocusTime() { return focusTime.get(); }

		void init();
		void focusOn()		{ digitalWrite(focusPin, HIGH); }
		void focusOff()		{ digitalWrite(focusPin, LOW); }
		void exposureOn()	{ digitalWrite(exposurePin, HIGH); }
		void exposureOff()	{ digitalWrite(exposurePin, LOW); }

	private:
		PersistentProperty exposureTime;
		PersistentProperty postExposureTime;
		PersistentProperty focusTime;
		PersistentProperty exposureMode;
		PersistentProperty focalLength;

		const int exposurePin;
		const int focusPin;
	
};

#endif