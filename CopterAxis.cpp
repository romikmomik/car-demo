#include "CopterAxis.hpp"

CopterAxis::CopterAxis(CopterMotor *_motor1, CopterMotor *_motor2) :
	m_motor1(_motor1),
	m_motor2(_motor2)
{
}

double CopterAxis::tilt() const
{
	return m_motor1->delta() - m_motor2->delta();
}

void CopterAxis::tilt(double _tilt) const
{
	m_motor1->delta(- _tilt / 2);
	m_motor2->delta(  _tilt / 2);
}

void CopterAxis::emergencyStop()
{
	m_motor1->emergencyStop();
	m_motor2->emergencyStop();
}
