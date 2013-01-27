#message(Qt version: $$[QT_VERSION_MAJOR])

TEMPLATE =      app

CONFIG +=       debug_and_release \
                warn_on \
                copy_dir_files

debug:CONFIG += console

CONFIG -=       warn_off

QT +=           network widgets

TARGET =        copter-pwm-ctrl-qt

SOURCES +=      main.cpp \
                main_window.cpp \
    settings.cpp

HEADERS +=      main_window.h \
    settings.h \
    settings_config.inc

FORMS +=        main_window.ui

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_ON = -Wno-reorder

unix {
  target.path = $$[INSTALL_ROOT]/bin
  INSTALLS +=   target
}
