#include <QTimer>
#include <QTcpSocket>

#include <cmath>
#include "CopterCtrl.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CopterCtrl::CopterCtrl() :
	m_power(0),
	m_tcpServer(),
	m_tcpConnection()
{
	initSettings();
	initMotors(m_settings->value("ControlPath").toString());

	m_tcpServer.listen(QHostAddress::Any, m_settings->value("TcpPort").toInt());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));
}

void CopterCtrl::initMotors(const QString& motorControlPath)
{
	QString motorControlFile = m_settings->value("MotorControlFile").toString();
	int powerMax = m_settings->value("PowerMotorMax").toInt();
	int powerMin = m_settings->value("PowerMotorMin").toInt();
	int angleMax = m_settings->value("AngleMotorMax").toInt();
	int angleMin = m_settings->value("AngleMotorMin").toInt();
	m_powerMotor = new CopterMotor(powerMin, powerMax, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:1/" + motorControlFile);
	m_angleMotor = new CopterMotor(angleMin, angleMax, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:0/" + motorControlFile);
//	m_cameraMotor = new CopterMotor(m_settings, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:0/" + motorControlFile);
}

void CopterCtrl::initSettings()
{
	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

	// TODO: write proper checker
	if (m_settings->allKeys().count() == 0) {
		// TODO: move to conf file
		m_settings->setValue("ControlPath", "/sys/devices/platform/");
		m_settings->setValue("TcpPort", 4000);
		m_settings->setValue("PowerStep1", 5);
		m_settings->setValue("PowerStep2", 20);
		m_settings->setValue("PowerMin", -100);
		m_settings->setValue("PowerMax", 100); // null 1540000
		m_settings->setValue("PowerMotorMax", 1680000);
		m_settings->setValue("PowerMotorMin", 1540000);
		m_settings->setValue("AngleMotorMax", 1800000);
		m_settings->setValue("AngleMotorMin", 1500000);
		m_settings->setValue("MotorControlFile", "duty_ns");
		// servo 1500000 +- 300000
	}

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();

	connect(this, SIGNAL(settingsValueChanged(QString,QVariant)), this, SLOT(onSettingsValueChange(QString,QVariant)));
}

void CopterCtrl::adjustSettingsValue(const QString &key, bool increase)
{
	QVariant::Type type = m_settings->value(key).type();
	switch(type) {
		case QMetaType::Int:
			m_settings->setValue(key, m_settings->value(key).toInt() + (increase ? 1 : -1));
			break;
		case QMetaType::Float:
			m_settings->setValue(key, m_settings->value(key).toFloat() * (increase ? 0.9 : (1.0 / 0.9)));
			break;
		case QMetaType::Double:
			m_settings->setValue(key, m_settings->value(key).toDouble() * (increase ? 0.9 : (1.0 / 0.9)));
			break;
		case QMetaType::Bool:
			m_settings->setValue(key, increase);
			break;
		default:
			tcpLog("Inappropriate type for adjusting: " + QString(QMetaType::typeName(type)));
			return;
			break;
	}
	emit settingsValueChanged(key, m_settings->value(key));
}

void CopterCtrl::onSettingsValueChange(const QString &key, const QVariant &value)
{
	if (value.canConvert(QVariant::String)) {
		tcpLog("Settings value for key " + key + " changed. New value: " + value.toString());
	}
	else {
		tcpLog("Settings value for key " + key + " changed. Value is inconvertable to string");
	}
}


void CopterCtrl::adjustPower(int _incr)
{
	setPower(m_power + _incr);
}

void CopterCtrl::setPower(int _power)
{
	static const int s_power_min = m_settings->value("PowerMotorMin").toInt();
	static const int s_power_max = m_settings->value("PowerMotorMax").toInt();

	m_power = _power;
	m_power = qMax(qMin(m_power, s_power_max), s_power_min);
	tcpLog("Motor power changed: " + QString::number(m_power));

	m_powerMotor->invoke(m_power);
}

void CopterCtrl::adjustAngle(int angle)
{
	setAngle(m_angle + angle);
}

void CopterCtrl::setAngle(int angle)
{
	static const int s_angle_min = m_settings->value("AngleMotorMin").toInt();
	static const int s_angle_max = m_settings->value("AngleMotorMax").toInt();

	m_angle = angle;
	m_angle = qMax(qMin(m_angle, s_angle_max), s_angle_min);
	tcpLog("Angle power changed: " + QString::number(m_angle));

	m_angleMotor->invoke(m_angle);
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
	m_powerMotor->invoke(0);
	m_angleMotor->invoke(0);
	m_cameraMotor->invoke(0);
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
			case 'Z': setPower(0); setAngle(0); break;
			case 'z':
				if (m_power == 0) {
					setPower(-100);
				}
				else {
					adjustPower(-s_power_step2);
				}
				break;
			case 'x': adjustAngle(-s_power_step1); break;
			case 'c': adjustAngle(+s_power_step1); break;
			case 'v': adjustPower(+s_power_step2); break;
			case 'V': adjustPower(+s_power_max); break;
			case 'a': emergencyStop(); break;
		}
	}
}

