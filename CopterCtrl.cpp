#include <QTimer>
#include <QTcpSocket>

#include <cmath>
#include "CopterCtrl.hpp"
#include "accelerometer.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CopterCtrl::CopterCtrl() :
	m_power(0),
	m_state(IDLE),
	m_tcpServer(),
	m_tcpConnection()
{
	initSettings();
	initMotors(m_settings->value("ControlPath").toString());

	m_tcpServer.listen(QHostAddress::Any, m_settings->value("TcpPort").toInt());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));

	// buttons reading
	const QString s_buttons_input_path = m_settings->value("ButtonsInputPath").toString();
	m_buttonsInputFd = ::open(s_buttons_input_path.toLatin1().data(), O_SYNC, O_RDONLY);
	if (m_buttonsInputFd == -1)
		qDebug() << "Cannot open buttons input file " << s_buttons_input_path << ", reason: " << errno;

	m_buttonsInputNotifier = new QSocketNotifier(m_buttonsInputFd, QSocketNotifier::Read, this);
	connect(m_buttonsInputNotifier, SIGNAL(activated(int)), this, SLOT(onButtonRead()));
	m_buttonsInputNotifier->setEnabled(true);

	m_accel = new Accelerometer(m_settings->value("AccelInputPath").toString(), this);
	connect(m_accel, SIGNAL(accelerometerRead(Axis)),
					this, SIGNAL(accelerometerRead(Axis)));
	connect(m_accel, SIGNAL(accelerometerRead(Axis)), this, SLOT(handleTilt(Axis)));
	connect(m_accel, SIGNAL(zeroAxisChanged(Axis)), this, SIGNAL(zeroAxisChanged(Axis)));
}

