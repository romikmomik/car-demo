#pragma once

#ifndef COPTERCTRL_HPP
#define COPTERCTRL_HPP

#include <QTcpServer>
#include <QPointer>
#include <QSocketNotifier>
#include <QSettings>

#include "CopterMotor.hpp"
#include "commands/CommandFactory.h"

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
	QSettings* getSettings() { return m_settings; }

public slots:
	void tcpLog(const QString& message);
	void emergencyStop();

protected slots:
	void onConnection();
	void onDisconnected();
	void onNetworkRead();
	void onAndroidConnection();
	void onAndroidDisconnected();
	void onAndroidNetworkRead();
	void initMotors(const QString& motorControlPath);
	void initSettings();

signals:
	void settingsValueChanged(QString const &key, QVariant const &value);

protected:
	CopterMotor* m_powerMotor;
	CopterMotor* m_angleMotor;
	QSettings* m_settings;

	QTcpServer           m_tcpServer;
	QPointer<QTcpSocket> m_tcpConnection;
	QTcpServer           m_androidServer;
	QPointer<QTcpSocket> m_androidConnection;

	commands::CommandFactory mFactory;
};

#endif // COPTERCTRL_HPP
