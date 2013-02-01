#pragma once

#include <QObject>

#if QT_VERSION >= 0x050000
    #include <QApplication>
#else
    #include <QtGui/QApplication>
#endif
#include <QMainWindow>
#include <QSharedPointer>
#include <QPointer>
#include <QSocketNotifier>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QLCDNumber>
#include <QString>
#include <QFile>
