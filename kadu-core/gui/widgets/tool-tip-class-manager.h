/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOL_TIP_CLASS_MANAGER_H
#define TOOL_TIP_CLASS_MANAGER_H

#include <QtCore/QMap>
#include <QtCore/QStringList>

#include "configuration/configuration-aware-object.h"
#include "exports.h"

#include "abstract-tool-tip.h"

class KADUAPI ToolTipClassManager : private ConfigurationAwareObject
{
	Q_DISABLE_COPY(ToolTipClassManager)

	static ToolTipClassManager * Instance;

	QMap<QString, AbstractToolTip *> ToolTipClasses;
	QString ToolTipClassName;
	AbstractToolTip *CurrentToolTipClass;

	ToolTipClassManager();
	~ToolTipClassManager();

public:
	static ToolTipClassManager * instance();

	void registerToolTipClass(const QString &toolTipClassName, AbstractToolTip *toolTipClass);
	void unregisterToolTipClass(const QString &toolTipClassName);

	QStringList getToolTipClasses();
	void useToolTipClass(const QString &toolTipClassName);

	bool showToolTip(const QPoint &point, Contact contact);
	bool hideToolTip();

	virtual void configurationUpdated();

};

#endif // TOOL_TIP_CLASS_MANAGER_H
