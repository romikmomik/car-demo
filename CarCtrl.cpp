#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpSocket>

#include <cmath>
#include "CarCtrl.hpp"
#include "Sensor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CarCtrl::CarCtrl() :
	m_tcpServer(),
	m_tcpConnection()
{
	initSettings();
	initMotors();
	initSensors();

	m_tcpServer.listen(QHostAddress::Any, m_settings->value("TcpPort").toInt());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));
	m_qrealServer.listen(QHostAddress::Any, m_settings->value("QRealPort").toInt());
	connect(&m_qrealServer, SIGNAL(newConnection()), this, SLOT(onQRealConnection()));
}

void CarCtrl::initMotors()
{
	int powerLeftMax = m_settings->value("MotorLeftMax").toInt();
	int powerLeftMin = m_settings->value("MotorLeftMin").toInt();
	int powerRightMax = m_settings->value("MotorRightMax").toInt();
	int powerRightMin = m_settings->value("MotorRightMin").toInt();
	m_motorLeft = new CarMotor(powerLeftMin, powerLeftMax, m_settings->value("MotorLeftControlPath").toString());
	m_motorRight = new CarMotor(powerRightMin, powerRightMax, m_settings->value("MotorRightControlPath").toString());

	connect(m_motorLeft, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
	connect(m_motorRight, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
}

void CarCtrl::initSensors()
{
	QString pathLightLeft = m_settings->value("LightSensorLeftFilePath").toString();
	QString pathLightRight = m_settings->value("LightSensorRightFilePath").toString();
	QString pathSonar = m_settings->value("SonarSensorFilePath").toString();
	unsigned int lightMin = m_settings->value("LightMin").toUInt();
	unsigned int lightMax = m_settings->value("LightMax").toUInt();
	unsigned int lightNormalizedMax = m_settings->value("LightNormalizedMax").toUInt();
	unsigned int sonarNormalizedMax = m_settings->value("SonarNormalizedMax").toUInt();
	unsigned int sonarMiltiplicator = m_settings->value("SonarMultiplicator").toUInt();
	m_lightSensorLeft = new Sensor(pathLightLeft, lightMin, lightMax, lightNormalizedMax, this);
	m_lightSensorRight = new Sensor(pathLightRight, lightMin, lightMax, lightNormalizedMax, this);
	m_sonarSensor = new Sensor(pathSonar, 0, sonarNormalizedMax * sonarMiltiplicator, sonarNormalizedMax, this);
}

void CarCtrl::initSettings()
{
	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

	// TODO: write proper checker
	if (m_settings->allKeys().count() == 0) {
		// TODO: move to conf file
		m_settings->setValue("MotorLeftControlPath", "/sys/devices/platform/ehrpwm.1/pwm/ehrpwm.1:0/duty_ns");
		m_settings->setValue("MotorRightControlPath", "/sys/devices/platform/ehrpwm.0/pwm/ehrpwm.0:1/duty_ns");
		m_settings->setValue("TcpPort", 4000);
		m_settings->setValue("QRealPort", 4444);
		m_settings->setValue("PowerStep1", 5);
		m_settings->setValue("PowerStep2", 20);
		m_settings->setValue("PowerMin", -100);
		m_settings->setValue("PowerMax", 100); // null 1540000
		m_settings->setValue("MotorLeftMax", 1680000);
		m_settings->setValue("MotorLeftMin", 1540000);
		m_settings->setValue("MotorRightMax", 1800000);
		m_settings->setValue("MotorRightMin", 1500000);
		m_settings->setValue("LightMin", 0);
		m_settings->setValue("LightMax", 4000000);
		m_settings->setValue("LightNormalizedMax", 1000);
		m_settings->setValue("SonarMultiplicator", 58);
		m_settings->setValue("SonarNormalizedMax", 400);
		m_settings->setValue("LightSensorLeftFilePath", "/tmp/light_left");
		m_settings->setValue("LightSensorRightFilePath", "/tmp/light_right");
		m_settings->setValue("SonarSensorFilePath", "/tmp/sonar");
		// servo 1500000 +- 300000
	}

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();
}

void CarCtrl::tcpLog(const QString &message)
{
	if (!m_tcpConnection.isNull()) {
		m_tcpConnection->write(message.toAscii());
		m_tcpConnection->write("\n\r");
	}
}

void CarCtrl::emergencyStop()
{
	m_motorLeft->invoke(0);
	m_motorRight->invoke(0);
	QApplication::quit();
}

void CarCtrl::onConnection()
{
	if (!m_tcpConnection.isNull())
		qDebug() << "Replacing existing connection";
	m_tcpConnection = m_tcpServer.nextPendingConnection();
	qDebug() << "Accepted new connection";
	m_tcpConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(m_tcpConnection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_tcpConnection, SIGNAL(readyRead()), this, SLOT(onNetworkRead()));
}

void CarCtrl::onDisconnected()
{
	qDebug() << "Existing connection disconnected";
	m_tcpConnection = 0;
}

void CarCtrl::onNetworkRead()
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


void CarCtrl::onQRealConnection()
{
	if (!m_qrealConnection.isNull())
		qDebug() << "Replacing existing QReal connection";
	m_qrealConnection = m_qrealServer.nextPendingConnection();
	qDebug() << "Accepted new QReal connection";
	m_qrealConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
	connect(m_qrealConnection, SIGNAL(disconnected()), this, SLOT(onQRealDisconnected()));
	connect(m_qrealConnection, SIGNAL(readyRead()), this, SLOT(onQRealNetworkRead()));
}

void CarCtrl::onQRealDisconnected()
{
	qDebug() << "Existing QReal connection disconnected";
	m_qrealConnection = 0;
}

void CarCtrl::qrealResponce(const QByteArray& a)
{
	m_qrealConnection->write(a);
}

void CarCtrl::onQRealNetworkRead()
{
	if (m_qrealConnection.isNull())
		return;

	while (m_qrealConnection->bytesAvailable() > 0)
	{
		char data[100];
		m_qrealConnection->readLine(data, 100);
		QString command(data);
		QStringList cmd = command.split(" ", QString::SkipEmptyParts);
		if (cmd.at(0) == "motor") {
			if (cmd.at(1).trimmed() == "left") {
				m_motorLeft->invoke(cmd.at(2).toInt());
			}
			else {
				m_motorRight->invoke(cmd.at(2).toInt());
			}
		}
		else if (cmd.at(0) == "light_sensor") {
			unsigned int ans = 0;
			if (cmd.at(1).trimmed() == "left") {
				ans = m_lightSensorLeft->getValue();
			}
			else {
				ans = m_lightSensorRight->getValue();
			}
			char buf[2];
			buf[0] = (ans >> 8) & 0xff;
			buf[1] = ans & 0xff;
			qrealResponce(QByteArray(buf, 2));
		}
		else if (cmd.at(0).trimmed() == "distance_sensor" ||
				cmd.at(0).trimmed() == "sonar_sensor") {
			unsigned int ans = 0;
			ans = m_sonarSensor->getValue();
			char buf[2];
			buf[0] = (ans >> 8) & 0xff;
			buf[1] = ans & 0xff;
			qrealResponce(QByteArray(buf, 2));
		}
		else {
			qDebug() << "Unknown command: " + cmd.at(0) << endl;
		}
		qDebug() << "QReal request " << command << endl;
	}
}
