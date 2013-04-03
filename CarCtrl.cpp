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
	m_qrealServer(),
	m_motors(),
	m_sensors()
{
	initSettings();
	initMotors();
	initSensors();

	m_qrealConnection = new QTcpSocket();
	m_qrealServer.listen(QHostAddress::Any, m_settings->value("QRealPort").toInt());
	connect(&m_qrealServer, SIGNAL(newConnection()), this, SLOT(onQRealConnection()));
}

void CarCtrl::initSettings()
{
	m_defaultSettings = new QSettings(this);

	// default settings init
	m_defaultSettings->beginGroup("MotorKeys");
	m_defaultSettings->setValue("MotorLeft", 0);
	m_defaultSettings->setValue("MotorRight", 1);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("SensorKeys");
	m_defaultSettings->setValue("SensorLightLeft", 0);
	m_defaultSettings->setValue("SensorLightRight", 1);
	m_defaultSettings->setValue("SensorSonar", 2);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("MotorLeft");
	m_defaultSettings->setValue("ControlPath", "/sys/devices/platform/ehrpwm.1/pwm/ehrpwm.1:0/duty_ns");
	m_defaultSettings->setValue("PercentMin", -100);
	m_defaultSettings->setValue("PercentMax", 100);
	m_defaultSettings->setValue("PowerMin", 1500000);
	m_defaultSettings->setValue("PowerMax", 1800000);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("MotorRight");
	m_defaultSettings->setValue("ControlPath", "/sys/devices/platform/ehrpwm.0/pwm/ehrpwm.0:1/duty_ns");
	m_defaultSettings->setValue("PercentMin", -100);
	m_defaultSettings->setValue("PercentMax", 100);
	m_defaultSettings->setValue("PowerMin", 1500000);
	m_defaultSettings->setValue("PowerMax", 1800000);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("SensorLightLeft");
	m_defaultSettings->setValue("FilePath", "/tmp/light_left");
	m_defaultSettings->setValue("ValueMin", 0);
	m_defaultSettings->setValue("ValueMax", 4000000);
	m_defaultSettings->setValue("NormalizedMax", 1000);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("SensorLightRight");
	m_defaultSettings->setValue("FilePath", "/tmp/light_right");
	m_defaultSettings->setValue("ValueMin", 0);
	m_defaultSettings->setValue("ValueMax", 4000000);
	m_defaultSettings->setValue("NormalizedMax", 1000);
	m_defaultSettings->endGroup();

	m_defaultSettings->beginGroup("SensorSonar");
	m_defaultSettings->setValue("FilePath", "/tmp/sonar");
	m_defaultSettings->setValue("ValueMin", 0);
	m_defaultSettings->setValue("ValueMax", 400 * 58);
	m_defaultSettings->setValue("NormalizedMax", 400);
	m_defaultSettings->endGroup();

	m_defaultSettings->setValue("QRealPort", 4444);

	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	// TODO: write proper checker
	if (m_settings->allKeys().count() == 0) {
		// use default
		m_settings = m_defaultSettings;
	}

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();
}

void CarCtrl::initMotors()
{
	m_settings->beginGroup("MotorKeys");
	QStringList motorKeys = m_settings->allKeys();
	m_settings->endGroup();
	for (int i = 0; i < motorKeys.size(); ++i) {
		QString name = motorKeys[i];
		m_settings->beginGroup(name);
		QString controlPath = m_settings->value("ControlPath").toString();
		int powerMin = m_settings->value("PowerMin").toInt();
		int powerMax = m_settings->value("PowerMax").toInt();
		CarMotor* motor = new CarMotor(powerMin, powerMax, controlPath, name);
		m_motors[name] = motor;
		m_settings->endGroup();
	}
}

void CarCtrl::initSensors()
{
	m_settings->beginGroup("SensorKeys");
	QStringList sensorKeys = m_settings->allKeys();
	m_settings->endGroup();
	for (int i = 0; i < sensorKeys.size(); ++i) {
		QString name = sensorKeys[i];
		m_settings->beginGroup(name);
		QString filePath = m_settings->value("FilePath").toString();
		int valueMin = m_settings->value("ValueMin").toInt();
		int valueMax = m_settings->value("ValueMax").toInt();
		int normalizedMax = m_settings->value("NormalizedMax").toInt();
		Sensor* sensor = new Sensor(filePath, valueMin, valueMax, normalizedMax, this);
		m_sensors[name] = sensor;
		m_settings->endGroup();
	}
}

void CarCtrl::emergencyStop()
{
	CarMotor* motor;
	foreach (motor, m_motors) {
		motor->invoke(0);
	}

	QApplication::quit();
}

void CarCtrl::playSound(const QString &fileName)
{
	QString command = "aplay --quiet " + fileName;
	system(command.toStdString().c_str());
}

void CarCtrl::onQRealConnection()
{
	if (m_qrealConnection->isValid())
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
	m_qrealConnection->disconnectFromHost();
}

void CarCtrl::qrealResponce(const QByteArray& a)
{
	m_qrealConnection->write(a);
}

void CarCtrl::onQRealNetworkRead()
{
	if (!m_qrealConnection->isValid())
		return;

	while (m_qrealConnection->bytesAvailable() > 0)
	{
		char data[100];
		m_qrealConnection->readLine(data, 100);
		QString command(data);
		QStringList cmd = command.split(" ", QString::SkipEmptyParts);

		QString commandName = cmd.at(0).trimmed();
		if (m_motors.contains(commandName)) {
			m_motors[commandName]->invoke(cmd.at(1).toInt());
		}
		else if (m_sensors.contains(commandName)) {
			qrealResponce(m_sensors[commandName]->getByteValue());
		}
		else if (commandName == "sound") {
			playSound("/home/root/alarm.wav");
		}
		else {
			qDebug() << "Unknown command: " + cmd.at(0);
		}
		qDebug() << "QReal request " << command;
	}
}
