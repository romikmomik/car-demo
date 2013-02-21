#pragma once
#include "Common.hpp"
#include "Settings.hpp"

class CopterMotor : public QObject
{
	Q_OBJECT
	Settings::sptr m_settings;
public:
	CopterMotor(Settings::sptr settings, const QString& _ctrlPath, QLCDNumber* _lcd);
	~CopterMotor();

	double factor() const { return m_factor; }
	void factor(double _factor);

	void setPower(unsigned _power);

protected:
	QLCDNumber* m_lcd;
	QFile       m_ctrlFile;
	double      m_factor;
	double m_powerMax, m_powerMin; // real power, to write to ctrlFile
	void invoke_open();
	void invoke_close();
	void invoke(int _power);
};
