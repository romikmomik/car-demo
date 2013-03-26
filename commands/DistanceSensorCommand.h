#pragma once

#include "SensorCommand.h"

namespace commands
{

class DistanceSensorCommand : public SensorCommand
{
public:
	DistanceSensorCommand(Port port);

	virtual bool execute();
	virtual QString sensorName() const;
};

}
