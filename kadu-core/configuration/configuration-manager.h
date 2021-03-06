/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATION_MANAGER
#define CONFIGURATION_MANAGER

#include <QtCore/QList>

class StorableObject;

class ConfigurationManager
{
	QList<StorableObject *> RegisteredStorableObjects;

public:
	ConfigurationManager();

	void load();
	void store();

	void registerStorableObject(StorableObject *object);
	void unregisterStorableObject(StorableObject *object);

};

#endif // CONFIGURATION_MANAGER
