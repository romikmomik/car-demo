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
	enum AxisDimension { X = 0,
											 Y,
											 Z,
											 NUM_DIMENSIONS };
	const double accelAxis(AxisDimension _dim) { return m_accelAxis[_dim]; }
	enum CopterState { IDLE = 0,
										 ADJUSTING_ACCEL,
										 NUM_STATES };
	const CopterState state() { return m_state; }
	const QString stateString() {
		switch (m_state) {
			case IDLE: return QString("Idling..."); break;
			case ADJUSTING_ACCEL: return QString("Adjusting accelerometer axis..."); break;
			default: return QString("Unknown status"); break;
		}
	}

public slots:
	void accelAxis(double _val, AxisDimension _dim) { m_accelAxis[_dim] = _val; }
	void setState(CopterState _state = IDLE) { m_state = _state; emit stateChanged(m_state); }

signals:
	void lcdUpdate(int);
	void stateChanged(CopterState state);

protected:
	QLCDNumber* m_lcd;
	int m_power;
	QSharedPointer<CopterAxis> m_axisX;
	QSharedPointer<CopterAxis> m_axisY;

	double m_accelAxis[NUM_DIMENSIONS];
	CopterState m_state;
};
