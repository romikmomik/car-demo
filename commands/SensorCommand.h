#pragma once

#include "AbstractCommand.h"
#include "CommandDefinitions.h"

namespace commands
{

/// A base class for all sensors commands
// (maybe even the only class that takes sensor specification and reads
// corresponding driver)
class SensorCommand : public AbstractCommand
{
public:
	SensorCommand(Port port);
	virtual bool execute();

	virtual QString sensorName() const;

protected:
	void answer(int value);

	Port mPort;
};

}
