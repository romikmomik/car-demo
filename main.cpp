#include <QtGui/QApplication>

#include <QStyleFactory>
#include <QDebug>
#include <QStringList>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  a.setStyle(QStyleFactory::create("Cleanlooks"));

  MainWindow w;
  w.show();
  return a.exec();
}
