/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACT_ACCOUNT_DATA
#define CONTACT_ACCOUNT_DATA

#include <QtNetwork/QHostAddress>
#include <QtXml/QDomElement>

#include "configuration/storable-object.h"
#include "contacts/avatar.h"
#include "status/status.h"

#include "contact.h"

#include "exports.h"

class Account;
class XmlConfigFile;

class KADUAPI ContactAccountData : public QObject, public StorableObject
{
	Q_OBJECT

	Account *ContactAccount;
	Avatar ContactAvatar;
	Contact OwnerContact;
	QString Id;

	Status CurrentStatus;

	QString ProtocolVersion;

	QHostAddress Address;
	unsigned int Port;
	QString DnsName;

	bool Blocked;
	bool OfflineTo;

protected:
	virtual StoragePoint * createStoragePoint();

public:
	ContactAccountData(Contact contact, Account *account, const QString &id = QString::null, bool loadFromConfiguration = true);

	virtual bool validateId() {return false;}
	virtual void load();
	virtual void store();

	Account * account() { return ContactAccount; }
	Contact contact() { return OwnerContact; }
	Avatar & avatar() { return ContactAvatar; }

	QString id() { return Id; }
	void setId(const QString &newId);

	bool isValid();

	bool hasFeature() { return false; }

	QString protocolVersion() { return ProtocolVersion; }
	void setProtocolVersion(QString protocolVersion) { ProtocolVersion = protocolVersion; }

	QHostAddress ip() { return Address; }
	void setIp(QHostAddress addres) { Address = addres; }

	unsigned int port() { return Port; }
	void setPort(int port) { Port = port; }

	QString dnsName() { return DnsName; }
	void refreshDNSName();

	Status status() { return CurrentStatus; }
	void setStatus(Status status) { CurrentStatus = status; }

	// properties

	bool isBlocked() { return Blocked; }
	void setBlocked(bool blocked) { Blocked = blocked; }

	bool isOfflineTo() { return OfflineTo; }
	void setOfflineTo(bool offlineTo) { OfflineTo = offlineTo; }

public slots:
	void setDNSName(const QString &ident, const QString &dnsName) { DnsName = dnsName; }

signals:
	void idChanged(const QString &id);

};

#endif
