#pragma once

#include "AbstractCommand.h"
#include "CommandDefinitions.h"

namespace commands
{

class MotorCommand : public AbstractCommand
{
public:
	MotorCommand(Motor type, int power);

	virtual bool execute();

private:
	Motor mType;
	int mPower;
};

}
