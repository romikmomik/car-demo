#pragma once

#include <QtCore/QObject>
#include <QtCore/QDebug>

namespace commands
{

class AbstractCommand : public QObject
{
	Q_OBJECT

public:
	/// Returns true if success
	virtual bool execute() = 0;

	// Here can be added some methods as name() for logging and so on

signals:
	void response(QString const &result);

protected:
	void answer(QString const &responseString);

};

}
