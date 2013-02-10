#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QSharedPointer>
#include <QVariant>
#include <type_traits>


class Settings : public QObject
{
	Q_OBJECT
	QSettings m_settings;

	template<typename T> struct transform_setting_type {
		typedef typename std::remove_reference<T>::type noref;
		typedef typename std::conditional<std::is_array<noref>::value,
		QString,
		T>::type type;

	};

public:
	typedef QSharedPointer<Settings> sptr;
	explicit Settings(QObject *parent = 0);

#define PWMCTRL_CONFIG_OPTION(id, def) \
	static const QString K_##id;/*key name*/\
	auto get##id() -> transform_setting_type<decltype(def)>::type{ \
	auto stored = m_settings.value(K_##id);\
	if (!stored.isValid()) {\
	auto v = def;\
	m_settings.setValue(K_##id, v);\
	return v;\
} else {\
	return  qvariant_cast<transform_setting_type<decltype(def)>::type>(stored);\
}}
#include "./settings_config.inc"
#undef PWMCTRL_CONFIG_OPTION

signals:

public slots:

};

#endif // SETTINGS_H
