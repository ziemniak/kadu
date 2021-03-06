 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACTS_MODEL
#define CONTACTS_MODEL

#include <QtCore/QAbstractListModel>
#include <QtCore/QModelIndex>

#include "accounts/account.h"
#include "accounts/accounts-aware-object.h"
#include "contacts/contact.h"
#include "status/status.h"

#include "contacts-model-base.h"

class ContactManager;

class ContactsModel : public ContactsModelBase
{
	Q_OBJECT

	ContactManager *Manager;

private slots:
	void contactAboutToBeAdded(Contact &contact);
	void contactAdded(Contact &contact);
	void contactAboutToBeRemoved(Contact &contact);
	void contactRemoved(Contact &contact);

public:
	explicit ContactsModel(ContactManager *manager, QObject *parent = 0);
	~ContactsModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	
	// AbstractContactsModel implementation
	virtual Contact contact(const QModelIndex &index) const;
	virtual const QModelIndex contactIndex(Contact contact) const;

};

#endif // CONTACTS_MODEL
