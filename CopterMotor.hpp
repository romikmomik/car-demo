#pragma once

#include <QLCDNumber>
#include <QFile>
#include <QSettings>

class CopterMotor : public QObject
{
	Q_OBJECT
public:
	CopterMotor(QSettings* settings, const QString& _ctrlPath);
	~CopterMotor();

	void invoke(int _power);

public slots:
	void emergencyStop();

protected:
	QSettings* m_settings;
	QFile       m_ctrlFile;
	float m_powerMax, m_powerMin; // real power, to write to ctrlFile
};
