#include "SensorCommand.h"

using namespace commands;

SensorCommand::SensorCommand(Port port)
	: mPort(port)
{
}

bool SensorCommand::execute()
{
	qDebug() << "Reading" << sensorName() << "on"
			<< (mPort == left ? "left" : "right") << "port";
	// Maybe some initializations common for all sensor commands here
	return true;
}

void SensorCommand::answer(int value)
{
	AbstractCommand::answer(QString::number(value));
}

QString SensorCommand::sensorName() const
{
	return "sensor";
}
