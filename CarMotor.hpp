#pragma once

#include <QFile>
#include <QSettings>

class CarMotor : public QObject
{
	Q_OBJECT
public:
	CarMotor(int motorMin, int motorMax, const QString& _ctrlPath, const QString& name = QString());
	~CarMotor();

	void invoke(int _power);

public slots:
	void emergencyStop();

signals:
	void toLog(QString msg);

protected:
	QString m_name;
	QFile   m_ctrlFile;
	int m_power;
	int m_powerMax, m_powerMin; // real power, to write to ctrlFile
};
