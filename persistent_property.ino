TransientProperty::TransientProperty()
{
}

void TransientProperty::init(char propertyKey)
{
	myKey = propertyKey;
	this->value = 0;

	DEBUGPRINT("Tra: ");
	DEBUGPRINTLN(myKey);
}

bool TransientProperty::set(unsigned long newValue)
{
	this->value = newValue;
	return true;
}

void TransientProperty::operator=(const unsigned long& newValue)
{
	set(newValue);
}

TransientProperty& TransientProperty::operator++()
{
	++(this->value);
	return *this;
}

TransientProperty& TransientProperty::operator+=(const unsigned long& incValue)
{
	this->value += incValue;
	return *this;
}

PersistentProperty::PersistentProperty()
{
	eepromAddr = 0;
}

void PersistentProperty::init(char propertyKey)
{
	myKey = propertyKey;

	eepromAddr = EEPROM.getAddress(sizeof(unsigned long));
	DEBUGPRINT("Per: ");
	DEBUGPRINT(myKey);

	this->value = EEPROM.readLong(eepromAddr);
	DEBUGPRINT(", val = ");
	DEBUGPRINTLN(this->value);
}

bool PersistentProperty::set(unsigned long newValue)
{
	if(eepromAddr > 0)
	{
		this->value = newValue;
		return EEPROM.updateLong(eepromAddr, this->value);
	}
	return false;
}

void PersistentProperty::operator=(const unsigned long& newValue)
{
	set(newValue);
}
