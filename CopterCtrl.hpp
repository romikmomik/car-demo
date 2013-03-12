#ifndef COPTERCTRL_HPP
#define COPTERCTRL_HPP

#include <QTcpServer>
#include <QPointer>
#include <QSocketNotifier>
#include <QSettings>
#include <QVector3D>

#include "CopterMotor.hpp"
#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif

QT_FORWARD_DECLARE_CLASS(Accelerometer)

class CopterCtrl : public QObject
{
	Q_OBJECT
public:
	CopterCtrl();

	void adjustPower(int _incr);
	void setPower(int _power);

	QSettings* getSettings() { return m_settings; }

public slots:
	void tcpLog(const QString& message);
	void emergencyStop();

protected slots:
	void onConnection();
	void onDisconnected();
	void onNetworkRead();
	void initMotors(const QString& motorControlPath);
	void initSettings();
	void adjustSettingsValue(const QString& key, bool increase = true);
	void onSettingsValueChange(const QString& key, const QVariant& value);

signals:
	void settingsValueChanged(QString key, QVariant value);

protected:
	int m_power;

	CopterMotor* m_powerMotor;
	CopterMotor* m_angleMotor;
	CopterMotor* m_cameraMotor;
	QSettings* m_settings;

	QTcpServer           m_tcpServer;
	QPointer<QTcpSocket> m_tcpConnection;
};

#endif // COPTERCTRL_HPP
