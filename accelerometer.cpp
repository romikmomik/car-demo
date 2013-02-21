#include "accelerometer.hpp"
#include "Settings.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <cmath>

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
			emit accelerometerRead(filterAxis(m_curAxis));
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

Axis Accelerometer::filterAxis(Axis axis)
{
//	return filterMean(axis);
	return filterKalman(axis);
}

Axis Accelerometer::filterMean(Axis axis)
{
	Axis countedAxis;
	m_prevAxis[m_counter] = axis - m_zeroAxis;
	m_counter = (m_counter + 1) % 5;
	for (int i = 0; i < 5; ++i) countedAxis = countedAxis + m_prevAxis[i] / 5;
	return countedAxis;
}

Axis Accelerometer::filterKalman(Axis axis)
{
	static double s_sigma_psi = m_copterCtrl->getSettings()->getKalmanSigmaPsi();
	static double s_sigma_eta = m_copterCtrl->getSettings()->getKalmanSigmaEta();

	static double s_e_opt = s_sigma_eta;
	static Axis s_axis_opt = axis;
	static bool s_first_time = true;
	if (s_first_time) return s_axis_opt;
	s_first_time = false;

	s_e_opt = sqrt(pow(s_sigma_eta, 2) * (pow(s_e_opt, 2) + pow(s_sigma_psi, 2)) /
								 (pow(s_sigma_eta, 2) + pow(s_e_opt, 2) + pow(s_sigma_psi, 2)));
	// Kalman coeff
	double k = pow(s_e_opt, 2) / pow(s_sigma_eta, 2);
	s_axis_opt = s_axis_opt * (1 - k) + axis * k;
	return s_axis_opt;
}

void Accelerometer::adjustZeroAxis()
{
	m_adjustCounter = 0;
	m_zeroAxis.x = m_zeroAxis.y = m_zeroAxis.z = 0;
}

