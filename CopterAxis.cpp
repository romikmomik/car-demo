#include "CopterAxis.hpp"

CopterAxis::CopterAxis(CopterMotor *_motor1, CopterMotor *_motor2) :
	m_motor1(_motor1),
	m_motor2(_motor2)
{
}

double CopterAxis::tilt() const
{
	return m_motor1->factor() - m_motor2->factor();
}

void CopterAxis::tilt(double _tilt) const
{
	m_motor1->factor(1.0 - _tilt / 2);
	m_motor2->factor(1.0 + _tilt / 2);
}
