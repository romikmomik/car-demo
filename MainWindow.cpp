#include <QStatusBar>

#include "MainWindow.hpp"

MainWindow::MainWindow(CopterCtrl *copterCtrl, QWidget* _parent) :
	QMainWindow(_parent),
	m_copterCtrl(copterCtrl),
	m_ui(new Ui::MainWindow())
{
	m_settings = m_copterCtrl->getSettings();
	m_ui->setupUi(this);

	connect(m_copterCtrl, SIGNAL(stateChanged(CopterState)), this, SLOT(onStateChange()));
	connect(m_copterCtrl, SIGNAL(accelerometerRead(Axis)), this, SLOT(onAccelerometerRead(Axis)));
	connect(m_copterCtrl, SIGNAL(motorPowerChanged(CopterCtrl::Motor,double)),
					this, SLOT(onMotorPowerChange(CopterCtrl::Motor,double)));

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

void MainWindow::onMotorPowerChange(CopterCtrl::Motor motor, double power)
{
	QLCDNumber* lcd;
	switch (motor) {
		case CopterCtrl::MotorX1: lcd = m_ui->motor_x1; break;
		case CopterCtrl::MotorX2: lcd = m_ui->motor_x2; break;
		case CopterCtrl::MotorY1: lcd = m_ui->motor_y1; break;
		case CopterCtrl::MotorY2: lcd = m_ui->motor_y2; break;
		case CopterCtrl::MotorAll: lcd = m_ui->motor_all; break;
	}

	QPalette palette = lcd->palette();
	QColor bg = palette.color(QPalette::Disabled, lcd->backgroundRole());
	double powerMax = m_settings->value("PowerMax").toDouble();
	double powerMin = m_settings->value("PowerMin").toDouble();
	double pwrSat = 1.0 - static_cast<double>((power - powerMin) / (2 * powerMax - powerMin));
	bg.setBlue(bg.blue()   * pwrSat);
	bg.setGreen(bg.green() * pwrSat + 0xff * (1.0 - pwrSat));
	bg.setRed(bg.red()     * pwrSat);
	palette.setColor(QPalette::Normal, lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Active, lcd->backgroundRole(), bg);
	palette.setColor(QPalette::Inactive, lcd->backgroundRole(), bg);
	lcd->setPalette(palette);
	lcd->display(power);
}


