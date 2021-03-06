/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QApplication>

#include "accounts/account.h"
#include "accounts/account_manager.h"
#include "chat_widget.h"
#include "chat_manager-old.h"
#include "debug.h"
#include "kadu.h"
#include "kadu_parser.h"

#include "autoresponder.h"

/**
 * @ingroup autoresponder
 * @{
 */
extern "C" KADU_EXPORT int autoresponder_init(bool firstLoad)
{
	autoresponder = new AutoResponder();
	MainConfigurationWindow::registerUiFile(dataPath("kadu/modules/configuration/autoresponder.ui"), autoresponder);
	return 0;
}

extern "C" KADU_EXPORT void autoresponder_close()
{
	MainConfigurationWindow::unregisterUiFile(dataPath("kadu/modules/configuration/autoresponder.ui"), autoresponder);
	delete autoresponder;
	autoresponder = 0;
}

AutoResponder::AutoResponder(QObject *parent, const char *name) : QObject(parent, name), repliedUsers()
{
	kdebugf();

	Protocol *gadu = AccountManager::instance()->defaultAccount()->protocol();
	connect(gadu, SIGNAL(messageReceived(Protocol *, UserListElements, const QString &, time_t )),
		this, SLOT(messageReceived(Protocol *, UserListElements, const QString&, time_t)));
	connect(chat_manager, SIGNAL(chatWidgetOpen(ChatWidget *)), this, SLOT(chatOpenedClosed(ChatWidget *)));
	connect(chat_manager, SIGNAL(chatWidgetDestroying(ChatWidget *)), this, SLOT(chatOpenedClosed(ChatWidget *)));

	createDefaultConfiguration();
	configurationUpdated();

	kdebugf2();
}

AutoResponder::~AutoResponder()
{
	kdebugf();

	Protocol *gadu = AccountManager::instance()->defaultAccount()->protocol();
	disconnect(gadu, SIGNAL(messageReceived(Protocol *, UserListElements, const QString&, time_t)),
		this, SLOT(messageReceived(Protocol *, UserListElements, const QString&, time_t)));
	disconnect(chat_manager, SIGNAL(chatWidgetOpen(ChatWidget *)), this, SLOT(chatOpenedClosed(ChatWidget *)));
	disconnect(chat_manager, SIGNAL(chatWidgetDestroying(ChatWidget *)), this, SLOT(chatOpenedClosed(ChatWidget *)));

	kdebugf2();
}

void AutoResponder::messageReceived(Protocol *protocol, UserListElements senders, const QString& msg, time_t /*time*/)
{
	kdebugf();
	if (msg.left(5) == "KADU ")
	{
		kdebugf2();
		return;
	}

	if (!respondConferences && (senders.count() > 1))
	{
		kdebugf2();
		return;
	}

	if (respondOnlyFirst && repliedUsers.contains(senders))
	{
		kdebugf2();
		return;
	}

	if ((statusAvailable && protocol->currentStatus().isOnline()) || (statusBusy && protocol->currentStatus().isBusy()) || (statusInvisible && protocol->currentStatus().isInvisible()))
	{
		protocol->sendMessage(senders, tr("KADU AUTORESPONDER:") + "\n" + KaduParser::parse(autotext, senders[0]));
		repliedUsers.append(senders); // dolaczamy uzytkownikow, ktorym odpowiedziano
	}

	kdebugf2();
}

void AutoResponder::chatOpenedClosed(ChatWidget *chat)
{
	repliedUsers.remove(chat->users()->toUserListElements());
}

void AutoResponder::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow) 
{
	mainConfigurationWindow->widgetById("autoresponder/AutoText")->setToolTip(qApp->translate("@default", Kadu::SyntaxText));
}

void AutoResponder::configurationUpdated()
{
	kdebugf();

	autotext = config_file.readEntry("Autoresponder", "Autotext");

	respondConferences = config_file.readBoolEntry("Autoresponder", "RespondConf");
	respondOnlyFirst = config_file.readBoolEntry("Autoresponder", "OnlyFirstTime");

	statusAvailable = config_file.readBoolEntry("Autoresponder", "StatusAvailable");
	statusBusy = config_file.readBoolEntry("Autoresponder", "StatusBusy");
	statusInvisible = config_file.readBoolEntry("Autoresponder", "StatusInvisible");

	kdebugf2();
}

void AutoResponder::createDefaultConfiguration()
{
	config_file.addVariable("Autoresponder", "Autotext", tr("I am busy."));
	config_file.addVariable("Autoresponder", "OnlyFirstTime", true);
	config_file.addVariable("Autoresponder", "RespondConf", true);
	config_file.addVariable("Autoresponder", "StatusAvailable", false);
	config_file.addVariable("Autoresponder", "StatusBusy", true);
	config_file.addVariable("Autoresponder", "StatusInvisible", false);
}

AutoResponder* autoresponder;

/** @} */
