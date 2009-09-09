/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"
#include "accounts/account-manager.h"
#include "chat/message/message.h"
#include "chat/message/message-render-info.h"
#include "chat/message/pending-messages-manager.h"
#include "configuration/configuration-file.h"
#include "configuration/xml-configuration-file.h"
#include "contacts/contact-account-data.h"
#include "contacts/contact-manager.h"
#include "contacts/ignored-helper.h"
#include "core/core.h"
#include "gui/actions/action.h"
#include "gui/actions/actions.h"
#include "gui/widgets/chat-edit-box.h"
#include "gui/widgets/chat-widget-actions.h"
#include "gui/widgets/contacts-list-widget-menu-manager.h"
#include "gui/widgets/custom-input.h"
#include "gui/windows/chat-window.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/kadu-window-actions.h"
#include "gui/windows/message-box.h"
#include "gui/windows/open-chat-with/open-chat-with.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocols-manager.h"

#include "activate.h"
#include "debug.h"
#include "icons-manager.h"
#include "misc/misc.h"
#include "search.h"

#include "chat-widget-manager.h"

ChatWidgetManager * ChatWidgetManager::Instance = 0;

ChatWidgetManager * ChatWidgetManager::instance()
{
	if (0 == Instance)
		Instance = new ChatWidgetManager();

	return Instance;
}

ChatWidgetManager::ChatWidgetManager()
{
	kdebugf();

	MessageRenderInfo::registerParserTags();
	EmoticonsManager::instance()->setEmoticonsTheme(config_file.readEntry("Chat", "EmoticonsTheme"));

	connect(Core::instance(), SIGNAL(messageReceived(const Message &)),
			this, SLOT(messageReceived(const Message &)));
	connect(Core::instance(), SIGNAL(messageSent(const Message &)),
			this, SLOT(messageSent(const Message &)));

	Actions = new ChatWidgetActions(this);

	QTimer *ClosedChatsTimer = new QTimer(this);
	connect(ClosedChatsTimer, SIGNAL(timeout()), this, SLOT(clearClosedChats()));
	ClosedChatsTimer->start(30*1000);

	configurationUpdated();

	kdebugf2();
}

void ChatWidgetManager::closeAllWindows()
{
	kdebugf();

	if (config_file.readBoolEntry("Chat", "SaveOpenedWindows", true))
		saveOpenedWindows();

	foreach (ChatWidget *chat, Chats)
	{
		ChatWindow *window = dynamic_cast<ChatWindow *>(chat->parent());
		if (window)
			delete window;
	}

	Chats.clear();

	kdebugf2();
}

void ChatWidgetManager::loadOpenedWindows()
{
	kdebugf();
	QDomElement root_elem = xml_config_file->rootElement();
	QDomElement chats_elem = xml_config_file->findElement(root_elem, "ChatWindows");
	if (!chats_elem.isNull())
	{
		for (QDomNode win = chats_elem.firstChild(); !win.isNull(); win = win.nextSibling())
		{
			const QDomElement &window_elem = win.toElement();
			if (window_elem.isNull())
				continue;
			if (window_elem.tagName() != "Window")
				continue;
			QDomElement protocolNode = xml_config_file->getNode(window_elem, "Protocol", XmlConfigFile::ModeFind);
			QString protocolId = protocolNode.text();
			QDomElement windowIdNode = xml_config_file->getNode(window_elem, "WindowId", XmlConfigFile::ModeFind);
			QString accountId = windowIdNode.text();

			ContactSet contacts;
			for (QDomNode contact = window_elem.firstChild(); !contact.isNull(); contact = contact.nextSibling())
			{
				const QDomElement &contact_elem = contact.toElement();
				if (contact_elem.isNull())
					continue;
				if (contact_elem.tagName() != "Contact")
					continue;
				QString uuid = contact_elem.text();
				contacts.insert(ContactManager::instance()->byUuid(uuid));
			}

			// TODO 0.6.6: fix
			Account *defaultAccount = AccountManager::instance()->defaultAccount();
			if (defaultAccount)
			{
				Chat *chat = defaultAccount->protocol()->findChat(contacts);
				if (chat)
					openChatWidget(chat, true);
			}
			else
				kdebugm(KDEBUG_WARNING, "protocol %s/%s not found!\n",
					       qPrintable(protocolId), qPrintable(accountId));
		}
	}
	kdebugf2();
}

