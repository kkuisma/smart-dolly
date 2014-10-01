#include <SoftwareSerial.h>
#include <pnew.cpp>
#include <EEPROMex.h>
#include "pins.h"
#include "dolly.h"
#include "camera.h"
#include "motor.h"

//#define DEBUG
#ifdef DEBUG
  #define DEBUGPRINT    Serial.print
  #define DEBUGPRINTLN  Serial.println
#else
  #define DEBUGPRINT    //
  #define DEBUGPRINTLN  //
#endif

//#define INFO
#ifdef INFO
  #define INFOPRINT    Serial.print
  #define INFOPRINTLN  Serial.println
#else
  #define INFOPRINT    //
  #define INFOPRINTLN  //
#endif

const unsigned long PersistentDataVersion = 1;

SoftwareSerial btSerial(TX_BT, RX_BT);

Dolly dolly(&btSerial, limitSwitchRightPin, limitSwitchLeftPin, batteryVoltageMeasPin);
Camera camera(&btSerial, exposurePin, focusPin);
Motor motor(&btSerial, stepPin, dirPin, enablePin, ms1Pin, ms2Pin, ms3Pin, sleepPin, resetPin);

enum RxStatus { WAIT_SYNC, WAIT_PAYLOAD };
RxStatus rxStatus = WAIT_SYNC;

void factoryReset()
{
  for(int addr = 0; addr < EEPROMSizeATmega168; addr++)
  {
    DEBUGPRINT(".");
    EEPROM.writeByte(addr, 0);
  }
}

void receivePayload(byte pSize)
{
  String payload;
  // ...and finally read the message payload
  while(pSize--)
  {
    payload += (char)btSerial.read();
  }
  processCommand(payload);
}

void receiveMessage()
{
  static byte payloadSize = 0;

  switch(rxStatus)
  {
    case WAIT_SYNC:
      if(btSerial.available() >= 6) // min msg size is 6
      {
        // Look for a sync byte...
        byte syncByte = (byte)btSerial.read();
        if(syncByte == Protocol::SyncByte)
        {
          // ...then read the length of the payload...
          payloadSize = (byte)btSerial.read();
          if(btSerial.available() < payloadSize)
          {
            rxStatus = WAIT_PAYLOAD;
          }
          else
          {
            receivePayload(payloadSize);
          }
        }
      }
      break;

    case WAIT_PAYLOAD:
      if(btSerial.available() >= payloadSize)
      {
        receivePayload(payloadSize);
        rxStatus = WAIT_SYNC;
        payloadSize = 0;
      }
      break;
  }
}

ConnectedDevice* targetObject(char objectId)
{
  ConnectedDevice* target = NULL;
  switch (objectId) {
      case TargetObjects::Dolly:
        target = &dolly;
        break;
      case TargetObjects::Camera:
        target = &camera;
        break;
      case TargetObjects::Motor:
        target = &motor;
        break;
  }
  return target;
}

void processCommand(String msg) { 
  DEBUGPRINT("<= ");
  DEBUGPRINTLN(msg);
  
  int valueSeparator = msg.indexOf(Protocol::Separator);
  String value = "";
  if(valueSeparator != -1) {
    value = msg.substring(valueSeparator+1,msg.length());
  }
  ConnectedDevice* obj = targetObject(msg[0]);
  char property = msg[1];
  char command = msg[2];

  if(obj != NULL)
  {
    switch (command)
    {
      case Commands::Get:
        obj->get(property);
        break;
      case Commands::Set:
        obj->set(property, value);
        break;
      case Commands::GetResponse:
        obj->getResponse(property, value);
        break;
      case Commands::SetResponse:
        obj->setResponse(property, value);
        break;
      default:
        // ERROR: Unknown command
        break;
    }
  }
}

void checkPersistentDataVersion()
{
  if(EEPROM.readLong(0) != PersistentDataVersion)
  {
    EEPROM.setMaxAllowedWrites(EEPROMSizeATmega168*2);
    EEPROM.setMemPool(0,EEPROMSizeATmega168);
    DEBUGPRINT("Reset EEPROM"); 
    factoryReset();
    DEBUGPRINTLN("done!");
    int eepromDataVersionAddress = EEPROM.getAddress(sizeof(unsigned long));
    DEBUGPRINT("Write EEPROM data version to: ");
    DEBUGPRINTLN(eepromDataVersionAddress);
    EEPROM.writeLong(eepromDataVersionAddress, PersistentDataVersion);
  }
  EEPROM.setMaxAllowedWrites(50);
  EEPROM.setMemPool(sizeof(unsigned long),200);
}

void setup() {
  Serial.begin(115200);
  INFOPRINTLN("Serial Monitor Connected");

  checkPersistentDataVersion();

  btSerial.begin(57600);
 
  // EEPROM addresses of properties are dependent on the init order!!!
  // Do not change the order of init's!!!
  dolly.init();
  motor.init();
  camera.init();
}

void loop() {
  receiveMessage();
  dolly.ping();
  dolly.runCycle();
}

