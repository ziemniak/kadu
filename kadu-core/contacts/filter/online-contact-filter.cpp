/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"

#include "contacts/contact-account-data.h"

#include "online-contact-filter.h"

OnlineContactFilter::OnlineContactFilter(QObject *parent)
	: AbstractContactFilter(parent), Enabled(false)
{
}

void OnlineContactFilter::setEnabled(bool enabled)
{
	if (enabled == Enabled)
		return;

	Enabled = enabled;
	emit filterChanged();
}

bool OnlineContactFilter::acceptContact(Contact contact)
{
	if (!Enabled)
		return true;

	Account *prefferedAccount = contact.prefferedAccount();
	if (!prefferedAccount)
		return false;

	Status status = contact.accountData(prefferedAccount)->status();
	return !status.isDisconnected();
}


/*
	if (config_file.readBoolEntry("General", "ShowBlocking") != showBlocking)
		changeDisplayingBlocking(!showBlocking);
	if (config_file.readBoolEntry("General", "ShowBlocked") != showBlocked)
		changeDisplayingBlocked(!showBlocked);*/
// 	if (config_file.readBoolEntry("General", "ShowOffline") != showOffline)
// 		changeDisplayingOffline(kadu->userbox(), !showOffline);
//  	if (config_file.readBoolEntry("General", "ShowWithoutDescription") != showWithoutDescription)
//  		changeDisplayingWithoutDescription(kadu->userbox(), !showWithoutDescription);

// BlockedUsers::BlockedUsers() : UserGroup()
// 		if (user.usesProtocol("Gadu") && user.protocolData("Gadu", "Blocking").toBool())
// 			addUser(user);

