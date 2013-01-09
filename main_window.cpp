#include "main_window.h"

#include <QtGlobal>
#include <QStringList>
#include <QProcess>



void CopterMotor::invoke(const QStringList& _args)
{
  QProcess prc;
  QStringList args(_args);
  args.push_front(m_ctrlArg);

  prc.start(m_ctrlPath, args);
  prc.waitForFinished();
}

CopterMotor::CopterMotor(const QString& _ctrlPath, const QString& _ctrlArg, QLCDNumber* _lcd)
 :m_ctrlPath(_ctrlPath),
  m_ctrlArg(_ctrlArg),
  m_factor(1.0)
{
  invoke(QStringList("open"));

  connect(this, SIGNAL(lcdUpdate(int)), _lcd, SLOT(display(int)));
}

CopterMotor::~CopterMotor()
{
  invoke(QStringList("close"));
}

void CopterMotor::factor(double _factor)
{
  m_factor = qMax(qMin(_factor, 1.0), 0.0);
  int factorPercentage = m_factor * 100;
  emit lcdUpdate(factorPercentage);
}

void CopterMotor::setPower(unsigned _power)
{
  int pwr = m_factor * (double)_power;

  QStringList args;
  args.push_back("set");
  args.push_back(QString::number(pwr));
  invoke(args);
}




CopterAxis::CopterAxis(const QSharedPointer<CopterMotor>& _motor1,
                       const QSharedPointer<CopterMotor>& _motor2)
 :m_motor1(_motor1),
  m_motor2(_motor2)
{
}

double CopterAxis::tilt() const
{
  return m_motor1->factor() - m_motor2->factor();
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
 :m_power(0),
  m_axisX(_axisX),
  m_axisY(_axisY)
{
  connect(this, SIGNAL(lcdUpdate(int)), _lcd, SLOT(display(int)));
}

void CopterCtrl::adjustTilt(double _tiltX, double _tiltY) const
{
  m_axisX->tilt(m_axisX->tilt() + _tiltX);
  m_axisY->tilt(m_axisY->tilt() + _tiltY);
}

void CopterCtrl::adjustPower(int _incr)
{
  m_power += _incr;
  m_power = qMax(qMin(m_power, 100), 0);
  emit lcdUpdate(m_power);
  m_axisX->setPower(m_power);
  m_axisY->setPower(m_power);
}




MainWindow::MainWindow(QWidget* _parent)
 :QMainWindow(_parent),
  m_ui(new Ui::MainWindow),
  m_copterCtrl(),
  m_tcpServer(),
  m_tcpConnection()
{
  m_ui->setupUi(this);

  QSharedPointer<CopterMotor> mx1(new CopterMotor("pwm-ctrl-helper", "0.0", m_ui->motor_x1));
  QSharedPointer<CopterMotor> mx2(new CopterMotor("pwm-ctrl-helper", "0.1", m_ui->motor_x2));
  QSharedPointer<CopterMotor> my1(new CopterMotor("pwm-ctrl-helper", "1.0", m_ui->motor_y1));
  QSharedPointer<CopterMotor> my2(new CopterMotor("pwm-ctrl-helper", "1.1", m_ui->motor_y2));
  QSharedPointer<CopterAxis>  m_axisX(new CopterAxis(mx1, mx2));
  QSharedPointer<CopterAxis>  m_axisY(new CopterAxis(my1, my2));
  m_copterCtrl = new CopterCtrl(m_axisX, m_axisY, m_ui->motor_all);

  m_tcpServer.listen(QHostAddress::Any, 4000);
  connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));

  m_copterCtrl->adjustPower(0);
}

void MainWindow::onConnection()
{
  if (!m_tcpConnection.isNull())
    qDebug() << "Replacing existing connection\n";
  m_tcpConnection = m_tcpServer.nextPendingConnection();
  qDebug() << "Accepted new connection\n";
  connect(m_tcpConnection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
  connect(m_tcpConnection, SIGNAL(readyRead()), this, SLOT(onNetworkRead()));
}

void MainWindow::onDisconnected()
{
  qDebug() << "Existing connection disconnected\n";
  m_tcpConnection = 0;
}

void MainWindow::onNetworkRead()
{
  if (m_tcpConnection.isNull())
    return;

  while (m_tcpConnection->isReadable())
  {
    char c;
    static const double s_tilt_step = 0.05;
    static const double s_power_step = 1;
    m_tcpConnection->getChar(&c);
    switch (c)
    {
      case '1': m_copterCtrl->adjustTilt(-s_tilt_step, -s_tilt_step); break;
      case '2': m_copterCtrl->adjustTilt(0,            -s_tilt_step); break;
      case '3': m_copterCtrl->adjustTilt(+s_tilt_step, -s_tilt_step); break;
      case '4': m_copterCtrl->adjustTilt(-s_tilt_step, 0); break;
      case '5': m_copterCtrl->adjustTilt(0,            0); break;
      case '6': m_copterCtrl->adjustTilt(+s_tilt_step, 0); break;
      case '7': m_copterCtrl->adjustTilt(-s_tilt_step, +s_tilt_step); break;
      case '8': m_copterCtrl->adjustTilt(0,            +s_tilt_step); break;
      case '9': m_copterCtrl->adjustTilt(+s_tilt_step, +s_tilt_step); break;
      case 'z': m_copterCtrl->adjustPower(-10*s_power_step); break;
      case 'x': m_copterCtrl->adjustPower(-s_power_step); break;
      case 'c': m_copterCtrl->adjustPower(+s_power_step); break;
      case 'v': m_copterCtrl->adjustPower(+10*s_power_step); break;
    }
  }
}

