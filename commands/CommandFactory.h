#pragma once

#include "AbstractCommand.h"
#include "CommandDefinitions.h"

namespace commands
{

class CommandFactory : public QObject
{
public:
	AbstractCommand *parseCommand(QString const &command);

private:
	AbstractCommand *parseCommand(QStringList const &parts);

	static QVector<int> parseIntParams(QStringList const &stringParams
			, int begin, int end);

	static Port parsePort(QString const &port);
};

}
