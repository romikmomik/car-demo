#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#include <QObject>
#include <QFile>

class LightSensor : public QObject
{
	Q_OBJECT
public:
	explicit LightSensor(const QString& filePath, unsigned int lightMin, unsigned int lightMax, QObject *parent = 0);

	unsigned int getLight();

signals:
	
public slots:


private:
	unsigned int m_min, m_max;
	QFile* m_file;
};

#endif // LIGHTSENSOR_HPP
