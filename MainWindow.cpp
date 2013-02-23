#include <QStatusBar>

#include "Common.hpp"
#include "MainWindow.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

MainWindow::MainWindow(QWidget* _parent)
	:QMainWindow(_parent),
		m_settings(new Settings()),
		m_ui(new Ui::MainWindow()),
		m_copterCtrl()
{
	m_ui->setupUi(this);
	const auto s_ctrl_path = m_settings->getControlPath();
	QSharedPointer<CopterMotor> mx1(new CopterMotor(m_settings, s_ctrl_path+"ehrpwm.0/pwm/ehrpwm.0:0/duty_percent", m_ui->motor_x1));
	QSharedPointer<CopterMotor> mx2(new CopterMotor(m_settings, s_ctrl_path+"ehrpwm.0/pwm/ehrpwm.0:1/duty_percent", m_ui->motor_x2));
	QSharedPointer<CopterMotor> my1(new CopterMotor(m_settings, s_ctrl_path+"ehrpwm.1/pwm/ehrpwm.1:0/duty_percent", m_ui->motor_y1));
	QSharedPointer<CopterMotor> my2(new CopterMotor(m_settings, s_ctrl_path+"ehrpwm.1/pwm/ehrpwm.1:1/duty_percent", m_ui->motor_y2));
	QSharedPointer<CopterAxis>  m_axisX(new CopterAxis(mx1, mx2));
	QSharedPointer<CopterAxis>  m_axisY(new CopterAxis(my1, my2));
	m_copterCtrl = new CopterCtrl(m_settings, m_axisX, m_axisY, m_ui->motor_all);

	connect(m_copterCtrl, SIGNAL(stateChanged(CopterState)), this, SLOT(onStateChange()));
	connect(m_copterCtrl, SIGNAL(accelerometerRead(Axis)), this, SLOT(onAccelerometerRead(Axis)));

	// customize status bar appearance
	QFont statusBarFont = statusBar()->font();
	statusBarFont.setPointSize(5);
	statusBar()->setFont(statusBarFont);
	statusBar()->showMessage(m_copterCtrl->stateString());

	// temporary, move to main
	m_copterCtrl->adjustPower(0);

	showFullScreen();
}

void MainWindow::onStateChange()
{
	statusBar()->showMessage(m_copterCtrl->stateString());
}

void MainWindow::onAccelerometerRead(Axis val)
{
	m_ui->cur_accel_x->setText(QString::number(val.x));
	m_ui->cur_accel_y->setText(QString::number(val.y));
	m_ui->cur_accel_z->setText(QString::number(val.z));
}


