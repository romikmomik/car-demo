#include <QStyleFactory>
#include <QDebug>
#include <QStringList>
#include <QWSServer>

#include "CopterCtrl.hpp"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef Q_WS_QWS
	QWSServer::setCursorVisible( false );
#endif
	a.setStyle(QStyleFactory::create("Cleanlooks"));

	CopterCtrl* ctrl = new CopterCtrl();
	return a.exec();
}

