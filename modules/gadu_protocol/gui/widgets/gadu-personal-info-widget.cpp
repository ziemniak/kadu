 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

#include "debug.h"
#include "gadu-account.h"

#include "gadu-personal-info-widget.h"

GaduPersonalInfoWidget::GaduPersonalInfoWidget(Account *account, QWidget* parent) :
		QWidget(parent)
{
	createGui();

	if (!account)
		return;

	Service = account->protocol()->personalInfoService();
	if (!Service)
		return;

	connect(Service, SIGNAL(personalInfoAvailable(Contact)), this, SLOT(personalInfoAvailable(Contact)));
	Service->fetchPersonalInfo();
}

GaduPersonalInfoWidget::~GaduPersonalInfoWidget()
{
}


void GaduPersonalInfoWidget::createGui()
{
	QGridLayout *layout = new QGridLayout(this);

	QLabel *nickNameLabel = new QLabel(tr("Nick"), this);
	QLabel *firstNameLabel = new QLabel(tr("First name"), this);
	QLabel *lastNameLabel = new QLabel(tr("Last name"), this);
	QLabel *sexLabel = new QLabel(tr("Sex"), this);
	QLabel *familyNameLabel = new QLabel(tr("Family name"), this);
	QLabel *birthYearLabel = new QLabel(tr("Birth year"), this);
	QLabel *cityLabel = new QLabel(tr("City"), this);
	QLabel *familyCityLabel = new QLabel(tr("Family city"), this);

	NickName = new QLineEdit(this);
	FirstName = new QLineEdit(this);
	LastName = new QLineEdit(this);
	Sex = new QComboBox(this);
	Sex->addItem(tr("Unknown Gender"));
	Sex->addItem(tr("Male"));
	Sex->addItem(tr("Female"));
	FamilyName = new QLineEdit(this);
	BirthYear = new QLineEdit(this);
	BirthYear->setInputMask("d000");
	City = new QLineEdit(this);
	FamilyCity = new QLineEdit(this);

	layout->addWidget(nickNameLabel, 0, 0);
	layout->addWidget(NickName, 1, 0);
	layout->addWidget(firstNameLabel, 0, 1);
	layout->addWidget(FirstName, 1, 1);
	layout->addWidget(lastNameLabel, 0, 2);
	layout->addWidget(LastName, 1, 2);
	layout->addWidget(sexLabel, 2, 0);
	layout->addWidget(Sex, 3, 0);
	layout->addWidget(familyNameLabel, 2, 1);
	layout->addWidget(FamilyName, 3, 1);
	layout->addWidget(birthYearLabel, 4, 0);
	layout->addWidget(BirthYear, 5, 0);
	layout->addWidget(cityLabel, 6, 0);
	layout->addWidget(City, 7, 0);
	layout->addWidget(familyCityLabel, 6, 1);
	layout->addWidget(FamilyCity, 7, 1);
	
	layout->setRowStretch(8, 100);
	
}

void GaduPersonalInfoWidget::personalInfoAvailable(Contact contact)
{
	kdebugmf (KDEBUG_INFO,"personal info available");
	NickName->setText(contact.nickName());
	FirstName->setText(contact.firstName());
	LastName->setText(contact.lastName());
	Sex->setCurrentIndex((int)contact.gender());
	FamilyName->setText(contact.familyName());
	BirthYear->setText(QString::number(contact.birthYear()));
	City->setText(contact.city());
	FamilyCity->setText(contact.familyCity());
}

void GaduPersonalInfoWidget::applyData()
{
	Contact contact;

	contact.setNickName((*NickName).text());
	contact.setFirstName((*FirstName).text());
	contact.setLastName((*LastName).text());
	contact.setFamilyName((*FamilyName).text());
	contact.setBirthYear((*BirthYear).text().toUShort());
	contact.setCity((*City).text());
	contact.setFamilyCity((*FamilyCity).text());
	contact.setGender((ContactData::ContactGender)Sex->currentIndex());

	Service->updatePersonalInfo(contact);
}
