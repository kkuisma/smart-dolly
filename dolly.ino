Dolly::Dolly(Stream* conn, int rightLimit, int leftLimit, int batteryVoltageMeas) : 
ConnectedDevice(conn), limitPinRight(rightLimit), limitPinLeft(leftLimit), 
batteryVoltageMeasPin(batteryVoltageMeas),
cyclePhase(Dolly::Focus),
previousPingTime(0), pingInterval(5000), cycleWaitTime(0),
previousCycleTime(0), cycleStartTime(0), mode(Dolly::Sms)
{
	pinMode(limitPinRight, INPUT_PULLUP);
	pinMode(limitPinLeft, INPUT_PULLUP);
	pinMode(batteryVoltageMeasPin, INPUT);
	analogReference(DEFAULT);
}

void Dolly::init()
{
	motionPerCycle = motor.getMovePerCycle();

	// EEPROM addresses of properties are dependent on the init order!!!
	// Do not change the order of init's!!!
	properties[DollyProperties::State] = &state;
	properties[DollyProperties::Ping] = &pingPong;
	properties[DollyProperties::Interval] = &interval;
	properties[DollyProperties::TotalTime] = &totalTime;
	properties[DollyProperties::ElapsedTime] = &elapsedTime;
	properties[DollyProperties::TotalShots] = &totalShots;
	properties[DollyProperties::ShotsTaken] = &shotsTaken;
	properties[DollyProperties::MaxDistance] = &maxDistance;
	properties[DollyProperties::TotalDistance] = &totalDistance;
	properties[DollyProperties::CurrentPosition] = &currentPosition;
	properties[DollyProperties::MotionPerCycle] = &motionPerCycle;
	properties[DollyProperties::RightLimit] = &rightLimitSwitch;
	properties[DollyProperties::LeftLimit] = &leftLimitSwitch;
	properties[DollyProperties::Measuring] = &measuring;
	properties[DollyProperties::Homing] = &homing;
	properties[DollyProperties::BatVoltage] = &batVoltage;

	for(std::map<char, Property*>::iterator p = properties.begin(); p != properties.end(); ++p)
	{
		p->second->init(p->first);
	}
	state = Dolly::Stopped;
}

void Dolly::fastSpeed()
{
	// Use quarter microstep resolution for faster move
	digitalWrite(ms1Pin, LOW);
	digitalWrite(ms3Pin, LOW);
	motor.setMicroSteps(4);
	// We don't use deceleration, move as long as limit found
	motor.setMovePerCycle(2000);
}

void Dolly::normalSpeed()
{
	digitalWrite(ms1Pin, HIGH);
	digitalWrite(ms3Pin, HIGH);
	motor.setMicroSteps(16);
	motor.setMovePerCycle(motionPerCycle);
}

void Dolly::figureOutMaxDistance()
{
	DEBUGPRINTLN("Measuring...");
	goHome(Dolly::Fast);
	fastSpeed();
	motor.prepareMove();
	long pos = 0;
	// Find another end
	while(digitalRead(limitPinRight) == HIGH)
	{
		motor.run();
		pos += (motor.getMovePerStep());
		checkNeedForPing();
	}
	currentPosition = pos / 10000; // pos is in 0.1 um !!! Divide by 10000 to make it mm !
	maxDistance = currentPosition;
	DEBUGPRINT("MAX distance = ");
	DEBUGPRINTLN((unsigned long)maxDistance);
	sendSetRequest(currentPosition);
	sendSetRequest(maxDistance);
	// Go back to the first limit/home
	DEBUGPRINTLN("...done. Go back to home...");
	goHome(Dolly::Fast);
}

void Dolly::getOffFromLimit()
{
	motor.prepareMove();
	if(digitalRead(limitPinRight) == LOW)
	{
		motor.setDirection(Motor::LEFT);
		while(digitalRead(limitPinRight) == LOW)
		{
			motor.run();
		}
	}
	else if(digitalRead(limitPinLeft) == LOW)
	{
		motor.setDirection(Motor::RIGHT);
		while(digitalRead(limitPinLeft) == LOW)
		{
			motor.run();
		}
	}
}

void Dolly::goHome(SpeedMode speed)
{
	// NOTE! Left limit is considered as a home position!
	// And limit switch 2 should be in left side!
	DEBUGPRINTLN("Homing...");
	getOffFromLimit();
	if(speed == Dolly::Fast)
	{
		fastSpeed();
	}
	motor.setDirection(Motor::LEFT);
	motor.prepareMove();
	// Find the left limit
	while(digitalRead(limitPinLeft) == HIGH)
	{
		motor.run();
		checkNeedForPing();
	}
	normalSpeed();
	getOffFromLimit();
	currentPosition = (unsigned long)0;
	sendSetRequest(currentPosition);
}

bool Dolly::get(char propertyKey)
{
	sendGetResponse(*properties[propertyKey]);
	return true;
}

bool Dolly::set(char propertyKey, String value)
{
	Property& property = *properties[propertyKey];
	property = value;

	switch(propertyKey)
	{
		case DollyProperties::State:
			value.toInt() == 1 ? start () : stop();
			break;
		case DollyProperties::MotionPerCycle:
			motor.setMovePerCycle(motionPerCycle);
			break;
		case DollyProperties::Measuring:
			figureOutMaxDistance();
			break;
		case DollyProperties::Homing:
			goHome(Dolly::Normal);
			break;
	}

	sendSetResponse(property);
	return true;
}

bool Dolly::getResponse(char propertyKey, String value)
{
	return true;
}

bool Dolly::setResponse(char propertyKey, String value)
{
	return true;
}

