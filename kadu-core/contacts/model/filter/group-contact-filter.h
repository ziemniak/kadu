 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GROUP_CONTACT_FILTER
#define GROUP_CONTACT_FILTER

#include "abstract-contact-filter.h"
#include "contacts/group.h"

class GroupContactFilter : public AbstractContactFilter
{
	Q_OBJECT
	Group *CurrentGroup;
public:
	GroupContactFilter(); 
	void setGroup(Group *group);
	virtual bool acceptContact(Contact contact);
};

#endif // GROUP_CONTACT_FILTER
