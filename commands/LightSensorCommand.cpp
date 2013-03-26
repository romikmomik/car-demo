#include "LightSensorCommand.h"

using namespace commands;

LightSensorCommand::LightSensorCommand(Port port)
	: SensorCommand(port)
{
}

bool LightSensorCommand::execute()
{
	SensorCommand::execute();
	// TODO: read value from sensor
	int value = 0;
	answer(value);
	return true;
}

QString LightSensorCommand::sensorName() const
{
	return "light sensor";
}
