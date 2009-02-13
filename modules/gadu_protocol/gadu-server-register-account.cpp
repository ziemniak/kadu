/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "misc.h"

#include "gadu-pubdir-socket-notifiers.h"

#include "gadu-server-register-account.h"

GaduServerRegisterAccount::GaduServerRegisterAccount(TokenReader *reader, const QString &mail, const QString &password)
	: GaduServerConnector(reader), Uin(0), Mail(mail), Password(password)
{
}

void GaduServerRegisterAccount::performAction(const QString &tokenId, const QString &tokenValue)
{
	struct gg_http *h = gg_register3(unicode2cp(Mail).data(), unicode2cp(Password).data(),
			unicode2cp(tokenId).data(), unicode2cp(tokenValue).data(), 1);
	if (h)
	{
		GaduPubdirSocketNotifiers *sn = new GaduPubdirSocketNotifiers(h);
		connect(sn, SIGNAL(done(bool, struct gg_http *)), this, SLOT(done(bool, struct gg_http *)));
		sn->start();
	}
	else
		finished(false);
}

void GaduServerRegisterAccount::done(bool ok, struct gg_http *h)
{
	if (ok)
		Uin = ((struct gg_pubdir *)h->data)->uin;

	finished(ok);
}