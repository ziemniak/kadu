#ifndef QT4_DOCKING_NOTIFY_H
#define QT4_DOCKING_NOTIFY_H

#include "gui/windows/main-configuration-window.h"
#include "notify/notifier.h"
#include "../qt4_docking/qt4_docking.h"

#include "qt4_docking_notify_configuration_widget.h"

class Chat;

/**
 * @defgroup qt4_notify Qt4 Notify
 * @{
 */
class Qt4Notify : public Notifier
{
	Q_OBJECT

	void import_0_6_5_configuration();
	void createDefaultConfiguration();
	QString toPlainText(const QString &text);
	QString parseText(const QString &text, Notification *notification, const QString &def);
	Chat *chat;
	Qt4NotifyConfigurationWidget *configurationWidget;

public:
	Qt4Notify();
	~Qt4Notify();

	virtual void notify(Notification *notification);

	virtual CallbackCapacity callbackCapacity() { return CallbackNotSupported; }

	void copyConfiguration(const QString &fromEvent, const QString &toEvent) {}

	virtual NotifierConfigurationWidget *createConfigurationWidget(QWidget *parent = 0);

public slots:
	void messageClicked();
};

extern Qt4Notify *qt4_notify;
extern Qt4TrayIcon* qt4_tray_icon;

/** @} */

#endif
