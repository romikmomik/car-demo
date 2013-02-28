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

	double delta() const { return m_delta; }
	void delta(double _factor);

	void setPower(unsigned _power);

signals:
	void powerChanged(double power);

public slots:
	void emergencyStop();

protected:
	QSettings* m_settings;
	QFile       m_ctrlFile;
	double      m_delta;
	double m_powerMax, m_powerMin; // real power, to write to ctrlFile
	void invoke_open();
	void invoke_close();
	void invoke(int _power);
};
