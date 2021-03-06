/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef TLEN_AVATAR_FETCHER_H
#define TLEN_AVATAR_FETCHER_H

#include <QtCore/QBuffer>
#include <QtGui/QPixmap>

#include "contacts/contact-account-data.h"

class QHttp;

class TlenAvatarFetcher : public QObject
{
	Q_OBJECT

	ContactAccountData *MyContactAccountData;
	QBuffer MyAvatarBuffer;
	QHttp *MyHttp;

private slots:
	void avatarDownloaded(int id, bool error);

public:
	TlenAvatarFetcher(ContactAccountData *contactAccountData, QObject *parent = 0);
	void fetchAvatar();

signals:
	void avatarFetched(ContactAccountData *contactAccountData, const QByteArray &avatar);

};

#endif // TLEN_AVATAR_FETCHER_H
