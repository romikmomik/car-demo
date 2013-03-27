#include <QStyleFactory>
#include <QDebug>
#include <QStringList>
#include <QWSServer>

#include "CarCtrl.hpp"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef Q_WS_QWS
	QWSServer::setCursorVisible( false );
#endif
	a.setStyle(QStyleFactory::create("Cleanlooks"));

	CarCtrl ctrl;
	Q_UNUSED(ctrl)
	return a.exec();
}

