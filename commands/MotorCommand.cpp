#include "MotorCommand.h"

using namespace commands;

MotorCommand::MotorCommand(Motor type, int power)
	: mType(type), mPower(power)
{
}

bool MotorCommand::execute()
{
	// TODO: move CopterMotor.cpp contents here or smth
	qDebug() << "Executing " << mType == power ? "power" : "angle"
			<< "motor command; power:" << mPower;
	return true;
}
