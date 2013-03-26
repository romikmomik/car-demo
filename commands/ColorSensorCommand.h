#pragma once

#include "SensorCommand.h"

namespace commands
{

class ColorSensorCommand : public SensorCommand
{
public:
	ColorSensorCommand();

	virtual bool execute();
	virtual QString sensorName() const;
};

}