void ChatWidgetManager::saveOpenedWindows()
{
	// TODO: 0.6.6 saveOpenedChats?

/*
	kdebugf();
	QDomElement root_elem = xml_config_file->rootElement();
	QDomElement chats_elem = xml_config_file->accessElement(root_elem, "ChatWindows");
	xml_config_file->removeChildren(chats_elem);
	foreach (ChatWidget *chat, ChatWidgets)
	{
		if (!chat->currentProtocol() || !chat->currentProtocol()->protocolFactory())
			continue;

		QDomElement windowNode = xml_config_file->getNode(chats_elem,
			"Window", XmlConfigFile::ModeCreate);
		// TODO 0.6.6 - gadu raus!

		xml_config_file->createTextNode(windowNode, "Protocol", chat->currentProtocol()->protocolFactory()->displayName());
		xml_config_file->createTextNode(windowNode, "WindowId", "Gadu");

		foreach(Contact contact, chat->contacts())
			xml_config_file->createTextNode(windowNode, "Contact", contact.uuid());
	}
	kdebugf2();*/
}

ChatWidgetManager::~ChatWidgetManager()
{
	kdebugf();

	MessageRenderInfo::unregisterParserTags();

	disconnect(Core::instance(), SIGNAL(messageReceived(const Message &)),
			this, SLOT(messageReceived(const Message &)));

	closeAllWindows();

	kdebugf2();
}

void ChatWidgetManager::openChatWith()
{
	(new OpenChatWith(Core::instance()->kaduWindow()))->show();
}

