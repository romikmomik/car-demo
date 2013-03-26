#pragma once

#include "SensorCommand.h"

namespace commands
{

class LightSensorCommand : public SensorCommand
{
public:
	LightSensorCommand();

	virtual bool execute();
	virtual QString sensorName() const;
};

}
