#include "Settings.hpp"
#define PWMCTRL_CONFIG_OPTION(id, default) const QString Settings::K_##id = #id;
#include "./settings_config.inc"
#undef PWMCTRL_CONFIG_OPTION

#if QT_VERSION >= 0x050000
#include <QApplication>
#else
#include <QtGui/QApplication>
#endif

Settings::Settings(QObject *parent) :
	QObject(parent),
	m_settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat)
{
	m_settings.setFallbacksEnabled(false);
	m_settings.sync();
	//#define PWMCTRL_CONFIG_OPTION(id, def) this->m_settings.setValue(K_##id, def);
	//#include "./settings_config.inc"
	//#undef PWMCTRL_CONFIG_OPTION
}
