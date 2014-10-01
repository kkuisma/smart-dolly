#ifndef  __PERSISTENT_PROPERTY_H
#define __PERSISTENT_PROPERTY_H

class Property
{
	public:
		Property() {}
		virtual void init(char propertyKey) {}
		virtual bool set(unsigned long newValue) = 0;
		virtual unsigned long get() const {	return value; }
		virtual void operator=(const unsigned long& newValue) = 0;
		virtual void operator=(const Property& newValue) { value = newValue.get(); }
		void operator=(const String& newValue) { set(newValue.toInt()); }
		operator unsigned long() const { return value; }
		String toString() const { return String(value); }
		char key() const { return myKey; }

	protected:
		unsigned long value;
		char myKey;
	
};

class TransientProperty : public Property
{
	public:
		TransientProperty();
		void init(char propertyKey);
		bool set(unsigned long newValue);
		void operator=(const unsigned long& newValue);
		TransientProperty& operator++();
		TransientProperty& operator+=(const unsigned long& incValue);

	private:
	
};

class PersistentProperty : public Property
{
	public:
		PersistentProperty();
		void init(char propertyKey);
		bool set(unsigned long newValue);
		void operator=(const unsigned long& newValue);

	private:
		int eepromAddr;
};

#endif