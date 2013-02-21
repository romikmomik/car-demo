#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#include <QObject>
#include "CopterCtrl.hpp"

class CopterCtrl;

class Accelerometer : public QObject
{
	Q_OBJECT
public:
	explicit Accelerometer(const QString inputPath, CopterCtrl* copterCtrl, QObject *parent = 0);
	
	void adjustZeroAxis();
	Axis zeroAxis() { return m_zeroAxis; }

signals:
	void accelerometerRead(Axis val);
	void zeroAxisChanged(Axis val);

public slots:
	void onRead();

private:
	double minVal, maxVal;
	int m_adjustCounter;
	int m_inputFd;
	Axis m_zeroAxis;
	Axis m_curAxis;
	Axis m_prevAxis[5];
	int m_counter;
	CopterCtrl* m_copterCtrl;
	QSocketNotifier* m_inputNotifier;
};

#endif // ACCELEROMETER_HPP
