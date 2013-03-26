#include "AbstractCommand.h"

using namespace commands;

void AbstractCommand::answer(QString const &responce)
{
	emit responce(responce.toLatin1());
}
