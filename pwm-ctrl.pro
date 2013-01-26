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
                main_window.cpp

HEADERS +=      main_window.h

FORMS +=        main_window.ui


unix {
  target.path = $$[INSTALL_ROOT]/bin
  INSTALLS +=   target
}

