#include <QTimer>
#include <QTcpSocket>

#include <cmath>
#include "CopterCtrl.hpp"
#include "accelerometer.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

CopterCtrl::CopterCtrl(Settings::sptr const & settings,
											 const QSharedPointer<CopterAxis>& _axisX,
											 const QSharedPointer<CopterAxis>& _axisY,
											 QLCDNumber* _lcd)
	:m_lcd(_lcd),
		m_power(0),
		m_axisX(_axisX),
		m_axisY(_axisY),
		m_settings(settings),
		m_state(IDLE),
		m_tcpServer(),
		m_tcpConnection()
{
	m_tcpServer.listen(QHostAddress::Any, m_settings->getTcpPort());
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));

	// buttons reading
	auto const s_buttons_input_path = m_settings->getButtonsInputPath();
	m_buttonsInputFd = ::open(s_buttons_input_path.toLatin1().data(), O_SYNC, O_RDONLY);
	if (m_buttonsInputFd == -1)
		qDebug() << "Cannot open buttons input file " << s_buttons_input_path << ", reason: " << errno;

	m_buttonsInputNotifier = new QSocketNotifier(m_buttonsInputFd, QSocketNotifier::Read, this);
	connect(m_buttonsInputNotifier, SIGNAL(activated(int)), this, SLOT(onButtonRead()));
	m_buttonsInputNotifier->setEnabled(true);

	m_accel = new Accelerometer(m_settings->getAccelInputPath(), this);
	connect(m_accel, SIGNAL(accelerometerRead(Axis)),
					this, SIGNAL(accelerometerRead(Axis)));
	connect(m_accel, SIGNAL(accelerometerRead(Axis)), this, SLOT(handleTilt(Axis)));
	connect(m_accel, SIGNAL(zeroAxisChanged(Axis)), this, SIGNAL(zeroAxisChanged(Axis)));
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
	static const auto s_power_min = m_settings->getPowerMin();
	static const auto s_power_max = m_settings->getPowerMax();

	m_power += _incr;
	m_power = qMax(qMin(m_power, s_power_max), s_power_min);

	QPalette palette = m_lcd->palette();
	QColor bg = palette.color(QPalette::Disabled, m_lcd->backgroundRole());
	double pwrSat = 1.0 - static_cast<double>(m_power-s_power_min)/(s_power_max-s_power_min);
	bg.setBlue( bg.blue() *pwrSat);
	bg.setGreen(bg.green()*pwrSat + 0xff*(1.0-pwrSat));
	bg.setRed(  bg.red()  *pwrSat);
	palette.setColor(QPalette::Normal, m_lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Active, m_lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Inactive, m_lcd->backgroundRole(), bg);
	m_lcd->setPalette(palette);
	m_lcd->display(m_power);

	m_axisX->setPower(m_power);
	m_axisY->setPower(m_power);
}

void CopterCtrl::adjustAccel()
{
	if (m_state != CopterCtrl::IDLE)
		return;
	setState(CopterCtrl::ADJUSTING_ACCEL);

	QTimer::singleShot(m_settings->getAccelAdjustingTime(), this, SLOT(setState()));
}

void CopterCtrl::onAccelerometerRead(Axis val)
{
}

void CopterCtrl::handleTilt(Axis tilt)
{
	static const auto s_accel_linear = m_settings->getAccelLinear();
	static const auto s_accel_derivative = m_settings->getAccelDerivative();
	Axis adj = tilt * s_accel_linear + (tilt - m_lastTilt) * s_accel_derivative;
	adjustTilt(adj);
	m_lastTilt = tilt;

//	double adj = s_accel_linear*_tilt + s_accel_derivative*(_tilt - m_lastTiltX);
//	m_copterCtrl->adjustTilt(adj, 0);
//	m_lastTiltX = _tilt;
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

	static const auto s_tilt_step = m_settings->getTiltStep();
	static const auto s_power_max = m_settings->getPowerMax();
	//static const auto s_power_min = m_settings->getPowerMin();
	static const auto s_power_step1 = m_settings->getPowerStep1();
	static const auto s_power_step2 = m_settings->getPowerStep2();

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

	if (button == Button3) {
		adjustAccel();
	}

	if (static_cast<bool>(evt.value)) {
		emit buttonPressed(button);
	}
	else {
		emit buttonReleased(button);
	}
}

