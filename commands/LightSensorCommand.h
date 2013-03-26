#pragma once

#include "SensorCommand.h"

namespace commands
{

class LightSensorCommand : public SensorCommand
{
public:
	LightSensorCommand(Port port);

	virtual bool execute();
	virtual QString sensorName() const;
};

}
