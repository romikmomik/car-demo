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

	double factor() const { return m_factor; }
	void factor(double _factor);

	void setPower(unsigned _power);

signals:
	void powerChanged(double power);

protected:
	QSettings* m_settings;
	QFile       m_ctrlFile;
	double      m_factor;
	double m_powerMax, m_powerMin; // real power, to write to ctrlFile
	void invoke_open();
	void invoke_close();
	void invoke(int _power);
};
