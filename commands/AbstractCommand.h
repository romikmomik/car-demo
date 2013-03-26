#pragma once

#include <QtCore/QObject>

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
	void responce(QByteArray const &result);

protected:
	void answer(QString const &responce);

};

}
