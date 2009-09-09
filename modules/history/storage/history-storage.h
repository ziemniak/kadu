/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef HISTORY_STORAGE_H
#define HISTORY_STORAGE_H

#include <QtCore/QDate>
#include <QtCore/QObject>

#include "chat/chat.h"
#include "contacts/contact.h"

#include "../history_exports.h"

class Message;

class HISTORYAPI HistoryStorage : public QObject
{
	Q_OBJECT

private slots:
	virtual void messageReceived(const Message &message) = 0;
	virtual void messageSent(const Message &message) = 0;

public:
	virtual QList<Chat *> chats() = 0;
	virtual QList<QDate> chatDates(Chat *chat) = 0;
	virtual QList<Message> messages(Chat *chat, QDate date = QDate(), int limit = 0) = 0;
	virtual QList<Message> messagesSince(Chat *chat, QDate date) = 0;
	virtual int messagesCount(Chat *chat, QDate date = QDate()) = 0;

	virtual void appendMessage(const Message &message) = 0;

	virtual void clearChatHistory(Chat *chat) = 0;

};

#endif // HISTORY_STORAGE_H
