#include "main_window.h"


MainWindow::MainWindow(QWidget* _parent)
 :QMainWindow(_parent),
  ui(new Ui::MainWindow),
  m_motorX1("pwm1", ui->motor_x1),
  m_motorX2("pwm2", ui->motor_x2),
  m_motorY1("pwm3", ui->motor_y1),
  m_motorY2("pwm4", ui->motor_y2),
  m_axisX(m_motorX1, m_motorX2),
  m_axisY(m_motorY1, m_motorY2),
  m_power(m_axisX, m_axisY, ui->motor_all)
{
}

MainWindow::~MainWindow()
{
}

#if 0
  protected:
    class Motor
    {
      public:
        Motor(const QString& _path, QLCDNumber* _lcd);
        ~Motor();

        double factor() const
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
        void adjustTilt(double _incr) const; // tilt increment

        void adjustPower(unsigned _power);
    };

    Motor m_motorX1;
    Motor m_motorX2;
    Motor m_motorY1;
    Motor m_motorY2;
    Axis m_axisX;
    Axis m_axisY;

  private:
    Ui::MainWindow* m_ui;
};


#endif
