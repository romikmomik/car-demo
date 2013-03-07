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
	float powerFactor = (float)(m_powerMax - m_powerMin) / 100;
	s.sprintf("%d\n", static_cast<int>(_power * powerFactor + m_powerMin));
	m_ctrlFile.write(s.toLatin1());
	m_ctrlFile.close();
}

CopterMotor::CopterMotor(QSettings* settings, const QString& _ctrlPath) :
	m_settings(settings),
	m_ctrlFile(_ctrlPath),
	m_delta(1.0)
{
	m_powerMin = m_settings->value("MotorMin").toFloat();
	m_powerMax = m_settings->value("MotorMax").toFloat();

	invoke_open();
}

CopterMotor::~CopterMotor()
{
	invoke_close();
}

void CopterMotor::delta(float _delta)
{
	m_delta = qMax(qMin(_delta, 100.0f), -100.0f);
}

void CopterMotor::setPower(unsigned _power)
{
	m_delta = qMax(m_delta, - static_cast<float>(_power));
	int pwr =  floor(_power + sqrt(m_delta + 0.5));
	pwr = qMin(pwr, static_cast<int>(_power) * 2);
	invoke(pwr);
	emit powerChanged(static_cast<float>(pwr));
}


void CopterMotor::emergencyStop()
{
	invoke(0);
}

