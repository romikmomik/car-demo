#include "main_window.h"

#include <QtGlobal>
#include <QStringList>
#include <QIODevice>
#include <QPalette>
#include <QColor>

#include <math.h>
#include <linux/input.h>


//static const QString s_ctrl_path("/sys/device/platform/ehrpwm");
static const QString s_ctrl_path("");
static const QString s_accel_path("/dev/input/event3");
static const double s_tilt_step = 0.02;
static const double s_power_step = 1;
static const double s_accel_linear = 0.02;
static const double s_accel_derivative = 0.04;




void CopterMotor::invoke_open()
{
  invoke(0);
}

void CopterMotor::invoke_close()
{
  invoke(0);
}

void CopterMotor::invoke(int _power)
{
  QString s;
  m_ctrlFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered|QIODevice::Text);
  s.sprintf("%d\n", _power);
  m_ctrlFile.write(s.toAscii().data());
  m_ctrlFile.close();
}

CopterMotor::CopterMotor(const QString& _ctrlPath, QLCDNumber* _lcd)
 :m_lcd(_lcd),
  m_ctrlFile(_ctrlPath),
  m_factor(1.0)
{
  invoke_open();
}

CopterMotor::~CopterMotor()
{
  invoke_close();
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

  invoke(pwr);
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
  m_tcpConnection(),
  m_accelerometerFile(s_accel_path),
  m_lastTiltX(0),
  m_lastTiltY(0)
{
  m_ui->setupUi(this);

  QSharedPointer<CopterMotor> mx1(new CopterMotor(s_ctrl_path+"0.0", m_ui->motor_x1));
  QSharedPointer<CopterMotor> mx2(new CopterMotor(s_ctrl_path+"0.1", m_ui->motor_x2));
  QSharedPointer<CopterMotor> my1(new CopterMotor(s_ctrl_path+"1.0", m_ui->motor_y1));
  QSharedPointer<CopterMotor> my2(new CopterMotor(s_ctrl_path+"1.1", m_ui->motor_y2));
  QSharedPointer<CopterAxis>  m_axisX(new CopterAxis(mx1, mx2));
  QSharedPointer<CopterAxis>  m_axisY(new CopterAxis(my1, my2));
  m_copterCtrl = new CopterCtrl(m_axisX, m_axisY, m_ui->motor_all);

  m_tcpServer.listen(QHostAddress::Any, 4000);
  connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(onConnection()));

  m_accelerometerFile.open(QIODevice::ReadOnly|QIODevice::Unbuffered);
  connect(&m_accelerometerFile, SIGNAL(readyRead()), this, SLOT(onAccelerometerRead()));

  m_copterCtrl->adjustPower(0);

  showFullScreen();
  showMaximized();
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
    if (!m_tcpConnection->getChar(&c))
      break;
    switch (c)
    {
      case '1': m_copterCtrl->adjustTilt(-s_tilt_step, -s_tilt_step); break;
      case '2': m_copterCtrl->adjustTilt(0,            -s_tilt_step); break;
      case '3': m_copterCtrl->adjustTilt(+s_tilt_step, -s_tilt_step); break;
      case '4': m_copterCtrl->adjustTilt(-s_tilt_step, 0); break;
      case '5': m_copterCtrl->tiltX(0); m_copterCtrl->tiltY(0); break;
      case '6': m_copterCtrl->adjustTilt(+s_tilt_step, 0); break;
      case '7': m_copterCtrl->adjustTilt(-s_tilt_step, +s_tilt_step); break;
      case '8': m_copterCtrl->adjustTilt(0,            +s_tilt_step); break;
      case '9': m_copterCtrl->adjustTilt(+s_tilt_step, +s_tilt_step); break;
      case 'Z': m_copterCtrl->adjustPower(-10000); break;
      case 'z': m_copterCtrl->adjustPower(-5*s_power_step); break;
      case 'x': m_copterCtrl->adjustPower(-s_power_step); break;
      case 'c': m_copterCtrl->adjustPower(+s_power_step); break;
      case 'v': m_copterCtrl->adjustPower(+5*s_power_step); break;
      case 'V': m_copterCtrl->adjustPower(+10000); break;
    }
  }
}

void MainWindow::onAccelerometerRead()
{
  struct input_event evt;

  while (m_accelerometerFile.isReadable())
  {
    if (m_accelerometerFile.read(reinterpret_cast<char*>(&evt), sizeof(evt)) != sizeof(evt))
    {
      qDebug() << "Incomplete accelerometer data read";
      continue;
    }

    if (evt.type != EV_ABS)
    {
      qDebug() << "Input event type is not ABS";
      continue;
    }

    if (m_tcpConnection.isNull())
      continue;

    char code = 0;
    switch (evt.code)
    {
      case ABS_X: code = 'x'; handleTiltX(evt.value); break;
      case ABS_Y: code = 'y'; handleTiltY(evt.value); break;
      case ABS_Z: code = 'z'; break;
    }
    if (code == 0)
      continue;
    QString buf;
    buf.sprintf("%c%u ", code, evt.value);
    m_tcpConnection->write(buf.toAscii());
  }
}

void MainWindow::handleTiltX(double _tilt)
{
  double adj = s_accel_linear*_tilt + s_accel_derivative*(_tilt - m_lastTiltX);
  m_copterCtrl->adjustPower(adj, 0);
  m_lastTiltX = _tilt;
}

void MainWindow::handleTiltY(double _tilt)
{
  double adj = s_accel_linear*_tilt + s_accel_derivative*(_tilt - m_lastTiltY);
  m_copterCtrl->adjustPower(0, adj);
  m_lastTiltY = _tilt;
}



