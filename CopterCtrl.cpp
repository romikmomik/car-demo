#include <QTimer>

#include "Common.hpp"
#include <cmath>
#include "CopterCtrl.hpp"
#include "accelerometer.hpp"

CopterCtrl::CopterCtrl(Settings::sptr const & settings,
											 const QSharedPointer<CopterAxis>& _axisX,
											 const QSharedPointer<CopterAxis>& _axisY,
											 QLCDNumber* _lcd)
	:m_lcd(_lcd),
		m_power(0),
		m_axisX(_axisX),
		m_axisY(_axisY),
		m_settings(settings),
		m_state(IDLE)
{
	m_accel = new Accelerometer(m_settings->getAccelInputPath(), this);
	connect(m_accel, SIGNAL(accelerometerRead(double,CopterCtrl::AxisDimension)),
					this, SIGNAL(accelerometerRead(double,CopterCtrl::AxisDimension)));
}

void CopterCtrl::adjustTilt(double _tiltX, double _tiltY) const
{
	m_axisX->tilt(m_axisX->tilt() + _tiltX);
	m_axisY->tilt(m_axisY->tilt() + _tiltY);
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
	m_state = CopterCtrl::ADJUSTING_ACCEL;

	QTimer::singleShot(m_settings->getAccelAdjustingTime(), this, SLOT(setState()));
}


