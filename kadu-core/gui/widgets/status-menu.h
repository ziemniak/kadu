/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATUS_MENU_H
#define STATUS_MENU_H

#include <QtCore/QObject>
#include <QtCore/QPoint>

#include "status/status.h"

#include "exports.h"

class QAction;
class QActionGroup;
class QMenu;

class StatusContainer;

class KADUAPI StatusMenu : public QObject
{
	Q_OBJECT

	QActionGroup * ChangeStatusActionGroup;
	QAction * ChangeDescription;
	QAction * ChangePrivateStatus;

	QPoint MousePositionBeforeMenuHide;

	StatusContainer *MyStatusContainer;

private slots:
	void aboutToHide();
	void changeStatus(QAction *action);
	void changeDescription();

	void changeStatusPrivate(bool toggled);
	void statusChanged();

public:
	explicit StatusMenu(StatusContainer *statusContainer, QWidget *parent = 0);
	virtual ~StatusMenu();

	void addToMenu(QMenu *menu);

};

#endif // STATUS_MENU_H
