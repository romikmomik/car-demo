#include "main_window.h"




CopterMotor::CopterMotor(const QString& _path, QLCDNumber* _lcd)
 :m_path(_path),
  m_lcd(_lcd),
  m_factor(1.0)
{
#warning TODO init engine
#warning TODO open _path
}

CopterMotor::~CopterMotor()
{
#warning TODO close _path
#warning TODO shutdown engine
}

void CopterMotor::factor(double _factor)
{
  m_factor = max(min(_factor, 1.0), 0.0);
  unsigned factorPercentage = m_factor * 100;
  m_lcd->display(factorPercentage);
}

void CopterMotor::setPower(unsigned _power)
{
  double pwr = factorPercentage * _power;
#warning TODO write to _path
}




CopterAxis::CopterAxis(const QSharedPointer<CopterMotor>& _motor1,
                       const QSharedPointer<CopterMotor>& _motor2)
 :m_motor1(_motor1),
  m_motor2(_motor2)
{
}

double CopterAxis::tilt() const
{
  return m_motor1.factor() - m_motor2.factor();
}

void CopterAxis::tilt(double _tilt) const
{
  if (_tilt == 0)
  {
    m_motor1->factor(1.0);
    m_motor2->factor(1.0);
  }
  else if (_tilt < 0)
  {
    m_motor1->factor(1.0+_tilt);
    m_motor2->factor(1.0);
  }
  else if (_tilt > 0)
  {
    m_motor1->factor(1.0);
    m_motor2->factor(_tilt);
  }
}




CopterCtrl::CopterCtrl(const QSharedPointer<CopterAxis>& _axisX,
                       const QSharedPointer<CopterAxis>& _axisY,
                       QLCDNumber* _lcd)
 :m_lcd(_lcd),
  m_power(0),
  m_axisX(_axisX),
  m_axisY(_axisY)
{
  adjustPower(0);
}

void CopterCtrl::adjustTilt(double _tiltX, double _tiltY) const
{
  m_axisX->tilt(m_axisX->tilt() + _tiltX);
  m_axisY->tilt(m_axisY->tilt() + _tiltY);
}

void CopterCtrl::adjustPower(int _incr)
{
  m_power += _incr;
  m_power = max(min(m_power, 100), 0);
  m_lcd->display(m_power);
  m_axisX->setPower(m_power);
  m_axisY->setPower(m_power);
}




MainWindow::MainWindow(QWidget* _parent)
 :QMainWindow(_parent),
  ui(new Ui::MainWindow)
{
}



