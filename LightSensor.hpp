#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#include <QObject>
#include <QFile>

class LightSensor : public QObject
{
	Q_OBJECT
public:
	explicit LightSensor(const QString& filePath, QObject *parent = 0);

	unsigned int getLight();

signals:
	
public slots:


private:
	QFile* m_file;
};

#endif // LIGHTSENSOR_HPP
