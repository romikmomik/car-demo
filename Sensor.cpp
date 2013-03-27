#include "Sensor.hpp"

Sensor::Sensor(const QString &filePath, uint lightMin, uint lightMax
		, uint normalizedMax, QObject *parent) :
	m_min(lightMin), m_max(lightMax), m_normalizedMax(normalizedMax), QObject(parent)
{
	m_file = new QFile(filePath);
	if (m_min < m_max) {
		qSwap(m_min, m_max);
	}
}

unsigned int Sensor::getValue()
{
	m_file->open(QIODevice::ReadOnly);
	char data[128];
	m_file->readLine(data, 128);
	QString s(data);
	m_file->close();
	if (m_max == m_min) return 0;
	unsigned int res = (((s.toUInt() - m_min) * m_normalizedMax) / (m_max - m_min));
	return res;
}
