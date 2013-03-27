#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#include <QObject>
#include <QFile>

class Sensor : public QObject
{
	Q_OBJECT
public:
	Sensor(const QString& filePath, uint lightMin, uint lightMax
			, uint normalizedMax, QObject *parent = 0);

	unsigned int getValue();

signals:
	
public slots:


private:
	uint m_min, m_max;
	uint m_normalizedMax;
	QFile* m_file;
};

#endif // LIGHTSENSOR_HPP
