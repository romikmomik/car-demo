TEMPLATE =      app

CONFIG +=       debug_and_release \
                warn_on \
                copy_dir_files

debug:CONFIG += console

QT +=           network

contains($$[QT_VERSION_MAJOR],5) {
    QT += widgets
}

TARGET =        copter-pwm-ctrl-qt

SOURCES +=      \
	CopterMotor.cpp \
	CopterCtrl.cpp \
	Main.cpp \
	commands/AbstractCommand.cpp \
    commands/CommandFactory.cpp \
    commands/MotorCommand.cpp \
    commands/BeepCommand.cpp \
    commands/SensorCommand.cpp \
    commands/DistanceSensorCommand.cpp \
    commands/LightSensorCommand.cpp

HEADERS +=      \
	CopterMotor.hpp \
	CopterCtrl.hpp \
    commands/CommandFactory.h \
    commands/MotorCommand.h \
    commands/BeepCommand.h \
    commands/SensorCommand.h \
    commands/DistanceSensorCommand.h \
    commands/LightSensorCommand.h \
    commands/CommandDefinitions.h \
    commands/AbstractCommand.h

FORMS +=        

QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_WARN_ON = -Wno-reorder

unix {
  target.path = $$[INSTALL_ROOT]/bin
  INSTALLS +=   target
}
