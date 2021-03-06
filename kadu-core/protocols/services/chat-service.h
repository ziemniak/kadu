/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHAT_SERVICE_H
#define CHAT_SERVICE_H

#include <QtCore/QObject>

#include "contacts/contact-list.h"

#include "chat/chat.h"
#include "chat/message/formatted-message.h"

#include "exports.h"

class Message;

class KADUAPI ChatService : public QObject
{
	Q_OBJECT

public:
	enum MessageStatus {
		StatusAcceptedDelivered,
		StatusAcceptedQueued,
		StatusRejectedBlocked,
		StatusRejectedBoxFull,
		StatusRejectedUnknown
	};

	ChatService(QObject *parent = 0)
		: QObject(parent) {}

public slots:
	virtual bool sendMessage(Chat *chat, const QString &messageContent);
	virtual bool sendMessage(Chat *chat, FormattedMessage &message) = 0;

signals:
	void sendMessageFiltering(Chat *chat, QByteArray &msg, bool &stop);
	void messageStatusChanged(int messsageId, ChatService::MessageStatus status);
	void receivedMessageFilter(Chat *chat, Contact sender, const QString &message, time_t time, bool &ignore);
	void messageSent(const Message &message);
	void messageReceived(const Message &message);

};

#endif // CHAT_SERVICE_H
