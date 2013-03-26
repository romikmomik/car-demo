#include "ColorSensorCommand.h"

using namespace commands;

ColorSensorCommand::ColorSensorCommand()
{
}

bool ColorSensorCommand::execute()
{
	SensorCommand::execute();
	// TODO: read value from sensor
	int value = 0;
	answer(value);
	return true;
}

QString ColorSensorCommand::sensorName() const
{
	return "color senor";
}
