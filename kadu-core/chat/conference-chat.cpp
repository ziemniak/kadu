/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "contacts/contact-list-configuration-helper.h"

#include "conference-chat.h"

ConferenceChat::ConferenceChat(Account *currentAccount, ContactList contacts, QUuid uuid)
	: Chat(currentAccount, uuid), CurrentContacts(contacts)
{
}

ConferenceChat::~ConferenceChat()
{
}

void ConferenceChat::load()
{
	if (!isValidStorage())
		return;

	XmlConfigFile *st = storage()->storage();

	Chat::load();
	CurrentContacts = ContactListConfigurationHelper::loadFromConfiguration(st, st->getNode(storage()->point(), "Contacts"));
}

void ConferenceChat::store()
{
	if (!isValidStorage())
		return;

	XmlConfigFile *st = storage()->storage();

	Chat::store();
	ContactListConfigurationHelper::saveToConfiguration(st, st->getNode(storage()->point(), "Contacts"), CurrentContacts);
}
