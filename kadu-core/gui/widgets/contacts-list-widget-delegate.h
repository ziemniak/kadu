/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACTS_LIST_WIDGET_DELEGATE_H
#define CONTACTS_LIST_WIDGET_DELEGATE_H

#include <QtGui/QItemDelegate>

#include "accounts/accounts-aware-object.h"
#include "configuration/configuration-aware-object.h"
#include "contacts/contact.h"
#include "status/status.h"

class QTextDocument;

class AbstractContactsModel;
class Account;

class ContactsListWidgetDelegate : public QItemDelegate, public ConfigurationAwareObject, public AccountsAwareObject
{
	Q_OBJECT

	AbstractContactsModel *Model;

	QFont Font;
	QFont DescriptionFont;

	bool AlignTop;
	bool ShowBold;
	bool ShowDescription;
	bool ShowMultiLineDescription;
	QColor DescriptionColor;

	QPixmap DefaultAvatarPixmap;

	QTextDocument * descriptionDocument(const QString &text, int width, QColor color) const;
	bool isBold(const QModelIndex &index) const;
	QPixmap avatar(const QModelIndex &index) const;

private slots:
	void contactStatusChanged(Account *account, Contact contact, Status oldStatus);

protected:
	virtual void accountRegistered(Account *account);
	virtual void accountUnregistered(Account *account);

public:
	ContactsListWidgetDelegate(QObject *parent = 0);
	virtual ~ContactsListWidgetDelegate();

	virtual void setModel(AbstractContactsModel *model);

	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	virtual void configurationUpdated();
};

#endif // CONTACTS_LIST_WIDGET_DELEGATE_H
