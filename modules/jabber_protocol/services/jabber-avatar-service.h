/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef JABBER_AVATAR_SERVICE_H
#define JABBER_AVATAR_SERVICE_H

#include "contacts/contact-account-data.h"
#include "protocols/services/avatar-service.h"

class JabberAvatarService : public AvatarService
{
	Q_OBJECT
	ContactAccountData *MyContactAccountData;

public:
	JabberAvatarService(QObject *parent = 0) : AvatarService(parent) {}
	void fetchAvatar(ContactAccountData *contactAccountData);

};

#endif // JABBER_AVATAR_SERVICE_H
