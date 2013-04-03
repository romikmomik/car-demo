#include "Sensor.hpp"
#include <QDebug>

Sensor::Sensor(const QString &filePath, uint lightMin, uint lightMax
		, uint normalizedMax, QObject *parent) :
	m_min(lightMin), m_max(lightMax), m_normalizedMax(normalizedMax), QObject(parent)
{
	m_file = new QFile(filePath);
}

unsigned int Sensor::getValue()
{
	if (m_file->open(QIODevice::ReadOnly)) {
		char data[128];
		m_file->readLine(data, 128);
		QString s(data);
		s = s.trimmed();
		m_file->close();
		if (m_max == m_min) return m_min;
		long long value = s.toLongLong();
		value = qMin(value, static_cast<long long>(m_max));
		value = qMax(value, static_cast<long long>(m_min));
		unsigned int res = (((value - m_min) * m_normalizedMax) / (m_max - m_min));
		return res;
	}
	qDebug() << "Can't open sensor file " << m_file->fileName();
}

QByteArray Sensor::getByteValue()
{
	unsigned int value = getValue();
	char buf[2];
	buf[0] = (value >> 8) & 0xff;
	buf[1] = value & 0xff;
	return QByteArray(buf, 2);
}
