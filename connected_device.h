#ifndef __CONNECTED_DEVICE_H
#define __CONNECTED_DEVICE_H

#include <SoftwareSerial.h>
#include "persistent_property.h"
// STL
#include <iterator>
#include <map>

namespace TargetObjects {
	const char Dolly  = 'D';
	const char Camera = 'C';
	const char Motor  = 'M';
}

namespace Commands {
	const char Get         = 'G';
	const char Set         = 'S';
	const char GetResponse = 'H';
	const char SetResponse = 'T';
}

namespace Protocol {
	const char SyncByte    = 'X';
	const char Separator   = ':';
}

class ConnectedDevice
{
	public:
		ConnectedDevice(Stream* conn) : connection(conn) {}
		virtual bool get(char property) = 0;
		virtual bool set(char property, String value) = 0;
		virtual bool getResponse(char prop, String val) = 0;
		virtual bool setResponse(char prop, String val) = 0;

		virtual char objectType() = 0;

		void sendGetResponse(const Property& property);
		void sendSetResponse(const Property& property);
		void sendGetRequest(const Property& property);
		void sendSetRequest(const Property& property);

	protected:
		std::map<char, Property*> properties;
		static unsigned long lastMessageSent;

	private: 
		Stream* connection;

		void sendMessage(String msg);
		void sendResponse(char responseType, const Property& property);

};

#endif