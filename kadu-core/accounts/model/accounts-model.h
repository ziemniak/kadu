 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ACCOUNTS_MODEL
#define ACCOUNTS_MODEL

#include <QtCore/QAbstractListModel>
#include <QtCore/QModelIndex>
#include <QtCore/QStringList>

class AccountsModel : public QAbstractListModel
{
	Q_OBJECT

public:
	AccountsModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

};

#endif // ACCOUNTS_MODEL
