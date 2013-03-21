#include <QTimer>
#include <QTcpSocket>

#include <cmath>
#include "CopterCtrl.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CopterCtrl::CopterCtrl() :
	m_tcpServer(),
	m_tcpConnection()
{
	initSettings();
	initMotors(m_settings->value("ControlPath").toString());

	// network setup
	m_tcpServer.listen(QHostAddress::Any, m_settings->value("TcpPort").toInt());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));
	m_androidServer.listen(QHostAddress::Any, m_settings->value("AndroidPort").toInt());
	connect(&m_androidServer, SIGNAL(newConnection()), this, SLOT(onAndroidConnection()));
}

void CopterCtrl::initMotors(const QString& motorControlPath)
{
	QString motorControlFile = m_settings->value("MotorControlFile").toString();
	int powerMax = m_settings->value("PowerMotorMax").toInt();
	int powerMin = m_settings->value("PowerMotorMin").toInt();
	int handMax = m_settings->value("HandMotorMax").toInt();
	int handMin = m_settings->value("HandMotorMin").toInt();
	m_rightMotor = new CopterMotor(powerMin, powerMax, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:1/" + motorControlFile);
	m_leftMotor = new CopterMotor(powerMin, powerMax, motorControlPath + "ehrpwm.1/pwm/ehrpwm.0:1/" + motorControlFile);
	m_handMotor = new CopterMotor(handMin, handMax, motorControlPath + "ehrpwm.1/pwm/ehrpwm.0:0/" + motorControlFile);

	connect(m_rightMotor, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
	connect(m_leftMotor, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
	connect(m_handMotor, SIGNAL(toLog(QString)), this, SLOT(tcpLog(QString)));
}

void CopterCtrl::initSettings()
{
	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

	// TODO: write proper checker
	if (m_settings->allKeys().count() == 0) {
		// TODO: move to conf file
		m_settings->setValue("ControlPath", "/sys/devices/platform/");
		m_settings->setValue("TcpPort", 4000);
		m_settings->setValue("AndroidPort", 4444);
		m_settings->setValue("PowerStep1", 100);
		m_settings->setValue("PowerStep2", 100);
		m_settings->setValue("PowerMin", -100);
		m_settings->setValue("PowerMax", 100); // null 1540000
		m_settings->setValue("PowerMotorMax", 2000000);
		m_settings->setValue("PowerMotorMin", 1500000);
		m_settings->setValue("HandMotorMax", 1500000);
		m_settings->setValue("HandMotorMin", 950000);
		m_settings->setValue("MotorControlFile", "duty_ns");
		// servo 1500000 +- 300000
	}

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();

	connect(this, SIGNAL(settingsValueChanged(QString,QVariant)), this, SLOT(onSettingsValueChange(QString,QVariant)));
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
	m_leftMotor->invoke(0);
	m_rightMotor->invoke(0);
	m_handMotor->invoke(0);
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

	static const int s_power_max = m_settings->value("PowerMax").toInt();
	//static const int s_power_min = m_settings->value("PowerMin").toInt();
	static const int s_power_step1 = m_settings->value("PowerStep1").toInt();
	static const int s_power_step2 = m_settings->value("PowerStep2").toInt();

	while (m_tcpConnection->isReadable())
	{
		char c;
		if (!m_tcpConnection->getChar(&c))
			break;
		switch (c)
		{
			case 'Z': m_leftMotor->setPower(0); m_rightMotor->setPower(0); m_handMotor->setPower(0); break;
			case 'z': m_leftMotor->adjustPower(-s_power_step2); m_rightMotor->adjustPower(-s_power_step2); break;
			case 'x': m_handMotor->adjustPower(-s_power_step1); break;
			case 'c': m_handMotor->adjustPower(+s_power_step1); break;
//			case 'v': m_powerMotor->adjustPower(+s_power_step2); break;
			case 'V': m_leftMotor->adjustPower(+s_power_max); m_rightMotor->adjustPower(+s_power_max); break;
			case 'a': emergencyStop(); break;
		}
	}
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

void CopterCtrl::onAndroidNetworkRead()
{
	if (m_androidConnection.isNull())
		return;

//	while (m_androidConnection->bytesAvailable() > 0)
//	{
//		char data[100];
//		m_androidConnection->readLine(data, 100);
//		QString command(data);// = QString(m_androidConnection->readAll());
//		QStringList cmd = command.split(" ", QString::SkipEmptyParts);
//		if (cmd.at(0) == "power") {
//			m_powerMotor->setPower(cmd.at(1).toInt());
//		}
//		else if (cmd.at(0) == "angle") {
//			m_angleMotor->setPower(cmd.at(1).toInt());
//		}
//		else {
//			qDebug() << "Unknown command: " + cmd.at(0) << endl;
//		}
//	}
}
