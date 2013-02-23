#pragma once
#include "CopterMotor.hpp"

class CopterAxis : public QObject
{
	Q_OBJECT
public:
	CopterAxis(const QSharedPointer<CopterMotor>& _motor1,
						 const QSharedPointer<CopterMotor>& _motor2);

	double tilt() const; // -1.0 .. +1.0
	void tilt(double _tilt) const;

	void setPower(unsigned _power) { m_motor1->setPower(_power); m_motor2->setPower(_power); }

protected:
	QSharedPointer<CopterMotor> m_motor1;
	QSharedPointer<CopterMotor> m_motor2;
};
