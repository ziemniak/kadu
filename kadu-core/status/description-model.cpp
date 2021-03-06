/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "status/description-manager.h"

#include "description-model.h"

// TODO: 0.6.6
// 	while (defaultdescriptions.count() > config_file.readNumEntry("General", "NumberOfDescriptions"))
// 		defaultdescriptions.pop_back();
// 	Description->setMaxVisibleItems(30);

DescriptionModel::DescriptionModel(DescriptionManager *manager) :
		Manager(manager)
{
	connect(Manager, SIGNAL(descriptionAboutToBeAdded(const QString &)),
			this, SLOT(descriptionAboutToBeAdded(const QString &)));
	connect(Manager, SIGNAL(descriptionAdded(const QString &)),
			this, SLOT(descriptionAdded(const QString &)));
	connect(Manager, SIGNAL(descriptionAboutToBeRemoved(const QString &)),
			this, SLOT(descriptionAboutToBeRemoved(const QString &)));
	connect(Manager, SIGNAL(descriptionRemoved(const QString &)),
			this, SLOT(descriptionRemoved(const QString &)));
}

DescriptionModel::~DescriptionModel()
{
	disconnect(Manager, SIGNAL(descriptionAboutToBeAdded(const QString &)),
			this, SLOT(descriptionAboutToBeAdded(const QString &)));
	disconnect(Manager, SIGNAL(descriptionAdded(const QString &)),
			this, SLOT(descriptionAdded(const QString &)));
	disconnect(Manager, SIGNAL(descriptionAboutToBeRemoved(const QString &)),
			this, SLOT(descriptionAboutToBeRemoved(const QString &)));
	disconnect(Manager, SIGNAL(descriptionRemoved(const QString &)),
			this, SLOT(descriptionRemoved(const QString &)));
}

int DescriptionModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return Manager->content().count();
}

Qt::ItemFlags DescriptionModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // fake
}

QVariant DescriptionModel::data(const QModelIndex &index, int role) const
{
	if (Qt::DisplayRole != role)
		return QVariant();

	if (!index.isValid())
		return QVariant();

	if (0 != index.column())
		return QVariant();

	if (index.row() < 0 || index.row() >= Manager->content().count())
		return QVariant();

	return Manager->content()[index.row()];
}

void DescriptionModel::descriptionAboutToBeAdded(const QString &description)
{
	beginInsertRows(QModelIndex(), 0, 0);
}

void DescriptionModel::descriptionAdded(const QString &description)
{
	endInsertRows();
}

void DescriptionModel::descriptionAboutToBeRemoved(const QString &description)
{
	int index = Manager->content().indexOf(description);
	beginRemoveRows(QModelIndex(), index, index);
}

void DescriptionModel::descriptionRemoved(const QString &description)
{
	endRemoveRows();
}
