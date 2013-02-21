#include "accelerometer.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

Accelerometer::Accelerometer(const QString inputPath, CopterCtrl* copterCtrl, QObject *parent) :
	QObject(parent),
	m_inputFd(-1),
	m_inputNotifier(0),
	m_adjustCounter(0),
	m_copterCtrl(copterCtrl),
	m_zeroAxis(),
	m_curAxis(),
	m_counter(0)
{
	for (int i = 0; i < 5; ++i) m_prevAxis[i] = Axis();
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
			if (m_copterCtrl->state() == CopterCtrl::ADJUSTING_ACCEL) {
				m_zeroAxis = (m_zeroAxis * m_adjustCounter + m_curAxis) / (m_adjustCounter + 1);
				++m_adjustCounter;
				emit zeroAxisChanged(m_zeroAxis);
			}
			Axis countedAxis;
			m_prevAxis[m_counter] = m_curAxis - m_zeroAxis;
			m_counter = (m_counter + 1) % 5;
			for (int i = 0; i < 5; ++i) countedAxis = countedAxis + m_prevAxis[i] / 5;
			emit accelerometerRead(countedAxis);
		}
		return;
	}

	switch (evt.code)
	{
		case ABS_X:
			m_curAxis.x = evt.value;
			break;
		case ABS_Y:
			m_curAxis.y = evt.value;
			break;
		case ABS_Z:
			m_curAxis.z = evt.value;
			break;
	}
}

void Accelerometer::adjustZeroAxis()
{
	m_adjustCounter = 0;
	m_zeroAxis.x = m_zeroAxis.y = m_zeroAxis.z = 0;
}

