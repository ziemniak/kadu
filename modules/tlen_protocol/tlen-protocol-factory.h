/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TLEN_PROTOCOL_FACTORY_H
#define TLEN_PROTOCOL_FACTORY_H

#include "protocols/protocol-factory.h"
#include "misc/misc.h"

class TlenProtocolFactory : public ProtocolFactory
{
	Q_DISABLE_COPY(TlenProtocolFactory)

	static TlenProtocolFactory *Instance;
	QList<StatusType *> SupportedStatusTypes;

	TlenProtocolFactory();

public:
	static TlenProtocolFactory * instance();

	virtual Account * newAccount();
	virtual Account * loadAccount(StoragePoint *accountStoragePoint);
	virtual ContactAccountData * newContactAccountData(Contact contact, Account *account, const QString &id, bool isAnonymous = false);
	virtual ContactAccountData * loadContactAccountData(Contact contact, Account *account);
	virtual AccountCreateWidget * newCreateAccountWidget(QWidget *parent);
	virtual AccountEditWidget* newEditAccountWidget(Account*, QWidget*);
	virtual ContactAccountDataWidget * newContactAccountDataWidget(ContactAccountData *contactAccountData, QWidget *parent = 0);
	//virtual ProtocolMenuManager * getProtocolMenuManager() { return 0; }
	virtual QList<StatusType *> supportedStatusTypes();

	virtual QString name() { return "tlen"; }
	virtual QString displayName() { return "Tlen"; }
	virtual QString iconName() {return dataPath("kadu/modules/data/tlen_protocol/").append("online.png");}
};

#endif // TLEN_PROTOCOL_FACTORY_H
