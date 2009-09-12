 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>

#include "gui/widgets/choose-identity-widget.h"
#include "tlen_account.h"
#include "tlen_protocol_factory.h"

#include "tlen-create-account-widget.h"

TlenCreateAccountWidget::TlenCreateAccountWidget(QWidget *parent)
		: AccountCreateWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	createGui();
}

TlenCreateAccountWidget::~TlenCreateAccountWidget()
{
}

void TlenCreateAccountWidget::createGui()
{
	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(5);

	gridLayout->setColumnMinimumWidth(0, 20);
	gridLayout->setColumnStretch(3, 10);

	int row = 0;

	QLabel *nameLabel = new QLabel(tr("Account name") + ":", this);
	gridLayout->addWidget(nameLabel, row, 1, Qt::AlignRight);
	AccountName = new QLineEdit(this);
	gridLayout->addWidget(AccountName, row++, 2, 1, 2);

	createAccountGui(gridLayout, row);
	// TODO: register support
}

void TlenCreateAccountWidget::createAccountGui(QGridLayout *gridLayout, int &row)
{
	QLabel *numberLabel = new QLabel(tr("Tlen.pl login") + ":", this);
	gridLayout->addWidget(numberLabel, row, 1, Qt::AlignRight);
	AccountId = new QLineEdit(this);
	connect(AccountId, SIGNAL(textChanged(QString)), this, SLOT(iHaveAccountDataChanged()));
	gridLayout->addWidget(AccountId, row++, 2, 1, 2);

	QLabel *passwordLabel = new QLabel(tr("Password") + ":", this);
	gridLayout->addWidget(passwordLabel, row, 1, Qt::AlignRight);
	AccountPassword = new QLineEdit(this);
	connect(AccountPassword, SIGNAL(textChanged(QString)), this, SLOT(iHaveAccountDataChanged()));
	AccountPassword->setEchoMode(QLineEdit::Password);
	gridLayout->addWidget(AccountPassword, row, 2, 1, 2); // remove 1,2 if remind pass
	//RemindPassword = new QPushButton(tr("Forgot password"), this);
	//RemindPassword->setEnabled(false);
	//gridLayout->addWidget(RemindPassword, row++, 3, Qt::AlignLeft);
	row++;

	QLabel *descriptionLabel = new QLabel(tr("Account description"), this);
	gridLayout->addWidget(descriptionLabel, row, 1, Qt::AlignRight);
	Identity = new ChooseIdentityWidget(this);
	connect(Identity, SIGNAL(identityChanged()), this, SLOT(iHaveAccountDataChanged()));
	gridLayout->addWidget(Identity, row++, 2, 1, 2);

	RememberPassword = new QCheckBox(tr("Remember password"), this);
	RememberPassword->setChecked(true);
	gridLayout->addWidget(RememberPassword, row++, 2, 1, 2);

	AddThisAccount = new QPushButton(tr("Add this account"), this);
	AddThisAccount->setEnabled(false);
	connect(AddThisAccount, SIGNAL(clicked(bool)), this, SLOT(addThisAccount()));
	gridLayout->addWidget(AddThisAccount, row++, 1, 1, 4);

	Widgets.append(numberLabel);
	Widgets.append(AccountId);
	Widgets.append(passwordLabel);
	Widgets.append(AccountPassword);
	//Widgets.append(RemindPassword);
	Widgets.append(descriptionLabel);
	Widgets.append(Identity);
	Widgets.append(RememberPassword);
	Widgets.append(AddThisAccount);
}

void TlenCreateAccountWidget::iHaveAccountDataChanged()
{
	//RemindPassword->setEnabled(!AccountId->text().isEmpty());
	AddThisAccount->setEnabled(!AccountId->text().isEmpty() && !AccountPassword->text().isEmpty()
				   && !Identity->identityName().isEmpty());
}

void TlenCreateAccountWidget::addThisAccount()
{
	Account *tlenAccount = TlenProtocolFactory::instance()->newAccount();
	tlenAccount->setName(AccountName->text());
	tlenAccount->setId(AccountId->text());
	tlenAccount->setPassword(AccountPassword->text());
	tlenAccount->setRememberPassword(RememberPassword->isChecked());

	emit accountCreated(tlenAccount);
}
