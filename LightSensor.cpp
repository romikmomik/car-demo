#include "LightSensor.hpp"

LightSensor::LightSensor(const QString &filePath, unsigned int lightMin, unsigned int lightMax, QObject *parent) :
	m_min(lightMin), m_max(lightMax), QObject(parent)
{
	m_file = new QFile(filePath);
}

unsigned int LightSensor::getLight()
{
	m_file->open(QIODevice::ReadOnly);
	char data[128];
	m_file->readLine(data, 128);
	QString s(data);
	m_file->close();
	if (m_max == m_min) return 0;
	unsigned int res = (((s.toUInt() - m_min) * 1000) / (m_max - m_min));
	return res;
}
