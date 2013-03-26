#include <QTimer>
#include <QTcpSocket>

#include <cmath>
#include "CopterCtrl.hpp"
#include "LightSensor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CopterCtrl::CopterCtrl() :
	m_tcpServer(),
	m_tcpConnection()
{
	initSettings();
	initMotors();
	initSensors();

	m_tcpServer.listen(QHostAddress::Any, m_settings->value("TcpPort").toInt());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));
	m_androidServer.listen(QHostAddress::Any, m_settings->value("AndroidPort").toInt());
	connect(&m_androidServer, SIGNAL(newConnection()), this, SLOT(onAndroidConnection()));
}

void CopterCtrl::initMotors()
{
	int powerLeftMax = m_settings->value("MotorLeftMax").toInt();
	int powerLeftMin = m_settings->value("MotorLeftMin").toInt();
	int powerRightMax = m_settings->value("MotorRightMax").toInt();
	int powerRightMin = m_settings->value("MotorRightMin").toInt();
	m_motorLeft = new CopterMotor(powerLeftMin, powerLeftMax, m_settings->value("MotorLeftControlPath").toString());
	m_motorRight = new CopterMotor(powerRightMin, powerRightMax, m_settings->value("MotorRightControlPath").toString());

	connect(m_motorLeft, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
	connect(m_motorRight, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
}

void CopterCtrl::initSensors()
{
	QString pathLeft = m_settings->value("LightSensorLeftFilePath").toString();
	QString pathRight = m_settings->value("LightSensorRightFilePath").toString();
	m_lightSensorLeft = new LightSensor(pathLeft, this);
	m_lightSensorRight = new LightSensor(pathRight, this);
}

void CopterCtrl::initSettings()
{
	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

	// TODO: write proper checker
	if (m_settings->allKeys().count() == 0) {
		// TODO: move to conf file
		m_settings->setValue("MotorLeftControlPath", "/sys/devices/platform/ehrpwm.1/pwm/ehrpwm.1:0/duty_ns");
		m_settings->setValue("MotorRightControlPath", "/sys/devices/platform/ehrpwm.0/pwm/ehrpwm.0:1/duty_ns");
		m_settings->setValue("TcpPort", 4000);
		m_settings->setValue("AndroidPort", 4444);
		m_settings->setValue("PowerStep1", 5);
		m_settings->setValue("PowerStep2", 20);
		m_settings->setValue("PowerMin", -100);
		m_settings->setValue("PowerMax", 100); // null 1540000
		m_settings->setValue("MotorLeftMax", 1680000);
		m_settings->setValue("MotorLeftMin", 1540000);
		m_settings->setValue("MotorRightMax", 1800000);
		m_settings->setValue("MotorRightMin", 1500000);
		m_settings->setValue("LightSensorLeftFilePath", "/tmp/light_left");
		m_settings->setValue("LightSensorRightFilePath", "/tmp/light_right");
		m_settings->setValue("SonicSensorFilePath", "/tmp/sonic");
		// servo 1500000 +- 300000
	}

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();
}

void CopterCtrl::tcpLog(const QString &message)
{
	if (!m_tcpConnection.isNull()) {
		m_tcpConnection->write(message.toAscii());
		m_tcpConnection->write("\n\r");
	}
}

void CopterCtrl::emergencyStop()
{
	m_motorLeft->invoke(0);
	m_motorRight->invoke(0);
	QApplication::quit();
}

void CopterCtrl::onConnection()
{
	if (!m_tcpConnection.isNull())
		qDebug() << "Replacing existing connection";
	m_tcpConnection = m_tcpServer.nextPendingConnection();
	qDebug() << "Accepted new connection";
	m_tcpConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(m_tcpConnection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_tcpConnection, SIGNAL(readyRead()), this, SLOT(onNetworkRead()));
}

void CopterCtrl::onDisconnected()
{
	qDebug() << "Existing connection disconnected";
	m_tcpConnection = 0;
}

void CopterCtrl::onNetworkRead()
{
	if (m_tcpConnection.isNull())
		return;

//	static const int s_power_max = m_settings->value("PowerMax").toInt();
//	//static const int s_power_min = m_settings->value("PowerMin").toInt();
//	static const int s_power_step1 = m_settings->value("PowerStep1").toInt();
//	static const int s_power_step2 = m_settings->value("PowerStep2").toInt();

//	while (m_tcpConnection->isReadable())
//	{
//		char c;
//		if (!m_tcpConnection->getChar(&c))
//			break;
//		switch (c)
//		{
//			case 'Z': m_powerMotor->setPower(0); m_angleMotor->setPower(0); break;
//			case 'z': m_powerMotor->adjustPower(-s_power_step2);
//			case 'x': m_angleMotor->adjustPower(-s_power_step1); break;
//			case 'c': m_angleMotor->adjustPower(+s_power_step1); break;
//			case 'v': m_powerMotor->adjustPower(+s_power_step2); break;
//			case 'V': m_powerMotor->adjustPower(+s_power_max); break;
//			case 'a': emergencyStop(); break;
//		}
//	}
}


void CopterCtrl::onAndroidConnection()
{
	if (!m_androidConnection.isNull())
		qDebug() << "Replacing existing android connection";
	m_androidConnection = m_androidServer.nextPendingConnection();
	qDebug() << "Accepted new android connection";
	m_androidConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(m_androidConnection, SIGNAL(disconnected()), this, SLOT(onAndroidDisconnected()));
	connect(m_androidConnection, SIGNAL(readyRead()), this, SLOT(onAndroidNetworkRead()));
}

void CopterCtrl::onAndroidDisconnected()
{
	qDebug() << "Existing android connection disconnected";
	m_androidConnection = 0;
}

void CopterCtrl::androidLog(const QByteArray& a)
{
	m_androidConnection->write(a);
}

void CopterCtrl::onAndroidNetworkRead()
{
	if (m_androidConnection.isNull())
		return;

	while (m_androidConnection->bytesAvailable() > 0)
	{
		char data[100];
		m_androidConnection->readLine(data, 100);
		QString command(data);// = QString(m_androidConnection->readAll());
		QStringList cmd = command.split(" ", QString::SkipEmptyParts);
		if (cmd.at(0) == "motor") {
			if (cmd.at(1).trimmed() == "left") {
//			m_motorLeft->setPower(cmd.at(2).toInt());
			}
			else {
//			m_motorRight->setPower(cmd.at(2).toInt());
			}
		}
		else if (cmd.at(0) == "light_sensor") {
			unsigned int ans = 0;
			if (cmd.at(1).trimmed() == "left") {
				ans = m_lightSensorLeft->getLight();
			}
			else {
				ans = m_lightSensorRight->getLight();
			}
			char buf[2];
			buf[0] = (ans >> 8) & 0xff;
			buf[1] = ans & 0xff;
			androidLog(QByteArray(buf, 2));
		}
		else {
			qDebug() << "Unknown command: " + cmd.at(0) << endl;
		}
		qDebug() << "Android request " << command << endl;
	}
}
