/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QMimeData>
#include <QtCore/QStringList>

#include "contact.h"
#include "contact-list.h"
#include "contact-list-mime-data-helper.h"
#include "contact-manager.h"

QLatin1String ContactListMimeDataHelper::MimeType("application/x-kadu-ules");

QStringList ContactListMimeDataHelper::mimeTypes()
{
	QStringList result;
	result << MimeType;
	return result;
}

QMimeData * ContactListMimeDataHelper::toMimeData(ContactList contactList)
{
	if (!contactList.count())
		return 0;

	QMimeData *mimeData = new QMimeData();

	QStringList contactListStrings;
	foreach (Contact contact, contactList)
		contactListStrings << contact.uuid().toString();

	mimeData->setData(MimeType, contactListStrings.join(":").toAscii());
	return mimeData;
}

ContactList ContactListMimeDataHelper::fromMimeData(const QMimeData * mimeData)
{
	ContactList result;

	QString contactListString(mimeData->data(MimeType));
	if (contactListString.isEmpty())
		return result;

	QStringList contactListStrings = contactListString.split(":");
	foreach (const QString &contactListString, contactListStrings)
	{
		Contact contact = ContactManager::instance()->byUuid(contactListString);
		if (contact.isNull())
			continue;

		result << contact;
	}

	return result;
}
