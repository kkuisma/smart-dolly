Motor::Motor(Stream* conn, int step, int dir, int enable, int ms1, int ms2, int ms3, int sleep, int reset) :
  ConnectedDevice(conn), stepPin(step), dirPin(dir), enablePin(enable), sleepPin(sleep), resetPin(reset),
  movePerCycle(0), stepsPerMove(0), movePerStep(0)
{
  pinMode(stepPin,  OUTPUT);
  pinMode(dirPin,   OUTPUT);
  pinMode(enablePin,OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(ms1Pin,   OUTPUT);
  pinMode(ms2Pin,   OUTPUT);
  pinMode(ms3Pin,   OUTPUT);

  digitalWrite(stepPin,  LOW);
  digitalWrite(dirPin,   LOW);
  digitalWrite(enablePin,HIGH);
  digitalWrite(sleepPin, HIGH);
  digitalWrite(resetPin, HIGH);
  digitalWrite(ms1Pin,   HIGH);
  digitalWrite(ms2Pin,   HIGH);
  digitalWrite(ms3Pin,   HIGH);
}

void Motor::init()
{
  // EEPROM addresses of properties are dependent on the init order!!!
  // Do not change the order of init's!!!
  properties[MotorProperties::StepsPerUnit] = &motorStepsPerMm;
  properties[MotorProperties::Speed] = &speed;
  properties[MotorProperties::Ramp] = &ramp;
  properties[MotorProperties::PostMoveTime] = &postMoveTime;
  properties[MotorProperties::Direction] = &direction;

  for(std::map<char, Property*>::iterator p = properties.begin(); p != properties.end(); ++p)
  {
    p->second->init(p->first);
  }

  microsteps = 16;     // when all ms1, ms2 and ms3 are HIGH
  calcMotorParams();
  setDirection();
}

void Motor::calcMotorParams()
{
  usedMotorRamp      = 0;      // steps
  stepsPerRev        = 200 * microsteps;
  motorStepsPerMm    = stepsPerRev / 40; // with the OpenBuilds GT2 / 20 teeth pulley
  movePerStep        = 10000 / motorStepsPerMm.get(); // 0.1 um !!!
  motorPulse         = 10;
  calculateMotorPulseWidth();
  motorPulseWidthMax = 4200;    // microseconds
  delayDelta         = 0;
}

void Motor::setMovePerCycle(unsigned long newVal)
{
  movePerCycle = newVal;
  stepsPerMove = motorStepsPerMm * movePerCycle;
}

void Motor::calculateMotorPulseWidth()
{
  motorPulseWidthMin = 1000000/(speed.get() * 3 * motorStepsPerMm.get()) - motorPulse;
  DEBUGPRINT("Motor pulse width = ");
  DEBUGPRINTLN(motorPulseWidthMin);
}

void Motor::setDirection()
{
  digitalWrite(dirPin, (direction.get() == RIGHT) ? LOW : HIGH);
}

bool Motor::get(char propertyKey)
{
  sendGetResponse(*properties[propertyKey]);
	return true;
}

bool Motor::set(char propertyKey, String value)
{
  Property& property = *properties[propertyKey];

  property = value;
  if(propertyKey == MotorProperties::Direction)
  {
    setDirection();
  }
  else if(propertyKey == MotorProperties::Speed)
  {
    calculateMotorPulseWidth();
  }
  
  sendSetResponse(property);
  return true;
}

bool Motor::getResponse(char propertyKey, String value)
{
  return true;
}

bool Motor::setResponse(char propertyKey, String value)
{
  return true;
}

void Motor::run()
{
  unsigned int motorPulseWidth = calculateRampDelay();
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motorPulse);
  digitalWrite(stepPin, LOW);
  if(motorPulseWidth > 0) 
  {
    delayMicroseconds(motorPulseWidth);
  }
  motorStepped++;
}

void Motor::prepareMove() 
{
  enableMotor();
  motorStepped = 0;

  if (ramp > (stepsPerMove >> 1))
  {
    usedMotorRamp = stepsPerMove >> 1;
  }
  else 
  {
    usedMotorRamp = ramp;
  }
 
  if (usedMotorRamp == 0) 
  {
    usedMotorRamp = 1;
  }
    
  float ramp_fac = (float) usedMotorRamp / (float) ramp.get();
  ramp_fac = sqrt(sqrt(ramp_fac));
  delayDelta = ramp_fac * (float) (motorPulseWidthMax - motorPulseWidthMin);
  DEBUGPRINT("Delay delta = ");
  DEBUGPRINTLN(delayDelta);
}

unsigned int Motor::calculateRampDelay()
{
  unsigned int res;
  float        xscale;
  
  // acceleration
  if (motorStepped < usedMotorRamp)
  {
    xscale = (float) motorStepped / (float) usedMotorRamp;
  } 
  // deceleration
  else if (motorStepped > (stepsPerMove - usedMotorRamp))
  {
    xscale = (float) (stepsPerMove - motorStepped) / (float) usedMotorRamp;
  }
  // max speed
  else
  {    
    delayMicroseconds(200);
    return motorPulseWidthMin; 
  }
  res = (atan(15.0 * xscale) / 1.51) * delayDelta;
  return motorPulseWidthMax - res;
}
