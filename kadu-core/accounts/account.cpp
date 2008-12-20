/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "account_data.h"
#include "protocols/protocol.h"
#include "protocols/protocol_factory.h"
#include "protocols/protocols_manager.h"
#include "xml_config_file.h"

#include "account.h"

Account::Account(const QString &name)
	: Name(name), ProtocolHandler(0), Data(0)
{
}

Account::Account(const QString &name, Protocol *protocol, AccountData *data)
	: Name(name), ProtocolHandler(protocol), Data(data)
{
	protocol->setData(Data);
}

Account::~Account()
{
	if (0 != ProtocolHandler)
	{
		delete ProtocolHandler;
		ProtocolHandler = 0;
	}
}

bool Account::loadConfiguration(XmlConfigFile *configurationStorage, QDomElement parent)
{
	QString protocolName = configurationStorage->getTextNode(parent, "Protocol");

	ProtocolHandler = ProtocolsManager::instance()->newInstance(protocolName);
	if (0 == ProtocolHandler)
		return false;

	Data = ProtocolHandler->createAccountData();
	if (0 == Data)
		return false;

	ProtocolHandler->setData(Data);
	return Data->loadConfiguration(configurationStorage, parent);
}

void Account::storeConfiguration(XmlConfigFile *configurationStorage, QDomElement parent)
{
	configurationStorage->createTextNode(
			parent, "Protocol",
			ProtocolHandler->protocolFactory()->name());
	Data->storeConfiguration(configurationStorage, parent);
}

UserStatus Account::currentStatus()
{
	return (0 == ProtocolHandler) ? UserStatus() : ProtocolHandler->currentStatus();
}