#include "Common.hpp"
#include "Settings.hpp"
#define PWMCTRL_CONFIG_OPTION(id, default) const QString Settings::K_##id = #id;
#include "./settings_config.inc"
#undef PWMCTRL_CONFIG_OPTION

Settings::Settings(QObject *parent) :
    QObject(parent),
    m_settings(QApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat)
{
    m_settings.setFallbacksEnabled(false);
//#define PWMCTRL_CONFIG_OPTION(id, def) this->m_settings.setValue(K_##id, def);
//#include "./settings_config.inc"
//#undef PWMCTRL_CONFIG_OPTION
}
