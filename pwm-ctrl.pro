TEMPLATE =      app

CONFIG +=       debug_and_release \
                warn_on \
                copy_dir_files

debug:CONFIG += console

CONFIG -=       warn_off

QT +=           network

contains($$[QT_VERSION_MAJOR],5) {
    QT += widgets
}

TARGET =        car-ctrl
SOURCES +=      \
    Main.cpp \
    Sensor.cpp \
    CarCtrl.cpp \
    CarMotor.cpp

HEADERS +=      \
    Sensor.hpp \
    CarCtrl.hpp \
    CarMotor.hpp

FORMS +=        

QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_WARN_ON = -Wno-reorder

unix {
  target.path = $$[INSTALL_ROOT]/bin
  INSTALLS +=   target
}
