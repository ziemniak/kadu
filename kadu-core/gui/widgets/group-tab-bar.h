/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GROUP_TAB_BAR_H
#define GROUP_TAB_BAR_H

#include <QtGui/QTabBar>

#include "contacts/contact-list.h"

class GroupContactFilter;
class Group;

class GroupTabBar : public QTabBar
{
	Q_OBJECT

	GroupContactFilter *Filter;
	//for dnd support
	Group *currentGroup;
	ContactList currentContacts;

private slots:
	void currentChangedSlot(int index);
	void groupAdded(Group *group);
	void groupRemoved(Group *group);
	void groupAppearanceChanged(const Group *group);
	void groupNameChanged(const Group *group);

	void renameGroup();
	void deleteGroup();
	void createNewGroup();
	void groupProperties();

 	void addToGroup();
 	void moveToGroup();

protected:
	void contextMenuEvent(QContextMenuEvent *event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

public:
	GroupTabBar(QWidget *parent = 0);
	~GroupTabBar();

	void addGroup(const Group *group);

	GroupContactFilter * filter() { return Filter; }

signals:
	void currentGroupChanged(const Group *group);

};

#endif // GROUP_TAB_BAR_H
