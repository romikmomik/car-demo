#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "CommandFactory.h"
#include "BeepCommand.h"
#include "MotorCommand.h"
#include "DistanceSensorCommand.h"
#include "LightSensorCommand.h"

using namespace commands;

AbstractCommand *CommandFactory::parseCommand(QString const &command)
{
	QString const normalizedCommand = command.toLower().trimmed();
	QStringList const parts = normalizedCommand.split(" ", QString::SkipEmptyParts);
	if (parts.isEmpty()) {
		return 0;
	}
	AbstractCommand *result = parseCommand(parts);
	if (result) {
		result->setParent(this);
	} else {
		qDebug() << "Error while parsing command" << command;
	}
	return result;
}

AbstractCommand *CommandFactory::parseCommand(QStringList const &parts)
{
	QString const command = parts[0];

	if (command.startsWith("motor")) {
		if (parts.count() < 2) {
			qDebug() << "Motor command specified in a wrong format";
			return 0;
		}
		Motor const motor = command.endsWith("angle") ? angle : power;
		return new MotorCommand(motor, parts[1].toInt());
	}
	if (command == "beep") {
		QVector<int> params = parseIntParams(parts, 1, 3);
		if (params.count() < 2) {
			qDebug() << "Beep command specified in a wrong format";
			return 0;
		}
		return new BeepCommand(params[0], params[1]);
	}
	if (command == "distance_sensor") {
		if (parts.count() < 2) {
			qDebug() << "Read distance sensor command specified in a wrong format";
		}
		Port const port = parsePort(parts[1]);
		return new DistanceSensorCommand(port);
	}
	if (command == "light_sensor") {
		if (parts.count() < 2) {
			qDebug() << "Read light sensor command specified in a wrong format";
		}
		Port const port = parsePort(parts[1]);
		return new LightSensorCommand(port);
	}
	// TODO: more sensors
	return 0;
}

QVector<int> CommandFactory::parseIntParams(QStringList const &stringParams
		, int begin, int end)
{
	QVector<int> result;
	if (end > stringParams.length()) {
		return result;
	}
	for (int i = begin; i < end; ++i) {
		// TODO: add some validation
		QString const currentParam = stringParams[i];
		result << currentParam.toInt();
	}
}

Port CommandFactory::parsePort(QString const &port)
{
	return port == "left" ? left : right;
}
