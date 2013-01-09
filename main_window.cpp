#include "main_window.h"

#include <QtGlobal>
#include <QStringList>
#include <QProcess>
#include <QPalette>
#include <QColor>

#include <math.h>


void CopterMotor::invoke(const QStringList& _args)
{
  QProcess prc;
  QStringList args(_args);
  args.push_front(m_ctrlPath);

  prc.start("pwm-ctrl-helper", args);
  prc.waitForFinished();
}

CopterMotor::CopterMotor(const QString& _ctrlPath, QLCDNumber* _lcd)
 :m_lcd(_lcd),
  m_ctrlPath(_ctrlPath),
  m_factor(1.0)
{
  invoke(QStringList("open"));
}

CopterMotor::~CopterMotor()
{
  invoke(QStringList("close"));
}

void CopterMotor::factor(double _factor)
{
  m_factor = qMax(qMin(_factor, 1.0), 0.0);
}

void CopterMotor::setPower(unsigned _power)
{
  int pwr = round(m_factor * (double)_power);

  QPalette palette = m_lcd->palette();
  QColor bg = palette.color(QPalette::Disabled, m_lcd->backgroundRole());
  double pwrSat = (100.0-_power)/100.0;
  double ftrSat = m_factor;
  bg.setBlue( bg.blue() *pwrSat);
  bg.setGreen(bg.green()*pwrSat + 200*(1.0-pwrSat)*ftrSat);
  bg.setRed(  bg.red()  *pwrSat + 200*(1.0-pwrSat)*(1-ftrSat));
  palette.setColor(QPalette::Normal, m_lcd->backgroundRole(), bg);
  palette.setColor(QPalette::Active, m_lcd->backgroundRole(), bg);
  palette.setColor(QPalette::Inactive, m_lcd->backgroundRole(), bg);
  m_lcd->setPalette(palette);
  m_lcd->display(pwr);

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
    m_motor2->factor(1.0-_tilt);
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
  m_power += _incr;
  m_power = qMax(qMin(m_power, 100), 0);

  QPalette palette = m_lcd->palette();
  QColor bg = palette.color(QPalette::Disabled, m_lcd->backgroundRole());
  double pwrSat = (100.0-m_power)/100.0;
  bg.setBlue( bg.blue() *pwrSat);
  bg.setGreen(bg.green()*pwrSat + 200*(1.0-pwrSat));
  bg.setRed(  bg.red()  *pwrSat);
  palette.setColor(QPalette::Normal, m_lcd->backgroundRole(), bg);
  palette.setColor(QPalette::Active, m_lcd->backgroundRole(), bg);
  palette.setColor(QPalette::Inactive, m_lcd->backgroundRole(), bg);
  m_lcd->setPalette(palette);
  m_lcd->display(m_power);

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

  QSharedPointer<CopterMotor> mx1(new CopterMotor("0.0", m_ui->motor_x1));
  QSharedPointer<CopterMotor> mx2(new CopterMotor("0.1", m_ui->motor_x2));
  QSharedPointer<CopterMotor> my1(new CopterMotor("1.0", m_ui->motor_y1));
  QSharedPointer<CopterMotor> my2(new CopterMotor("1.1", m_ui->motor_y2));
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
    qDebug() << "Replacing existing connection";
  m_tcpConnection = m_tcpServer.nextPendingConnection();
  qDebug() << "Accepted new connection";
  m_tcpConnection->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  connect(m_tcpConnection, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
  connect(m_tcpConnection, SIGNAL(readyRead()), this, SLOT(onNetworkRead()));
}

void MainWindow::onDisconnected()
{
  qDebug() << "Existing connection disconnected";
  m_tcpConnection = 0;
}

void MainWindow::onNetworkRead()
{
  if (m_tcpConnection.isNull())
    return;

  while (m_tcpConnection->isReadable())
  {
    char c;
    static const double s_tilt_step = 0.02;
    static const double s_power_step = 1;
    if (!m_tcpConnection->getChar(&c))
      break;
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
      case 'z': m_copterCtrl->adjustPower(-5*s_power_step); break;
      case 'x': m_copterCtrl->adjustPower(-s_power_step); break;
      case 'c': m_copterCtrl->adjustPower(+s_power_step); break;
      case 'v': m_copterCtrl->adjustPower(+5*s_power_step); break;
    }
  }
}

