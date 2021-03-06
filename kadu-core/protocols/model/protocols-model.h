 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROTOCOLS_MODEL
#define PROTOCOLS_MODEL

#include <QtCore/QAbstractListModel>
#include <QtCore/QModelIndex>

#include "model/first-empty.h"

class ProtocolFactory;

class ProtocolsModel : public FirstEmpty
{
	Q_OBJECT

public:
	ProtocolsModel(const QString &emptyString, QObject *parent = 0);
	explicit ProtocolsModel(QObject *parent = 0);
	virtual ~ProtocolsModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	ProtocolFactory * protocolFactory(const QModelIndex &index) const;
	int protocolFactoryIndex(ProtocolFactory *protocolFactory);

};

#endif // PROTOCOLS_MODEL
