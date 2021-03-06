 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ABSTRACT_CONTACTS_MODEL
#define ABSTRACT_CONTACTS_MODEL

class QModelIndex;

class Contact;

class AbstractContactsModel
{

public:
	virtual Contact contact(const QModelIndex &index) const;
	virtual const QModelIndex contactIndex(Contact contact) const = 0;

};

#endif // ABSTRACT_CONTACTS_MODEL
