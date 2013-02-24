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
	double powerFactor = (double)(m_powerMax - m_powerMin) / 100;
	s.sprintf("%lf\n", _power * powerFactor + m_powerMin);
	m_ctrlFile.write(s.toLatin1());
	m_ctrlFile.close();
}

CopterMotor::CopterMotor(Settings::sptr settings, const QString& _ctrlPath) :
	m_settings(settings),
	m_ctrlFile(_ctrlPath),
	m_factor(1.0)
{
	m_powerMin = settings->getMotorMin();
	m_powerMax = settings->getMotorMax();

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
	invoke(pwr);
	emit powerChanged(static_cast<double>(pwr));
}


