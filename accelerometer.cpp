#include "accelerometer.hpp"

Accelerometer::Accelerometer(const QString inputPath, QObject *parent) :
	QObject(parent),
	m_inputFd(-1),
	m_inputNotifier(0)
{
	m_inputFd = ::open(inputPath.toLatin1().data(), O_SYNC, O_RDONLY);
	if (m_inputFd == -1)
		qDebug() << "Cannot open accelerometer input file " << inputPath << ", reason: " << errno;

	m_inputNotifier = new QSocketNotifier(m_inputFd, QSocketNotifier::Read, this);
	connect(m_inputNotifier, SIGNAL(activated(int)), this, SLOT(onRead()));
	m_inputNotifier->setEnabled(true);
}


void Accelerometer::onRead()
{
	struct input_event evt;

	if (read(m_inputFd, reinterpret_cast<char*>(&evt), sizeof(evt)) != sizeof(evt))
	{
		qDebug() << "Incomplete accelerometer data read";
		return;
	}

	if (evt.type != EV_ABS)
	{
		if (evt.type != EV_SYN)
			qDebug() << "Input event type is not EV_ABS or EV_SYN: " << evt.type;
		else {
			if (m_copterCtrl->state() == CopterCtrl::ADJUSTING_ACCEL)
				++m_adjustCounter;
		}
		return;
	}

	switch (evt.code)
	{
		case ABS_X:
			if (m_copterCtrl->state() == CopterCtrl::ADJUSTING_ACCEL) {
				zeroAxis[CopterCtrl::X] = ((m_adjustCounter * zeroAxis[CopterCtrl::X] + evt.value) / (m_adjustCounter + 1));
			}
			emit accelerometerRead(evt.val, CopterCtrl::X);
			break;
		case ABS_Y:
			if (m_copterCtrl->state() == CopterCtrl::ADJUSTING_ACCEL) {
				zeroAxis[CopterCtrl::Y] = ((m_adjustCounter * zeroAxis[CopterCtrl::Y] + evt.value) / (m_adjustCounter + 1));
			}
			emit accelerometerRead(evt.val, CopterCtrl::Y);
			break;
		case ABS_Z:
			if (m_copterCtrl->state() == CopterCtrl::ADJUSTING_ACCEL) {
				zeroAxis[CopterCtrl::Z] = ((m_adjustCounter * zeroAxis[CopterCtrl::Z] + evt.value) / (m_adjustCounter + 1));
			}
			emit accelerometerRead(evt.val, CopterCtrl::Z);
			break;
	}
}

void Accelerometer::adjustZeroAxis()
{
	if (m_copterCtrl->state() != CopterCtrl::IDLE)
		return;

	m_copterCtrl->setState(CopterCtrl::ADJUSTING_ACCEL);
	m_adjustCounter = 0;

	// TODO: move timer to ctrl class
	QTimer::singleShot(m_settings->getAccelAdjustingTime(), m_copterCtrl, SLOT(setState()));
}