void CopterCtrl::initMotors(const QString& motorControlPath)
{
	CopterMotor* mx1 = new CopterMotor(m_settings, motorControlPath + "ehrpwm.0/pwm/ehrpwm.0:0/duty_percent");
	CopterMotor* mx2 = new CopterMotor(m_settings, motorControlPath + "ehrpwm.0/pwm/ehrpwm.0:1/duty_percent");
	CopterMotor* my1 = new CopterMotor(m_settings, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:0/duty_percent");
	CopterMotor* my2 = new CopterMotor(m_settings, motorControlPath + "ehrpwm.1/pwm/ehrpwm.1:1/duty_percent");
	m_motorIds.insert(mx1, MotorX1);
	m_motorIds.insert(mx2, MotorX2);
	m_motorIds.insert(my1, MotorY1);
	m_motorIds.insert(my2, MotorY2);
	connect(mx1, SIGNAL(powerChanged(double)), this, SLOT(onMotorPowerChange(double)));
	connect(mx2, SIGNAL(powerChanged(double)), this, SLOT(onMotorPowerChange(double)));
	connect(my1, SIGNAL(powerChanged(double)), this, SLOT(onMotorPowerChange(double)));
	connect(my2, SIGNAL(powerChanged(double)), this, SLOT(onMotorPowerChange(double)));

	m_axisX = new CopterAxis(mx1, mx2);
	m_axisY = new CopterAxis(my1, my2);
}

void CopterCtrl::initSettings()
{
	m_settings = new QSettings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

	// TODO: move to conf file
	m_settings->setValue("ControlPath", QVariant("/sys/devices/platform/"));
	m_settings->setValue("AccelInputPath", QVariant("/dev/input/event1"));
	m_settings->setValue("ButtonsInputPath", QVariant("/dev/input/event0"));
	m_settings->setValue("AccelAdjustingTime", QVariant(10000));
	m_settings->setValue("TcpPort", QVariant(4000));
	m_settings->setValue("TiltStep", QVariant(0.02d));
	m_settings->setValue("PowerStep1", QVariant(1));
	m_settings->setValue("PowerStep2", QVariant(5));
	m_settings->setValue("PowerMin", QVariant(0));
	m_settings->setValue("PowerMax", QVariant(100));
	m_settings->setValue("MotorMax", QVariant(72));
	m_settings->setValue("MotorMin", QVariant(48));
	m_settings->setValue("KalmanK", QVariant(0.95));
	m_settings->setValue("AccelLinear", QVariant(-0.02d));
	m_settings->setValue("AccelDerivative", QVariant(-0.005d));

	m_settings->setFallbacksEnabled(false);
	m_settings->sync();
}

void CopterCtrl::onMotorPowerChange(double power)
{
	emit motorPowerChanged(m_motorIds[dynamic_cast<CopterMotor*>(sender())], power);
}

void CopterCtrl::adjustTilt(Axis tilt) const
{
	m_axisX->tilt(m_axisX->tilt() + tilt.x);
	m_axisY->tilt(m_axisY->tilt() + tilt.y);
	m_axisX->setPower(m_power);
	m_axisY->setPower(m_power);
}

void CopterCtrl::adjustPower(int _incr)
{
	static const int s_power_min = m_settings->value("PowerMin").toInt();
	static const int s_power_max = m_settings->value("PowerMax").toInt();

	m_power += _incr;
	m_power = qMax(qMin(m_power, s_power_max), s_power_min);
	emit motorPowerChanged(MotorAll, m_power);

	m_axisX->setPower(m_power);
	m_axisY->setPower(m_power);
}

void CopterCtrl::setupAccelZeroAxis()
{
	if (m_state != CopterCtrl::IDLE)
		return;
	tcpLog("Start zero axis setup");
	setState(CopterCtrl::ADJUSTING_ACCEL);

	QTimer::singleShot(m_settings->value("AccelAdjustingTime").toInt(), this, SLOT(setState()));
}

void CopterCtrl::onAccelerometerRead(Axis val)
{
}

void CopterCtrl::handleTilt(Axis tilt)
{
	double accelLinear = m_settings->value("AccelLinear").toDouble();
	double accelDerivative = m_settings->value("AccelDerivative").toDouble();
	Axis adj = tilt * accelLinear + (tilt - m_lastTilt) * accelDerivative;
	adjustTilt(adj);
	m_lastTilt = tilt;
}

void CopterCtrl::tcpLog(const QString &message)
{
	if (!m_tcpConnection.isNull()) {
		m_tcpConnection->write(message.toAscii());
		m_tcpConnection->write("\n");
	}
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

	static const double s_tilt_step = m_settings->value("TiltStep").toDouble();
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
			case '1': adjustTilt(-s_tilt_step, -s_tilt_step); break;
			case '2': adjustTilt(0,            -s_tilt_step); break;
			case '3': adjustTilt(+s_tilt_step, -s_tilt_step); break;
			case '4': adjustTilt(-s_tilt_step, 0); break;
			case '5': tiltX(0); tiltY(0); break;
			case '6': adjustTilt(+s_tilt_step, 0); break;
			case '7': adjustTilt(-s_tilt_step, +s_tilt_step); break;
			case '8': adjustTilt(0,            +s_tilt_step); break;
			case '9': adjustTilt(+s_tilt_step, +s_tilt_step); break;
			case 'Z': adjustPower(-s_power_max); break;
			case 'z': adjustPower(-s_power_step2); break;
			case 'x': adjustPower(-s_power_step1); break;
			case 'c': adjustPower(+s_power_step1); break;
			case 'v': adjustPower(+s_power_step2); break;
			case 'V': adjustPower(+s_power_max); break;
			case '0': setupAccelZeroAxis(); break;
			case '[':
				m_settings->setValue("MotorMax", QVariant(m_settings->value("MotorMax").toInt() - 1));
				tcpLog("MotorMax changed: " + QString::number(m_settings->value("MotorMax").toInt()));
				break;
			case ']':
				m_settings->setValue("MotorMax", QVariant(m_settings->value("MotorMax").toInt() + 1));
				tcpLog("MotorMax changed: " + QString::number(m_settings->value("MotorMax").toInt()));
				break;
			case '{':
				m_settings->setValue("MotorMin", QVariant(m_settings->value("MotorMin").toInt() - 1));
				tcpLog("MotorMin changed: " + QString::number(m_settings->value("MotorMin").toInt()));
				break;
			case '}':
				m_settings->setValue("MotorMin", QVariant(m_settings->value("MotorMin").toInt() + 1));
				tcpLog("MotorMin changed: " + QString::number(m_settings->value("MotorMin").toInt()));
				break;
			case ',':
				m_settings->setValue("AccelLinear", QVariant(m_settings->value("AccelLinear").toDouble() * 0.9));
				tcpLog("AccelLinear changed: " + QString::number(m_settings->value("AccelLinear").toDouble()));
				break;
			case '.':
				m_settings->setValue("AccelLinear", QVariant(m_settings->value("AccelLinear").toDouble() / 0.9));
				tcpLog("AccelLinear changed: " + QString::number(m_settings->value("AccelLinear").toDouble()));
				break;
			case '<':
				m_settings->setValue("AccelDerivative", QVariant(m_settings->value("AccelDerivative").toDouble() * 0.9));
				tcpLog("AccelDerivative changed: " + QString::number(m_settings->value("AccelDerivative").toDouble()));
				break;
			case '>':
				m_settings->setValue("AccelDerivative", QVariant(m_settings->value("AccelDerivative").toDouble() / 0.9));
				tcpLog("AccelDerivative changed: " + QString::number(m_settings->value("AccelDerivative").toDouble()));
				break;
		}
	}
}


void CopterCtrl::onButtonRead()
{
	struct input_event evt;

	if (read(m_buttonsInputFd, reinterpret_cast<char*>(&evt), sizeof(evt)) != sizeof(evt))
	{
		qDebug() << "Incomplete buttons data read";
		return;
	}

	if (evt.type != EV_KEY)
	{
		if (evt.type != EV_SYN)
			qDebug() << "Input event type is not EV_KEY or EV_SYN: " << evt.type;
		return;
	}

	BoardButton button;

	switch (evt.code) {
		case KEY_F1: button = Button1; break;
		case KEY_F2: button = Button2; break;
		case KEY_F3: button = Button3; break;
		case KEY_F4: button = Button4; break;
		case KEY_F5: button = Button5; break;
		case KEY_F6: button = Button6; break;
		case KEY_F7: button = Button7; break;
		case KEY_F8: button = Button8; break;
	}

	tcpLog("Button pressed: " + QString::number(button));

	if (static_cast<bool>(evt.value)) {
		emit buttonPressed(button);
	}
	else {
		emit buttonReleased(button);
	}
}

