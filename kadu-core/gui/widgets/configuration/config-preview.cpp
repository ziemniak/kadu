/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QApplication>
#include <QtGui/QLabel>

#include "gui/widgets/configuration/config-preview.h"
#include "gui/widgets/configuration/config-group-box.h"

#include "debug.h"

ConfigPreview::ConfigPreview(const QString &widgetCaption, const QString &toolTip, ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
	: Preview(parentConfigGroupBox->widget()), ConfigWidget(widgetCaption, toolTip, parentConfigGroupBox, dataManager), label(0)
{
	createWidgets();
}

ConfigPreview::ConfigPreview(ConfigGroupBox *parentConfigGroupBox, ConfigurationWindowDataManager *dataManager)
	: Preview(parentConfigGroupBox->widget()), ConfigWidget(parentConfigGroupBox, dataManager), label(0)
{
}

ConfigPreview::~ConfigPreview()
{
	if (label)
		delete label;
}

void ConfigPreview::createWidgets()
{
	kdebugf();

	label = new QLabel(qApp->translate("@default", widgetCaption.toAscii().data()) + ":", parentConfigGroupBox->widget());
	parentConfigGroupBox->addWidgets(label, this);
}

void ConfigPreview::show()
{
	label->show();
	Preview::show();
}

void ConfigPreview::hide()
{
	label->hide();
	QWidget::hide();
}
