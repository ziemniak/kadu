/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBER_ACCOUNT
#define JABBER_ACCOUNT

#include <QtCore/QString>

#include "accounts/account.h"

class JabberAccount : public Account
{
	QString Jid;
	QString Resource;
	int Priority;

public:
	static JabberAccount * loadFromStorage(StoragePoint *storagePoint);

	explicit JabberAccount(const QUuid &uuid = QUuid());

	// TODO - jid is always null - why?
	// QString jid() { return Jid; }

	QString resource() { return Resource; }
	void setResource(const QString &resource) { Resource = resource; }

	int priority() { return Priority; }
	void setPriority(const int &priority) { Priority = priority; }

	virtual bool setId(const QString &id);

	virtual void load();
	virtual void store();
};

#endif // JABBER_ACCOUNT