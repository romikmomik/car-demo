#ifndef CARCTRL_HPP
#define CARCTRL_HPP

#include <QTcpServer>
#include <QSocketNotifier>
#include <QSettings>
#include <QVector3D>

#include "CarMotor.hpp"
#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif

QT_FORWARD_DECLARE_CLASS(Sensor)

class CarCtrl : public QObject
{
	Q_OBJECT
public:
	CarCtrl();
	QSettings* getSettings() { return m_settings; }

public slots:
	void qrealResponce(const QByteArray& a);
	void emergencyStop();

protected slots:
	void onQRealConnection();
	void onQRealDisconnected();
	void onQRealNetworkRead();
	void initMotors();
	void initSettings();
	void initSensors();

signals:

protected:
	QMap<QString, CarMotor*> m_motors;
	QMap<QString, Sensor*> m_sensors;

	QSettings* m_settings;
	QSettings* m_defaultSettings;

	QTcpServer  m_qrealServer;
	QTcpSocket* m_qrealConnection;
};

#endif // CARCTRL_HPP
