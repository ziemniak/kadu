/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtNetwork/QHttp>
#include <QtXml/QDomDocument>

#include "accounts/account.h"
#include "contacts/avatar-manager.h"
#include "misc/path-conversion.h"

#include "client/jabber-client.h"
#include "jabber-protocol.h"
#include "utils/vcard-factory.h"

#include "jabber-avatar-fetcher.h"

JabberAvatarFetcher::JabberAvatarFetcher(ContactAccountData *contactAccountData, QObject *parent) :
		QObject(parent), MyContactAccountData(contactAccountData)
{
}


void JabberAvatarFetcher::fetchAvatar()
{
	JabberProtocol *jabberProtocol = dynamic_cast<JabberProtocol *>(MyContactAccountData->account()->protocol());
	if (!jabberProtocol || !jabberProtocol->isConnected())
		return;
	VCardFactory::instance()->getVCard(MyContactAccountData->id(), jabberProtocol->client()->rootTask(), this, SLOT(receivedVCard()));
}

void JabberAvatarFetcher::receivedVCard()
{
	const VCard* vcard = VCardFactory::instance()->vcard(MyContactAccountData->id());
	if (vcard && !vcard->photo().isEmpty()) 
	{
		MyContactAccountData->avatar().setNextUpdate(QDateTime::fromTime_t(QDateTime::currentDateTime().toTime_t() + 7200));
		emit avatarFetched(MyContactAccountData, vcard->photo());
	}
	deleteLater();
}
