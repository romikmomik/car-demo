#include "AbstractCommand.h"

using namespace commands;

void AbstractCommand::answer(QString const &responseString)
{
	emit response(responseString);
}
