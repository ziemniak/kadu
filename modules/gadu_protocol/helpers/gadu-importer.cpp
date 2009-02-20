/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"
#include "accounts/account_manager.h"

#include "contacts/contact-manager.h"
#include "contacts/contact.h"
#include "contacts/ignored-helper.h"

#include "config_file.h"
#include "misc.h"
#include "xml_config_file.h"

#include "gadu_account_data.h"
#include "gadu-contact-account-data.h"

#include "gadu-importer.h"

GaduImporter * GaduImporter::Instance;

GaduImporter * GaduImporter::instance()
{
	if (0 == Instance)
		Instance = new GaduImporter();

	return Instance;
}

void GaduImporter::importAccounts()
{
	if (0 == config_file.readNumEntry("General", "UIN"))
		return;

	GaduAccountData *gaduAccountData = new GaduAccountData(
			"Gadu-Gadu",
			config_file.readNumEntry("General", "UIN"),
			unicode2cp(pwHash(config_file.readEntry("General", "Password"))));

	Account *defaultGaduGadu = AccountManager::instance()->createAccount(
			"gadu", gaduAccountData);
	AccountManager::instance()->registerAccount(defaultGaduGadu);
}

void GaduImporter::importContacts()
{
	connect(ContactManager::instance(), SIGNAL(contactAdded(Contact &)),
			this, SLOT(contactAdded(Contact &)));

	foreach (Contact contact, ContactManager::instance()->contacts())
		contactAdded(contact);

	importIgnored();
}

void GaduImporter::importGaduContact(Contact& contact)
{
	Account *account = AccountManager::instance()->defaultAccount();
	QString id = contact.customData()["uin"];

	GaduContactAccountData *gcad = new GaduContactAccountData(contact, account, id);

	gcad->setBlocked(QVariant(contact.customData()["blocking"]).toBool());
	gcad->setOfflineTo(QVariant(contact.customData()["offline_to"]).toBool());

	contact.customData().remove("uin");
	contact.customData().remove("blocking");
	contact.customData().remove("offline_to");

	contact.addAccountData(gcad);
}

void GaduImporter::importIgnored()
{
	Account *account = AccountManager::instance()->defaultAccount();
	if (!account)
		return;

	QDomElement ignored = xml_config_file->getNode("Ignored", XmlConfigFile::ModeFind);
	if (ignored.isNull())
		return;

	QDomNodeList ignoredGroups = xml_config_file->getNodes(ignored, "IgnoredGroup");
	for (int i = 0; i < ignoredGroups.count(); i++)
	{
		QDomElement ignoredGroup = ignoredGroups.item(i).toElement();
		if (ignoredGroup.isNull())
			continue;

		ContactList ignoredList;
		QDomNodeList ignoredContacts = xml_config_file->getNodes(ignoredGroup, "IgnoredContact");
		for (int j = 0; j < ignoredContacts.count(); j++)
		{
			QDomElement ignoredContact = ignoredContacts.item(j).toElement();
			if (ignoredContact.isNull())
				continue;

			ignoredList.append(ContactManager::instance()->byId(account, ignoredContact.attribute("uin")));
		}

		if (0 == ignoredList.count())
			continue;

		IgnoredHelper::setIgnored(ignoredList);
	}
}

void GaduImporter::contactAdded(Contact &contact)
{
	if (contact.customData().contains("uin"))
		importGaduContact(contact);
}