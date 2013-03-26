#pragma once

#include "AbstractCommand.hpp"

namespace commands
{

class BeepCommand : public AbstractCommand
{
public:
	BeepCommand(int duration, int frequency);

	virtual bool execute();

private:
	int mDuration;
	int mFrequency;
};

}
