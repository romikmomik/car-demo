#include "MainWindow.hpp"
#include <QStyleFactory>
#include <QDebug>
#include <QStringList>
#include <QWSServer>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

#ifdef Q_WS_QWS
	QWSServer::setCursorVisible( false );
#endif

	a.setStyle(QStyleFactory::create("Cleanlooks"));

	MainWindow w;
	w.show();
	return a.exec();
}

