/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACT_DATA_MANAGER_H
#define CONTACT_DATA_MANAGER_H

#include "configuration/configuration-window-data-manager.h"
#include "contacts/contact.h"

class ContactDataManager : public ConfigurationWindowDataManager
{
	Q_OBJECT

	Contact CurrentContact;

public:
	ContactDataManager(Contact contact, QObject *parent = 0);

	virtual void writeEntry(const QString &section, const QString &name, const QVariant &value);
	virtual QVariant readEntry(const QString &section, const QString &name);

};

#endif // CONTACT_DATA_MANAGER_H
