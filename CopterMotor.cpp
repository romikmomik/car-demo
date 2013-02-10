#include "Common.hpp"
#include "CopterMotor.hpp"

void CopterMotor::invoke_open()
{
	invoke(0);
}

void CopterMotor::invoke_close()
{
	invoke(0);
}

void CopterMotor::invoke(int _power)
{
	QString s;
	m_ctrlFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered|QIODevice::Text);
	s.sprintf("%d\n", _power);
	m_ctrlFile.write(s.toLatin1());
	m_ctrlFile.close();
}

CopterMotor::CopterMotor(Settings::sptr settings, const QString& _ctrlPath, QLCDNumber* _lcd)
	:m_settings(settings),
		m_lcd(_lcd),
		m_ctrlFile(_ctrlPath),
		m_factor(1.0)
{
	invoke_open();
}

CopterMotor::~CopterMotor()
{
	invoke_close();
}

void CopterMotor::factor(double _factor)
{
	m_factor = qMax(qMin(_factor, 1.0), 0.0);
}

void CopterMotor::setPower(unsigned _power)
{
	int pwr =  floor(m_factor * _power + 0.5);
	static const auto s_power_min = m_settings->getPowerMin();
	static const auto s_power_max = m_settings->getPowerMax();
	QPalette palette = m_lcd->palette();
	QColor bg = palette.color(QPalette::Disabled, m_lcd->backgroundRole());
	double pwrSat = 1.0d - static_cast<double>(_power-s_power_min)/(s_power_max-s_power_min);
	double ftrSat = m_factor;
	bg.setBlue( bg.blue() *pwrSat);
	bg.setGreen(bg.green()*pwrSat + 0xff*(1.0-pwrSat)*ftrSat);
	bg.setRed(  bg.red()  *pwrSat + 0xff*(1.0-pwrSat)*(1-ftrSat));
	palette.setColor(QPalette::Normal, m_lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Active, m_lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Inactive, m_lcd->backgroundRole(), bg);
	m_lcd->setPalette(palette);
	m_lcd->display(pwr);

	invoke(pwr);
}


