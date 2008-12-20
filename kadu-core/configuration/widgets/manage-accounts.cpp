/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include "accounts/account.h"
#include "accounts/account_data.h"
#include "accounts/account_manager.h"
#include "gui/widgets/configuration/configuration-window.h"
#include "protocols/protocol.h"
#include "protocols/protocol_factory.h"
#include "protocols/protocols_manager.h"
#include "icons_manager.h"

#include "manage-accounts.h"

ManageAccounts::ManageAccounts(QWidget *parent)
		: QWidget(parent)
{
	createGui();
	loadAccounts();
}

ManageAccounts::~ManageAccounts()
{
}

void ManageAccounts::createGui()
{
	AccountsListWidget = new QListWidget(this);
	AddAccountButton = new QPushButton(tr("Add account..."), this);
	AddAccountButton->setMenu(createGuiAddAccountMenu());

	EditAccountButton = new QPushButton(tr("Edit account..."), this);
	RemoveAccountButton = new QPushButton(tr("Remove account..."), this);
	MoveUpAccountButton = new QPushButton(tr("Move up"), this);
	MoveDownAccountButton = new QPushButton(tr("Move down"), this);

	connect(EditAccountButton, SIGNAL(clicked()), this, SLOT(editAccount()));
	connect(RemoveAccountButton, SIGNAL(clicked()), this, SLOT(removeAccount()));

	QVBoxLayout *buttonLayout = new QVBoxLayout();
	buttonLayout->addWidget(AddAccountButton);
	buttonLayout->addWidget(EditAccountButton);
	buttonLayout->addWidget(RemoveAccountButton);
	buttonLayout->addWidget(MoveUpAccountButton);
	buttonLayout->addWidget(MoveDownAccountButton);
	buttonLayout->addStretch(100);

	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget(AccountsListWidget, 3);
	mainLayout->addItem(buttonLayout);

	setLayout(mainLayout);
}

QMenu * ManageAccounts::createGuiAddAccountMenu()
{
	QMenu *addAccountMenu = new QMenu(this);

	foreach (ProtocolFactory *protocolFactory, ProtocolsManager::instance()->protocolFactories())
	{
		QIcon icon = icons_manager->loadIcon(protocolFactory->iconName());
		QString name = protocolFactory->name();
		QString displayName = protocolFactory->displayName();

		QAction *protocolAction = addAccountMenu->addAction(icon, displayName,
			this, SLOT(addAccount()));
		protocolAction->setData(name);
	}

	return addAccountMenu;
}

void ManageAccounts::loadAccounts()
{
	AccountsListWidget->clear();

	foreach (Account *account, AccountManager::instance()->accounts())
	{
		QListWidgetItem *accountListWidgetItem = new QListWidgetItem();

		accountListWidgetItem->setText(account->name());
		accountListWidgetItem->setIcon(account->protocol()->icon());

		AccountsListWidget->addItem(accountListWidgetItem);
	}
}

void ManageAccounts::addAccount()
{
	QAction *senderAction = dynamic_cast<QAction *>(sender());
	if (0 == senderAction)
		return;

	QString protocolName = senderAction->data().toString();
	if (protocolName.isEmpty())
		return;

	ProtocolFactory *protocolFactory = ProtocolsManager::instance()->protocolFactory(protocolName);
	if (0 == protocolFactory)
		return;

	AccountData *newAccountData = protocolFactory->newAccountData();
	if (0 == newAccountData)
		return;

	ConfigurationWindow *configurationDialog = protocolFactory->newConfigurationDialog(newAccountData, this);
	if (0 == configurationDialog)
	{
		delete newAccountData;
		return;
	}

	configurationDialog->setWindowModality(Qt::WindowModal);
	if (configurationDialog->exec() == QDialog::Accepted)
	{
		Account *newAccount = AccountManager::instance()->createAccount(
				newAccountData->name(), protocolName , newAccountData);
		AccountManager::instance()->registerAccount(newAccount);
		loadAccounts();
		return;
	}
	delete newAccountData;
}

void ManageAccounts::removeAccount()
{
	QListWidgetItem *currentAccountItem = AccountsListWidget->currentItem();
	if (0 == currentAccountItem)
		return;

	QString accountName = currentAccountItem->text();

	AccountManager::instance()->unregisterAccount(accountName);
	loadAccounts();
}

void ManageAccounts::editAccount()
{
	QListWidgetItem *currentAccountItem = AccountsListWidget->currentItem();
	if (0 == currentAccountItem)
		return;

	QString accountName = currentAccountItem->text();
	Account *account = AccountManager::instance()->account(accountName);
	if (0 == account)
		return;

	Protocol *protocol = account->protocol();
	if (0 == protocol)
		return;

	ProtocolFactory *protocolFactory = protocol->protocolFactory();
	if (0 == protocolFactory)
		return;

	ConfigurationWindow *configurationDialog = protocolFactory->newConfigurationDialog(account->data(), this);
	if (0 == configurationDialog)
		return;

	configurationDialog->setWindowModality(Qt::WindowModal);
	if (QDialog::Accepted == configurationDialog->exec())
		loadAccounts();
}