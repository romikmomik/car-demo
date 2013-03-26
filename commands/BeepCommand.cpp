#include "BeepCommand.h"

using namespace commands;

BeepCommand::BeepCommand(int duration, int frequency)
	: mDuration(duration), mFrequency(frequency)
{
}

bool BeepCommand::execute()
{
	// TODO:
	qDebug() << "Executing beep command; duration:" << mDuration
			 << "frequency:" << mFrequency;
	return true;
}
