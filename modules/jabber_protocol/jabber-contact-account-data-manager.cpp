/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabber-contact-account-data.h"

#include "jabber-contact-account-data-manager.h"

JabberContactAccountDataManager::JabberContactAccountDataManager(JabberContactAccountData* data, QObject *parent)
	: ContactAccountDataManager(data, parent), Data(data)
{
}

void JabberContactAccountDataManager::writeEntry(const QString &section, const QString &name, const QVariant &value)
{
	if (section != "Jabber")
	{
		ContactAccountDataManager::writeEntry(section, name, value);
		return;
	}

	// TODO : setId

	// other data
}

QVariant JabberContactAccountDataManager::readEntry(const QString &section, const QString &name)
{
	if (section != "Jabber")
		return ContactAccountDataManager::readEntry(section, name);

	if (name == "Jid")
		return Data->id();

	// other data

	return QVariant(QString::null);
}
