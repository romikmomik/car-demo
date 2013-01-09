#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtGui/QApplication>
#include <QMainWindow>
#include <QLCDNumber>
#include <QString>
#include "ui_main_window.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
  public:
    MainWindow(QWidget* _parent = 0);
    ~MainWindow();

  private:
    Ui::MainWindow* m_ui;

  protected:
    class Motor
    {
      public:
        Motor(const QString& _path, QLCDNumber* _lcd);
        ~Motor();

        double factor() const;
        void factor(double _factor);

        void adjustPower(unsigned _power);

      protected:
        QLCDNumber* m_lcd;
        double m_factor;
    };

    class Axis
    {
      public:
        Axis(Motor& _motor1, Motor& _motor2);

        double tilt() const; // -1.0 .. +1.0
        void tilt(double _tilt) const;

        void adjustPower(unsigned _power);

      protected:
        Motor& m_motor1;
        Motor& m_motor2;
    };

    class CopterCtrl
    {
      public:
        CopterCtrl(Axis& _axisX, Axis& _axisY, QLCDNumber* _lcd);

        double tiltX() const { return m_axisX.tilt(); }
        double tiltY() const { return m_axisY.tilt(); }
        void tiltX(double _tilt) const { m_axisX.tilt(_tilt); m_axisX.adjustPower(m_power); }
        void tiltY(double _tilt) const { m_axisY.tilt(_tilt); m_axisY.adjustPower(m_power); }
        void adjustTilt(double _tiltX, double _tiltY) const;

        void adjustPower(unsigned _power);
      protected:
        unsigned m_power;
        Axis& m_axisX;
        Axis& m_axisY;
    };

    Motor m_motorX1;
    Motor m_motorX2;
    Motor m_motorY1;
    Motor m_motorY2;
    Axis m_axisX;
    Axis m_axisY;
    CopterCtrl m_copterCtrl;
};


#endif // !MAIN_WINDOW_H_
