#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_
#include "Settings.hpp"
#include "ui_MainWindow.h"
#include "CopterCtrl.hpp"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(CopterCtrl* copterCtrl, QWidget* _parent = 0);

public slots:
	void onStateChange();
	void onAccelerometerRead(Axis val);
	void onMotorPowerChange(CopterCtrl::Motor motor, double power);

private:
	CopterCtrl* m_copterCtrl;
	Ui::MainWindow* m_ui;
	Settings::sptr m_settings;
};


#endif // !MAIN_WINDOW_H_
