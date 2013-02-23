#include "CopterAxis.hpp"

CopterAxis::CopterAxis(const QSharedPointer<CopterMotor>& _motor1,
											 const QSharedPointer<CopterMotor>& _motor2)
	:m_motor1(_motor1),
		m_motor2(_motor2)
{
}

double CopterAxis::tilt() const
{
	return m_motor1->factor() - m_motor2->factor();
}

void CopterAxis::tilt(double _tilt) const
{
	if (_tilt == 0)
	{
		m_motor1->factor(1.0);
		m_motor2->factor(1.0);
	}
	else if (_tilt < 0)
	{
		m_motor1->factor(1.0+_tilt);
		m_motor2->factor(1.0);
	}
	else if (_tilt > 0)
	{
		m_motor1->factor(1.0);
		m_motor2->factor(1.0-_tilt);
	}
}

