/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

#include "gui/widgets/configuration/config-group-box.h"
#include "gui/widgets/configuration/config-tab.h"

ConfigGroupBox::ConfigGroupBox(const QString &name, ConfigTab *configTab, QGroupBox *groupBox)
	: name(name), configTab(configTab), groupBox(groupBox)
{
	container = new QWidget(groupBox);
	groupBox->layout()->addWidget(container);

	gridLayout = new QGridLayout(container);
	gridLayout->setContentsMargins(5, 5, 5, 5);
	gridLayout->setColumnStretch(1, 100);
}

ConfigGroupBox::~ConfigGroupBox()
{
	delete groupBox;

	configTab->removedConfigGroupBox(name);
}

bool ConfigGroupBox::empty()
{
	return container->children().count() == 1;
}

void ConfigGroupBox::addWidget(QWidget *widget, bool fullSpace)
{
	int numRows = gridLayout->rowCount();

	if (fullSpace)
		gridLayout->addWidget(widget, numRows, 0, 1, 2);
	else
		gridLayout->addWidget(widget, numRows, 1);
}

void ConfigGroupBox::addWidgets(QWidget *widget1, QWidget *widget2)
{
	int numRows = gridLayout->rowCount();

	if (widget1)
		gridLayout->addWidget(widget1, numRows, 0, Qt::AlignRight);

	if (widget2)
		gridLayout->addWidget(widget2, numRows, 1);
}

void ConfigGroupBox::insertWidget(int pos, QWidget *widget, bool fullSpace)
{
    	if (fullSpace)
		gridLayout->addWidget(widget, pos, 0, 1, 2);
	else
		gridLayout->addWidget(widget, pos, 1);
}

void ConfigGroupBox::insertWidgets(int pos, QWidget *widget1, QWidget *widget2)
{
    	if (widget1)
		gridLayout->addWidget(widget1, pos, 0, Qt::AlignRight);

	if (widget2)
		gridLayout->addWidget(widget2, pos, 1);
}
