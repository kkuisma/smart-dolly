Camera::Camera(Stream* conn, int exposure, int focus) : exposurePin(exposure), focusPin(focus),
	ConnectedDevice(conn)
{
	pinMode(exposurePin, OUTPUT);
	digitalWrite(exposurePin, LOW);

	pinMode(focusPin, OUTPUT);
	digitalWrite(focusPin, LOW);
}

void Camera::init()
{
	// EEPROM addresses of properties are dependent on the init order!!!
	// Do not change the order of init's!!!
	properties[CameraProperties::ExposureTime] = &exposureTime;
	properties[CameraProperties::PostExposureTime] = &postExposureTime;
	properties[CameraProperties::FocusTime] = &focusTime;
	properties[CameraProperties::ExposureMode] = &exposureMode;
	properties[CameraProperties::FocalLength] = &focalLength;

	for(std::map<char, Property*>::iterator p = properties.begin(); p != properties.end(); ++p)
	{
		p->second->init(p->first);
	}
}

bool Camera::get(char propertyKey)
{
	sendGetResponse(*properties[propertyKey]);
	return true;
}

bool Camera::set(char propertyKey, String value)
{
	Property& property = *properties[propertyKey];
   	property = value;
	sendSetResponse(property);
	return true;
}

bool Camera::getResponse(char propertyKey, String value)
{
	return true;
}

bool Camera::setResponse(char propertyKey, String value)
{
	return true;
}