bool Dolly::checkNeedForPing()
{
	bool retValue = false;
	unsigned long currentTime = millis();
	if((currentTime - previousPingTime) > pingInterval)
	{
		sendGetRequest(pingPong);
		previousPingTime = currentTime;
		DEBUGPRINTLN(previousPingTime);
		retValue = true;
	}
	return retValue;
}

void Dolly::ping()
{
	if(checkNeedForPing())
	{
		int bat = analogRead(batteryVoltageMeasPin);
		// ref voltage (measured) = 5.02V
		// voltage divider (measured) = 0.2747 - 18k and 47k resistors
		// multiplied by 1000 to make it millivolts
		float batVolt = bat * (5.02/1023.0*1000.0) / (0.2747);
		INFOPRINT("BAT:");
		batVoltage = (unsigned long)batVolt;
		INFOPRINT(batVoltage.get());
		INFOPRINTLN(" mV");
		sendSetRequest(batVoltage);
		if(batVolt < LowBatteryLimit)
		{
			stop();
		}
	}
}

void Dolly::start()
{
	state = Dolly::Running;
	shotsTaken = 0;
	elapsedTime = 0;
	lastElapsedTimeSent = 0;
	sendSetRequest(shotsTaken);
	sendSetRequest(elapsedTime);
	startTime = millis();
	previousCycleTime = startTime;
	cycleStartTime = startTime;
	camera.focusOn();
	normalSpeed();
	getOffFromLimit();
	DEBUGPRINTLN("STARTED");
}

void Dolly::stop()
{
	state = Dolly::Stopped;
	cyclePhase = Dolly::Focus;
	goHome(Dolly::Normal);
	motor.disableMotor();
	DEBUGPRINTLN("STOPPED");
}

bool Dolly::limit1Reached()
{
	unsigned long prevState = rightLimitSwitch;
	rightLimitSwitch = (digitalRead(limitPinRight) == LOW) ? 1 : 0;
	if(rightLimitSwitch != prevState)
	{
		DEBUGPRINTLN("Limit1 changed!");
		sendSetRequest(rightLimitSwitch);
	}
	return rightLimitSwitch ? true : false;
}

bool Dolly::limit2Reached()
{
	unsigned long prevState = leftLimitSwitch;
	leftLimitSwitch = (digitalRead(limitPinLeft) == LOW) ? 1 : 0;
	if(leftLimitSwitch != prevState)
	{
		DEBUGPRINTLN("Limit2 changed!");
		sendSetRequest(leftLimitSwitch);
	}
	return leftLimitSwitch ? true : false;
}

void Dolly::runCycle()
{
	if(limit1Reached() || limit2Reached()) // || currentPosition.get() >= totalDistance.get())
	{
		if(state.get() == Dolly::Running)
		{
			stop();
			sendSetRequest(state);
			return;
		}
	}

	if(state.get() != Dolly::Running)
	{
		return;
	}

	unsigned long currentTime = millis();
	unsigned long cycleTime = currentTime - previousCycleTime;

	CyclePhase oldPhase = cyclePhase;

	elapsedTime = (millis() - startTime)/1000;
	if(lastElapsedTimeSent != elapsedTime.get())
	{
		sendSetRequest(elapsedTime);
		lastElapsedTimeSent = elapsedTime;
		if(elapsedTime.get() >= totalTime.get())
		{
			stop();
			sendSetRequest(state);
		}
	}

	switch (cyclePhase) 
	{
		case Dolly::Waiting:
			if((currentTime-cycleStartTime) > interval)
			{
				cyclePhase = Dolly::Focus;
				camera.focusOn();
				previousCycleTime = currentTime;
				cycleStartTime = currentTime;
			}
			break;
		case Dolly::Focus:
			if(cycleTime > camera.getFocusTime())
			{
				cyclePhase = Dolly::Exposure;
				camera.exposureOn();
				previousCycleTime = currentTime;
			}
			break;
		case Dolly::Exposure:
			if(cycleTime > camera.getExposureTime())
			{
				cyclePhase = Dolly::PostExposure;
				camera.focusOff();
				camera.exposureOff();
				++shotsTaken;
				sendSetRequest(shotsTaken);
				previousCycleTime = currentTime;
				if((elapsedTime.get() >= totalTime.get()) || (shotsTaken.get() >= totalShots.get()))
				{
					stop();
					sendSetRequest(state);
				}
			}
			break;
		case Dolly::PostExposure:
			if(cycleTime > camera.getPostExposureTime())
			{
				cyclePhase = Dolly::Move;
				motor.prepareMove();
				previousCycleTime = currentTime;
			}
			break;
		case Dolly::Move:
			if(motor.getMotorStepped() < motor.getStepsPerMove())
			{
				motor.run();
			}
			else
			{
				cyclePhase = Dolly::PostMove;
				currentPosition += motor.getMovePerCycle();
				sendSetRequest(currentPosition);
				previousCycleTime = currentTime;
			}
			break;
		case Dolly::PostMove:
			if(cycleTime > motor.getPostMoveTime())
			{
				cyclePhase = Dolly::Waiting;
				previousCycleTime = currentTime;
			}
			break;
		default:
			break;
	}

	if(cyclePhase != oldPhase) 
	{
		printCyclePhase();
	}
}

#ifdef DEBUG
String phaseTexts[] = { "Waiting", "Focus", "Exposure", "PostExposure", "Move", "PostMove" };
#else
String phaseTexts[] = {};
#endif

void Dolly::printCyclePhase() 
{
	DEBUGPRINT(millis());
	DEBUGPRINT(": Cycle phase = ");
	DEBUGPRINTLN(phaseTexts[cyclePhase]);
}
