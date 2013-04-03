#include "CarMotor.hpp"
#include <QDebug>

CarMotor::CarMotor(int motorMin, int motorMax, const QString& _ctrlPath, const QString& name) :
	m_ctrlFile(_ctrlPath),
	m_powerMin(motorMin),
	m_powerMax(motorMax),
	m_power(0),
	m_name(name)
{
	invoke(0);
}

void CarMotor::invoke(int _power)
{
	QString s;
	if (m_ctrlFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered|QIODevice::Text)) {
		qreal powerFactor = (qreal) (m_powerMax - m_powerMin) / 100;
		s.sprintf("%d\n", static_cast<int>(_power * powerFactor + m_powerMin));
		m_ctrlFile.write(s.toLatin1());
		m_ctrlFile.close();
	}
	else {
		qDebug() << "Can't open motor control file " << m_ctrlFile.fileName();
	}
}

CarMotor::~CarMotor()
{
	invoke(0);
}

void CarMotor::emergencyStop()
{
	invoke(0);
}

