#include "CopterMotor.hpp"

void CopterMotor::invoke(int _power)
{
	QString s;
	m_ctrlFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered|QIODevice::Text);
	float powerFactor = (float)(m_powerMax - m_powerMin) / 100;
	s.sprintf("%d\n", static_cast<int>(_power * powerFactor + m_powerMin));
	m_ctrlFile.write(s.toLatin1());
	m_ctrlFile.close();
}

CopterMotor::CopterMotor(int motorMin, int motorMax, const QString& _ctrlPath) :
	m_ctrlFile(_ctrlPath),
	m_powerMin(motorMin),
	m_powerMax(motorMax)
{
	invoke(0);
}

CopterMotor::~CopterMotor()
{
	invoke(0);
}

void CopterMotor::emergencyStop()
{
	invoke(0);
}

