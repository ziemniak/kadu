/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef JABBER_AVATAR_FETCHER_H
#define JABBER_AVATAR_FETCHER_H

#include <QtCore/QBuffer>
#include <QtGui/QPixmap>

#include "contacts/contact-account-data.h"

class QHttp;

class JabberAvatarFetcher : public QObject
{
	Q_OBJECT

	ContactAccountData *MyContactAccountData;
	
private slots:
	void receivedVCard();

public:
	JabberAvatarFetcher(ContactAccountData *contactAccountData, QObject *parent = 0);
	void fetchAvatar();

signals:
	void avatarFetched(ContactAccountData *contactAccountData, const QByteArray &avatar);

};

#endif // JABBER_AVATAR_FETCHER_H
