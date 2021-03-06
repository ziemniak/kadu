/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFERENCE_CHAT_H
#define CONFERENCE_CHAT_H

#include <QtCore/QObject>
#include <QtCore/QUuid>

#include "accounts/account.h"
#include "chat/chat.h"
#include "configuration/storable-object.h"
#include "contacts/contact-list.h"

class XmlConfigFile;

class ConferenceChat : public Chat
{
	ContactSet CurrentContacts;

public:
	ConferenceChat(StoragePoint *storage);
	ConferenceChat(Account *parentAccount, ContactSet contacts, QUuid uuid = QUuid());
	virtual ~ConferenceChat();

	virtual void load();
	virtual void store();

	virtual ChatType type() const;
	virtual ContactSet contacts() const { return CurrentContacts; }
	virtual QString name() const;

};

#endif // CONFERENCE_CHAT_H
