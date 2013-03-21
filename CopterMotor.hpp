#pragma once

#include <QLCDNumber>
#include <QFile>
#include <QSettings>

class CopterMotor : public QObject
{
	Q_OBJECT
public:
	CopterMotor(int motorMin, int motorMax, const QString& _ctrlPath, QSettings* settings, const QString& name = QString());
	~CopterMotor();

	void invoke(int _power);
	int power() const { return m_power; }

public slots:
	void emergencyStop();
	void setPower(int power);
	void adjustPower(int adj);

signals:
	void toLog(QString msg);

protected:
	QString m_name;
	QSettings* m_settings;
	QFile       m_ctrlFile;
	int m_power;
	int m_powerMax, m_powerMin; // real power, to write to ctrlFile
};
