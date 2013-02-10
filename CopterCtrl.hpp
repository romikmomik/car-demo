#pragma once
#include "Common.hpp"
#include "Settings.hpp"
#include "CopterAxis.hpp"

class CopterCtrl : public QObject
{
	Q_OBJECT
	Settings::sptr m_settings;
public:
	CopterCtrl(Settings::sptr const & settings,
						 const QSharedPointer<CopterAxis>& _axisX,
						 const QSharedPointer<CopterAxis>& _axisY,
						 QLCDNumber* _lcd);

	double tiltX() const { return m_axisX->tilt(); }
	double tiltY() const { return m_axisY->tilt(); }
	void tiltX(double _tilt) const { m_axisX->tilt(_tilt); m_axisX->setPower(m_power); }
	void tiltY(double _tilt) const { m_axisY->tilt(_tilt); m_axisY->setPower(m_power); }
	void adjustTilt(double _tiltX, double _tiltY) const;

	void adjustPower(int _incr);
protected:
	QLCDNumber* m_lcd;
	int m_power;
	QSharedPointer<CopterAxis> m_axisX;
	QSharedPointer<CopterAxis> m_axisY;

signals:
	void lcdUpdate(int);
};
