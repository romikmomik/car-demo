#include "DistanceSensorCommand.h"

using namespace commands;

DistanceSensorCommand::DistanceSensorCommand(Port port)
	: SensorCommand(port)
{
}

bool DistanceSensorCommand::execute()
{
	SensorCommand::execute();
	// TODO: read value from sensor
	int value = 0;
	answer(value);
	return true;
}

QString DistanceSensorCommand::sensorName() const
{
	return "distance sensor";
}
