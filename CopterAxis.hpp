#pragma once
#include "CopterMotor.hpp"

class CopterAxis : public QObject
{
	Q_OBJECT
public:
	CopterAxis(CopterMotor* _motor1, CopterMotor* _motor2);

	double tilt() const; // -1.0 .. +1.0
	void tilt(double _tilt) const;

	void setPower(unsigned _power) { m_motor1->setPower(_power); m_motor2->setPower(_power); }

protected:
	CopterMotor* m_motor1;
	CopterMotor* m_motor2;
};
