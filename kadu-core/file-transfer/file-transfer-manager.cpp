/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts/account.h"
#include "configuration/storage-point.h"
#include "file-transfer/file-transfer.h"
#include "protocols/protocol.h"
#include "protocols/services/file-transfer-service.h"

#include "modules.h"
#include "xml_config_file.h"

#include "file-transfer-manager.h"

FileTransferManager * FileTransferManager::Instance = 0;

FileTransferManager * FileTransferManager::instance()
{
	if (!Instance)
		Instance = new FileTransferManager();

	return Instance;
}

FileTransferManager::FileTransferManager()
{
	triggerAllAccountsRegistered();
}

FileTransferManager::~FileTransferManager()
{
	triggerAllAccountsUnregistered();
}

StoragePoint * FileTransferManager::createStoragePoint() const
{
	return new StoragePoint(xml_config_file, xml_config_file->getNode("FileTransfersNew"));
}

void FileTransferManager::loadConfigurationForAccount(Account *account)
{
	if (!isValidStorage())
		return;

	XmlConfigFile *configurationStorage = storage()->storage();
	QDomElement transfersNewNode = storage()->point();

	if (transfersNewNode.isNull())
		return;

	QDomNodeList fileTransferNodes = transfersNewNode.elementsByTagName("FileTransfer");

	int count = fileTransferNodes.count();

	QString uuid = account->uuid().toString();
	for (int i = 0; i < count; i++)
	{
		QDomElement fileTransferElement = fileTransferNodes.at(i).toElement();
		if (fileTransferElement.isNull())
			continue;

		if (configurationStorage->getTextNode(fileTransferElement, "Account") != uuid)
			continue;

		StoragePoint *contactStoragePoint = new StoragePoint(configurationStorage, fileTransferElement);
		FileTransfer *fileTransfer = FileTransfer::loadFromStorage(contactStoragePoint);

		if (fileTransfer)
			addFileTransfer(fileTransfer);
// 		else TODO: remove?
// 			transfersNewNode.removeChild(fileTransferElement);
	}
}

void FileTransferManager::accountRegistered(Account *account)
{
	loadConfigurationForAccount(account);

	Protocol *protocol = account->protocol();
	if (!protocol)
		return;

	FileTransferService *service = protocol->fileTransferService();
	if (!service)
		return;

	connect(service, SIGNAL(incomingFileTransfer(FileTransfer *)),
			this, SLOT(incomingFileTransfer(FileTransfer *)));
}

void FileTransferManager::storeConfigurationForAccount(Account *account)
{
	foreach (FileTransfer *fileTransfer, FileTransfers)
		if (fileTransfer->account() == account)
			fileTransfer->storeConfiguration();
}

void FileTransferManager::accountUnregistered(Account *account)
{
	storeConfigurationForAccount(account);

	Protocol *protocol = account->protocol();
	if (!protocol)
		return;

	FileTransferService *service = protocol->fileTransferService();
	if (!service)
		return;

	disconnect(service, SIGNAL(incomingFileTransfer(FileTransfer *)),
			this, SLOT(incomingFileTransfer(FileTransfer *)));
}

void FileTransferManager::addFileTransfer(FileTransfer *fileTransfer)
{
	emit fileTransferAboutToBeAdded(fileTransfer);
	FileTransfers.append(fileTransfer);
	emit fileTransferAdded(fileTransfer);
}

void FileTransferManager::removeFileTransfer(FileTransfer *fileTransfer)
{
	emit fileTransferAboutToBeRemoved(fileTransfer);
	FileTransfers.removeAll(fileTransfer);
	fileTransfer->removeFromStorage();
	emit fileTransferRemoved(fileTransfer);

	fileTransfer->deleteLater();
}

void FileTransferManager::cleanUp()
{
	foreach (FileTransfer *fileTransfer, FileTransfers)
		if (FileTransfer::StatusFinished == fileTransfer->transferStatus())
			removeFileTransfer(fileTransfer);
}

void FileTransferManager::incomingFileTransfer(FileTransfer *fileTransfer)
{
	if (!modules_manager->loadedModules().contains("file_transfer"))
		fileTransfer->reject();

	emit incomingFileTransferNeedAccept(fileTransfer);
}