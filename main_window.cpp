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

