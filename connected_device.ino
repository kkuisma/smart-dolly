unsigned long ConnectedDevice::lastMessageSent = 0;

void ConnectedDevice::sendMessage(String msg)
{
	byte msgLen = msg.length();

	if(msgLen < 256)
	{
		connection->write(Protocol::SyncByte);
		connection->write(msgLen+1);
		connection->write(objectType());
		connection->print(msg);
	
		DEBUGPRINT("=> ");
		DEBUGPRINT(Protocol::SyncByte);
		DEBUGPRINT(msgLen+1);
		DEBUGPRINT(objectType());
		DEBUGPRINTLN(msg);
	}
	ConnectedDevice::lastMessageSent = millis();
}

void ConnectedDevice::sendResponse(char responseType, const Property& property)
{
	String msg;
	msg += property.key();
	msg += responseType;
	msg += Protocol::Separator;
	msg += property.toString();
	INFOPRINT(" - MSG=");
	INFOPRINTLN(msg);
	sendMessage(msg);
}

void ConnectedDevice::sendSetResponse(const Property& property)
{
	sendResponse(Commands::SetResponse, property);
}

void ConnectedDevice::sendGetResponse(const Property& property)
{

	sendResponse(Commands::GetResponse, property);
}

void ConnectedDevice::sendGetRequest(const Property& property)
{
	String msg = String(property.key());
	msg += Commands::Get;
	msg += Protocol::Separator;
	sendMessage(msg);
}

void ConnectedDevice::sendSetRequest(const Property& property)
{
	String msg = String(property.key());
	msg += Commands::Set;
	msg += Protocol::Separator;
	msg += property.toString();
	sendMessage(msg);
}
