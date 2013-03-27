#ifndef COPTERCTRL_HPP
#define COPTERCTRL_HPP

#include <QTcpServer>
#include <QPointer>
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
	void tcpLog(const QString& message);
	void qrealResponce(const QByteArray& a);
	void emergencyStop();

protected slots:
	void onConnection();
	void onDisconnected();
	void onNetworkRead();
	void onQRealConnection();
	void onQRealDisconnected();
	void onQRealNetworkRead();
	void initMotors();
	void initSettings();
	void initSensors();

signals:

protected:
	CarMotor* m_motorLeft;
	CarMotor* m_motorRight;
	Sensor* m_lightSensorLeft;
	Sensor* m_lightSensorRight;
	Sensor* m_sonarSensor;

	QSettings* m_settings;

	QTcpServer           m_tcpServer;
	QPointer<QTcpSocket> m_tcpConnection;
	QTcpServer           m_qrealServer;
	QPointer<QTcpSocket> m_qrealConnection;
};

#endif // COPTERCTRL_HPP
