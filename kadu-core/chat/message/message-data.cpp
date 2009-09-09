/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "message-data.h"

MessageData::MessageData(Chat *chat, Message::Type type, Contact sender) :
		QObject(), MyChat(chat), Sender(sender), MyStatus(Message::StatusUnknown), MyType(type)
{
}

MessageData::~MessageData()
{
}

MessageData & MessageData::setChat(Chat *chat)
{
	MyChat = chat;
	return *this;
}

MessageData & MessageData::setSender(Contact sender)
{
	Sender = sender;
	return *this;
}

MessageData & MessageData::setContent(const QString &content)
{
	Content = content;
	return *this;
}

MessageData & MessageData::setReceiveDate(QDateTime receiveDate)
{
	ReceiveDate = receiveDate;
	return *this;
}

MessageData & MessageData::setSendDate(QDateTime sendDate)
{
	SendDate = sendDate;
	return *this;
}

MessageData & MessageData::setStatus(Message::Status status)
{
	if (status != MyStatus)
	{
		MyStatus = status;
		emit statusChanged(MyStatus);
	}

	return *this;
}


MessageData& MessageData::setType(Message::Type type)
{
	MyType = type;
	return *this;
}
