/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtXml/QDomElement>

#include "configuration/storage-point.h"

#include "xml_config_file.h"

#include "group.h"

#include "group-manager.h"

GroupManager *GroupManager::Instance = 0;

GroupManager * GroupManager::instance()
{
	if (!Instance)
		Instance = new GroupManager();

	return Instance;
}

GroupManager::GroupManager()
{
}

GroupManager::~GroupManager()
{
}

StoragePoint * GroupManager::createStoragePoint() const
{
	QDomElement groupsNode = xml_config_file->getNode("Groups");
	return new StoragePoint(xml_config_file, groupsNode);
}

void GroupManager::importConfiguration()
{
	StoragePoint *sp = storage();
	if (!sp || !sp->storage())
		return;

	QSet<QString> groups;
	XmlConfigFile *configurationStorage = sp->storage();

	QDomElement contactsNode = configurationStorage->getNode("Contacts", XmlConfigFile::ModeFind);
	if (contactsNode.isNull())
		return;

	QDomNodeList contactsNodes = configurationStorage->getNodes(contactsNode, "Contact");
	int count = contactsNodes.count();
	for (int i = 0; i < count; i++)
	{
		QDomElement contactElement = contactsNodes.item(i).toElement();
		if (contactElement.isNull())
			continue;

		foreach (QString newGroup, contactElement.attribute("groups").split(",", QString::SkipEmptyParts))
			groups << newGroup;
	}

	foreach (QString groupName, groups)
		byName(groupName); // it can do import, too
}

void GroupManager::loadConfiguration()
{
	QDomElement groupsNode = xml_config_file->getNode("Groups", XmlConfigFile::ModeFind);
	if (groupsNode.isNull())
	{
		importConfiguration();
		return;
	}

	QDomNodeList groupNodes = groupsNode.elementsByTagName("Group");

	int count = groupNodes.count();
	for (int i = 0; i < count; i++)
	{
		QDomElement groupElement = groupNodes.at(i).toElement();
		if (groupElement.isNull())
			continue;

		StoragePoint *groupStoragePoint = new StoragePoint(xml_config_file, groupElement);
		addGroup(Group::loadFromStorage(groupStoragePoint));
	}
}

void GroupManager::storeConfiguration()
{
	foreach (Group *group, Groups)
		group->storeConfiguration();
}

void GroupManager::addGroup(Group *newGroup)
{
	emit groupAboutToBeAdded(newGroup);
	Groups << newGroup;
	emit groupAdded(newGroup);
}

Group * GroupManager::byUuid(const QString &uuid) const
{
	if (uuid.isEmpty())
		return 0;

	foreach (Group *group, Groups)
	{
		if (uuid == group->uuid().toString())
			return group;
	}

	return 0;
}

Group * GroupManager::byName(const QString &name, bool create)
{
	if (name.isEmpty())
		return 0;

	foreach (Group *group, Groups)
	{
		if (name == group->name())
			return group;
	}

	if (!create)
		return 0;

	Group *newGroup = new Group();
	newGroup->importConfiguration(name);
	addGroup(newGroup);

	return newGroup;
}