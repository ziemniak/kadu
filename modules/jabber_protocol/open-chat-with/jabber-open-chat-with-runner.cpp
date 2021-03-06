/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"

#include "contacts/contact.h"

#include "debug.h"

#include "jabber-contact-account-data.h"
#include "jabber-open-chat-with-runner.h"

JabberOpenChatWithRunner::JabberOpenChatWithRunner(Account *account) : ParentAccount(account)
{
}

ContactList JabberOpenChatWithRunner::matchingContacts(const QString &query)
{
	kdebugf();

	ContactList matchedContacts;
	if (!validateUserID(query))
		return matchedContacts;

	Contact c;

	JabberContactAccountData *gcad = new JabberContactAccountData(c, ParentAccount, query);
	c.addAccountData(gcad);
	c.setDisplay(ParentAccount->name() + ": " + query);
	matchedContacts.append(c);

	return matchedContacts;
}

bool JabberOpenChatWithRunner::validateUserID(const QString &uid)
{
	// TODO validate ID
	//QString text = uid;
	return true;
}
