#ifndef ACCELEROMETER_HPP
#define ACCELEROMETER_HPP

#include <QObject>
#include "CopterCtrl.hpp"

class CopterCtrl;

class Accelerometer : public QObject
{
	Q_OBJECT
public:
	explicit Accelerometer(const QString inputPath, QObject *parent = 0);
	
	void adjustZeroAxis();

signals:
	void accelerometerRead(double val, CopterCtrl::AxisDimension dim);
	
public slots:
	void onRead();

private:
	double minVal, maxVal;
	int m_adjustCounter;
	int m_inputFd;
	double m_zeroAxis[CopterCtrl::NUM_DIMENSIONS];
	CopterCtrl* m_copterCtrl;
	QSocketNotifier* m_inputNotifier;
};

#endif // ACCELEROMETER_HPP
