#ifndef COPTERCTRL_HPP
#define COPTERCTRL_HPP

#include "Common.hpp"
#include "Settings.hpp"
#include "CopterAxis.hpp"
//#include "accelerometer.hpp"

QT_FORWARD_DECLARE_CLASS(Accelerometer)

struct Axis {
	Axis(double _x = 0, double _y = 0, double _z = 0) :
		x(_x), y(_y), z(_z) {}

	Axis operator +(const Axis& other) {
		return Axis(x + other.x, y + other.y, z + other.z);
	}
	Axis operator -(const Axis& other) {
		return Axis(x - other.x, y - other.y, z - other.z);
	}
	Axis operator *(const double scalar) {
		return Axis (x * scalar, y * scalar, z * scalar);
	}
	Axis operator /(const double scalar) {
		return Axis (x / scalar, y / scalar, z / scalar);
	}

	Axis& operator =(const Axis & other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	double x, y, z;
};

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
	void adjustTilt(double tiltX, double tiltY) const { Axis tilt(tiltX, tiltY); adjustTilt(tilt); }
	void adjustTilt(Axis tilt) const;
	void adjustPower(int _incr);
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
	void setState(CopterState _state = IDLE) { m_state = _state; emit stateChanged(m_state); }
	void adjustAccel();
	void handleTilt(Axis tilt);

protected slots:
	void onAccelerometerRead(Axis val);

signals:
	void lcdUpdate(int);
	void stateChanged(CopterState state);
	void accelerometerRead(Axis val);
	void zeroAxisChanged(Axis val);

protected:
	QLCDNumber* m_lcd;
	int m_power;
	QSharedPointer<CopterAxis> m_axisX;
	QSharedPointer<CopterAxis> m_axisY;

	Axis m_lastTilt;

	Accelerometer* m_accel;
	CopterState m_state;
};

#endif // COPTERCTRL_HPP
