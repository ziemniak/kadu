/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROTOCOL_FACTORY_H
#define PROTOCOL_FACTORY_H

#include <QtCore/QString>
#include <QtGui/QDialog>

class Account;
class AccountCreateWidget;
class AccountEditWidget;
class AccountData;
class ConfigurationWindow;
class Contact;
class ContactAccountData;
class ContactAccountDataWidget;
class Protocol;
class ProtocolMenuManager;
class StatusType;
class StoragePoint;

class ProtocolFactory : public QObject
{
public:
	virtual Account * newAccount() = 0;
	virtual Account * loadAccount(StoragePoint *accountStoragePoint) = 0;
	virtual ContactAccountData * newContactAccountData(Contact contact, Account *account, const QString &id, bool loadFromConfiguration = true) = 0;
	virtual ContactAccountData * loadContactAccountData(Contact contact, Account *account) = 0;
	virtual AccountCreateWidget * newCreateAccountWidget(QWidget *parent = 0) = 0;
	virtual AccountEditWidget * newEditAccountWidget(Account *, QWidget *parent = 0) = 0;
	virtual ContactAccountDataWidget * newContactAccountDataWidget(ContactAccountData *contactAccountData, QWidget *parent = 0) = 0;
	virtual ProtocolMenuManager * protocolMenuManager() { return 0; }
	virtual QList<StatusType *> supportedStatusTypes() = 0;

	virtual QString name() = 0;
	virtual QString displayName() = 0;

	virtual QString iconName()
	{
		return QString::null;
	}

};

#endif // PROTOCOL_FACTORY_H
