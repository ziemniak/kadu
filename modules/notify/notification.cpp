/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtimer.h>

#include "notification.h"

Notification::Notification(const QString &type, const QString &icon, const UserListElements &userListElements)
	: Type(type), Ule(userListElements), Title(""), Text(""), Icon(icon), DefaultCallbackTimer(0)
{
}

Notification::~Notification()
{
}

void Notification::close()
{
	emit closed();
	deleteLater();
}

void Notification::clearCallbacks()
{
	Callbacks.clear();
}

void Notification::addCallback(const QString &caption, const char *slot)
{
	Callbacks.append(qMakePair(caption, slot));
}

void Notification::setDefaultCallback(int timeout, const char *defaultSlot)
{
	DefaultCallbackTimer = new QTimer(this);
	connect(DefaultCallbackTimer, SIGNAL(timeout()), this, defaultSlot);
	DefaultCallbackTimer->start(timeout, true);
}

void Notification::callbackAccept()
{
	close();
}

void Notification::callbackDiscard()
{
	close();
}

void Notification::clearDefaultCallback()
{
	if (DefaultCallbackTimer)
	{
		delete DefaultCallbackTimer;
		DefaultCallbackTimer = 0;
	}
}

QString Notification::type()
{
	return Type;
}

UserListElements Notification::userListElements()
{
	return Ule;
}

void Notification::setTitle(const QString &title)
{
	Title = title;
}

QString Notification::title()
{
	return Title;
}

void Notification::setText(const QString &text)
{
	Text = text;
}

QString Notification::text()
{
	return Text;
}

void Notification::setIcon(const QString &icon)
{
	Icon = icon;
}

QString Notification::icon()
{
	return Icon;
}

const QValueList<QPair<QString, const char *> > & Notification::getCallbacks()
{
	return Callbacks;
}