#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_
#include "Common.hpp"
#include "Settings.hpp"
#include "ui_MainWindow.h"
#include "CopterCtrl.hpp"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* _parent = 0);
	enum BoardButton {
		Button1 = 0,
		Button2,
		Button3,
		Button4,
		Button5,
		Button6,
		Button7,
		Button8,
		NUM_BUTTONS
	};

signals:
	void buttonPressed(BoardButton button);
	void buttonReleased(BoardButton button);

protected:
	QPointer<CopterCtrl> m_copterCtrl;
	QTcpServer           m_tcpServer;
	QPointer<QTcpSocket> m_tcpConnection;
	int                  m_accelerometerInputFd;
	QPointer<QSocketNotifier> m_accelerometerInputNotifier;
	int                  m_buttonsInputFd;
	QPointer<QSocketNotifier> m_buttonsInputNotifier;

	void handleTiltX(double _tilt);
	void handleTiltY(double _tilt);
	double m_lastTiltX;
	double m_lastTiltY;

protected slots:
	void onConnection();
	void onDisconnected();
	void onNetworkRead();
	void onAccelerometerRead();
	void onButtonRead();

private:
	Ui::MainWindow* m_ui;
	Settings::sptr m_settings;

};


#endif // !MAIN_WINDOW_H_