void ChatWidgetManager::insertEmoticonActionEnabled()
{
 	foreach (Action *action, Actions->insertEmoticon()->actions())
	{
		if ((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle") == EMOTS_NONE)
		{
			action->setToolTip(tr("Insert emoticon - enable in configuration"));
			action->setEnabled(false);
		}
		else
		{
			action->setToolTip(tr("Insert emoticon"));
			action->setEnabled(true);
		}
	}
}

const QHash<Chat *, ChatWidget *> & ChatWidgetManager::chats() const
{
	return Chats;
}

const QList<Chat *> ChatWidgetManager::closedChats() const
{
	return ClosedChats;
}

void ChatWidgetManager::registerChatWidget(ChatWidget *chat)
{
	kdebugf();
	int index = ClosedChats.indexOf(chat->chat());
	ClosedChats.removeAt(index);
	ClosedChatsDates.removeAt(index);
	Chats.insert(chat->chat(), chat);
}

void ChatWidgetManager::unregisterChatWidget(ChatWidget *chat)
{
	kdebugf();

	if (!Chats.contains(chat->chat()))
		return;

	if (chat->countMessages())
	{
		ClosedChats.prepend(chat->chat());
		ClosedChatsDates.prepend(QDateTime::currentDateTime());
	}

	emit chatWidgetDestroying(chat);
	Chats.remove(chat->chat());
}

ChatWidget * ChatWidgetManager::byChat(Chat *chat, bool create) const
{
	return Chats.contains(chat)
		? Chats[chat]
		: 0;
}

void ChatWidgetManager::activateChatWidget(ChatWidget *chatWidget, bool forceActivate)
{
	QWidget *win = chatWidget->window();
	kdebugm(KDEBUG_INFO, "parent: %p\n", win);
	chatWidget->makeActive();
	emit chatWidgetOpen(chatWidget);
}

ChatWidget * ChatWidgetManager::openChatWidget(Chat *chat, bool forceActivate)
{
	kdebugf();

	ChatWidget *chatWidget = byChat(chat);
	if (chatWidget)
	{
		activateChatWidget(chatWidget, forceActivate);
		return chatWidget;
	}

	chatWidget = new ChatWidget(chat);

	bool handled = false;
	emit handleNewChatWidget(chatWidget, handled);
	if (!handled)
	{
		ChatWindow *window = new ChatWindow();
		chatWidget->setParent(window);
		chatWidget->show();
		window->setChatWidget(chatWidget);
		window->show();
	}

	connect(chatWidget, SIGNAL(messageSentAndConfirmed(Chat *, const QString &)),
		this, SIGNAL(messageSentAndConfirmed(Chat *, const QString &)));

	if (forceActivate) //TODO 0.6.6:
	{
		chatWidget->makeActive();
	}

	emit chatWidgetCreated(chatWidget);
// TODO: remove, it is so stupid ...
	emit chatWidgetCreated(chatWidget, time(0));
	emit chatWidgetOpen(chatWidget);

	kdebugf2();

	return chatWidget;
}

void ChatWidgetManager::deletePendingMsgs(Chat *chat)
{
	kdebugf();
	for (int i = 0; i < PendingMessagesManager::instance()->count(); ++i)
		if ((*PendingMessagesManager::instance())[i].chat() == chat)
		{
			PendingMessagesManager::instance()->deleteMsg(i);
			--i;
		}

// TODO: 0.6.6
// 	UserBox::refreshAllLater();
	kdebugf2();
}

void ChatWidgetManager::openPendingMsgs(Chat *chat, bool forceActivate)
{
	kdebugf();

	QList<MessageRenderInfo *> messages;
	Message msg;

	ChatWidget *chatWidget = openChatWidget(chat, forceActivate);
	if (!chatWidget)
		return;

	for (int i = 0; i < PendingMessagesManager::instance()->count(); ++i)
	{
		msg = (*PendingMessagesManager::instance())[i];
		if (msg.chat() != chat)
			continue;
		messages.append(new MessageRenderInfo(msg));
		PendingMessagesManager::instance()->deleteMsg(i--);
	}

	if (messages.size())
	{
		// TODO: Lame API
		if (!chatWidget->countMessages())
			chatWidget->appendMessages(messages, true);
// TODO: 0.6.6
// 		UserBox::refreshAllLater();
	}

	kdebugf2();
}

void ChatWidgetManager::openPendingMsgs(bool forceActivate)
{
	kdebugf();

	if (PendingMessagesManager::instance()->count())
		openPendingMsgs((*PendingMessagesManager::instance())[0].chat(), forceActivate);

	kdebugf2();
}

void ChatWidgetManager::sendMessage(Chat *chat)
{
	kdebugf();

	for (int i = 0; i < PendingMessagesManager::instance()->count(); ++i)
		if ((*PendingMessagesManager::instance())[i].chat() == chat)
		{
			openPendingMsgs((*PendingMessagesManager::instance())[i].chat());
			return;
		}

	if (chat)
		openChatWidget(chat, true);

	kdebugf2();
}

void ChatWidgetManager::configurationUpdated()
{
	kdebugf();

	insertEmoticonActionEnabled();

	kdebugf2();
}

void ChatWidgetManager::messageReceived(const Message &message)
{
	kdebugf();

	Chat *chat = message.chat();
	ChatWidget *chatWidget = byChat(chat);
	if (chatWidget)
	{
		MessageRenderInfo *messageRenderInfo = new MessageRenderInfo(message);
		chatWidget->newMessage(messageRenderInfo);
	}
	else
	{
		if (config_file.readBoolEntry("General","AutoRaise"))
		{
			Core::instance()->kaduWindow()->showNormal();
			Core::instance()->kaduWindow()->setFocus();
		}

		if (config_file.readBoolEntry("Chat", "OpenChatOnMessage"))
		{
			// TODO: 0.6.6
			if (config_file.readBoolEntry("Chat", "OpenChatOnMessageWhenOnline") && false /*!Myself.status("Gadu").isOnline()*/)
			{
				PendingMessagesManager::instance()->addMsg(message);
				return;
			}

			// TODO: it is lame
			openChatWidget(chat);
			chatWidget = byChat(chat);

			MessageRenderInfo *messageRenderInfo = new MessageRenderInfo(message);
			chatWidget->newMessage(messageRenderInfo);
		}
		else
			PendingMessagesManager::instance()->addMsg(message);
	}

	kdebugf2();
}

void ChatWidgetManager::messageSent(const Message &message)
{
	Chat *chat = message.chat();
	ChatWidget *chatWidget = byChat(chat);
	MessageRenderInfo *messageRenderInfo = new MessageRenderInfo(message);

	if (!chatWidget)
	{
		openChatWidget(chat);
		chatWidget = byChat(chat);
	}

	if (!chatWidget)
		return;

	chatWidget->appendMessage(messageRenderInfo);
}

void ChatWidgetManager::clearClosedChats()
{
	QDateTime now = QDateTime::currentDateTime();
	int i = 0;
	while (i < ClosedChatsDates.size())
	{
		if (ClosedChatsDates.at(i).addSecs(config_file.readNumEntry("Chat", "RecentChatsTimeout")*60) < now)
		{
			ClosedChats.removeAt(i);
			ClosedChatsDates.removeAt(i);
		}
		else
		{
			i++;
		}
	}
}
