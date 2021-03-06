/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QListWidget>
#include <QtXml/QDomElement>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QDialogButtonBox>

#include "configuration/configuration-file.h"
#include "gui/widgets/configuration/configuration-widget.h"
#include "gui/widgets/configuration/config-group-box.h"
#include "gui/widgets/configuration/config-widget.h"
#include "gui/widgets/configuration/config-section.h"
#include "gui/widgets/configuration/config-line-edit.h"
#include "gui/widgets/configuration/config-gg-password-edit.h"
#include "gui/widgets/configuration/config-check-box.h"
#include "gui/widgets/configuration/config-spin-box.h"
#include "gui/widgets/configuration/config-combo-box.h"
#include "gui/widgets/configuration/config-hot-key-edit.h"
#include "gui/widgets/configuration/config-path-list-edit.h"
#include "gui/widgets/configuration/config-color-button.h"
#include "gui/widgets/configuration/config-select-font.h"
#include "gui/widgets/configuration/config-syntax-editor.h"
#include "gui/widgets/configuration/config-action-button.h"
#include "gui/widgets/configuration/config-select-file.h"
#include "gui/widgets/configuration/config-preview.h"
#include "gui/widgets/configuration/config-slider.h"
#include "gui/widgets/configuration/config-label.h"
#include "gui/widgets/configuration/config-list-widget.h"
#include "gui/windows/configuration-window.h"

#include "debug.h"

ConfigurationWidget::ConfigurationWidget(ConfigurationWindowDataManager *dataManager, QWidget *parent)
	: QWidget(parent), currentSection(0), dataManager(dataManager)
{
	QHBoxLayout *center_layout = new QHBoxLayout(this);
	center_layout->setMargin(0);
	center_layout->setSpacing(0);

	left = new QWidget();
	left->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	left->hide();
	QVBoxLayout *left_layout = new QVBoxLayout(left);
	left_layout->setMargin(0);
	left_layout->setSpacing(0);

	container = new QWidget;
	new QHBoxLayout(container);

	sectionsListWidget = new QListWidget(this);
	sectionsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sectionsListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	sectionsListWidget->setIconSize(QSize(32, 32));
	connect(sectionsListWidget, SIGNAL(currentTextChanged(const QString &)), this, SLOT(changeSection(const QString &)));
	left_layout->addWidget(sectionsListWidget);

	center_layout->addWidget(left);
	center_layout->addWidget(container);
}

ConfigurationWidget::~ConfigurationWidget()
{
	if (sectionsListWidget->currentItem())
		config_file.writeEntry("General", "ConfigurationWindow_" + Name, sectionsListWidget->currentItem()->text());
	qDeleteAll(configSections);
}

void ConfigurationWidget::init()
{
	QString lastSection = config_file.readEntry("General", "ConfigurationWindow_" + Name);
	if (configSections.contains(lastSection))
		configSections[lastSection]->activate();
	else
		sectionsListWidget->setCurrentItem(0);
}

QList<ConfigWidget *> ConfigurationWidget::appendUiFile(const QString &fileName, bool load)
{
	QList<ConfigWidget *> widgets = processUiFile(fileName);

	if (load)
		foreach(ConfigWidget *widget, widgets)
			if (widget)
				widget->loadConfiguration();

	changeSection(*configSections.keys().begin());

	return widgets;
}

void ConfigurationWidget::removeUiFile(const QString &fileName)
{
	processUiFile(fileName, false);
}

QList<ConfigWidget *>  ConfigurationWidget::processUiFile(const QString &fileName, bool append)
{
	kdebugf();

	QList<ConfigWidget *> result;
	QFile file(fileName);

	QDomDocument uiFile;
	file.open(QIODevice::ReadOnly);

	if (!uiFile.setContent(&file))
	{
		kdebugf2();
		file.close();
		return result;
	}

	file.close();

	QDomElement kaduConfigurationUi = uiFile.documentElement();
	if (kaduConfigurationUi.tagName() != "configuration-ui")
	{
		kdebugf2();
		return result;
	}

	QDomNodeList children = kaduConfigurationUi.childNodes();
	int length = children.length();
	for (int i = 0; i < length; i++)
		result += processUiSectionFromDom(children.item(i), append);

	kdebugf2();
	return result;
}

QList<ConfigWidget *> ConfigurationWidget::processUiSectionFromDom(QDomNode sectionNode, bool append)
{
	kdebugf();

	QList<ConfigWidget *> result;
	if (!sectionNode.isElement())
	{
		kdebugf2();
		return result;
	}

	const QDomElement &sectionElement = sectionNode.toElement();
	if (sectionElement.tagName() != "section")
	{
		kdebugf2();
		return result;
	}

	const QString &iconName = sectionElement.attribute("icon");

	const QString &sectionName = sectionElement.attribute("name");
	if (sectionName.isEmpty())
	{
		kdebugf2();
		return result;
	}

	configSection(iconName, qApp->translate("@default", sectionName.toAscii().data()), true);

	const QDomNodeList children = sectionElement.childNodes();
	int length = children.length();
	for (int i = 0; i < length; i++)
		result += processUiTabFromDom(children.item(i), iconName, sectionName, append);

	kdebugf2();
	return result;
}

QList<ConfigWidget *> ConfigurationWidget::processUiTabFromDom(QDomNode tabNode, const QString &iconName,
	const QString &sectionName, bool append)
{
	kdebugf();

	QList<ConfigWidget *> result;
	if (!tabNode.isElement())
	{
		kdebugf2();
		return result;
	}

	const QDomElement &tabElement = tabNode.toElement();
	if (tabElement.tagName() != "tab")
	{
		kdebugf2();
		return result;
	}

	const QString tabName = tabElement.attribute("name");
	if (tabName.isEmpty())
	{
		kdebugf2();
		return result;
	}

	const QDomNodeList &children = tabElement.childNodes();
	int length = children.length();
	for (int i = 0; i < length; i++)
		result += processUiGroupBoxFromDom(children.item(i), sectionName, tabName, append);

	kdebugf2();
	return result;
}

QList<ConfigWidget *> ConfigurationWidget::processUiGroupBoxFromDom(QDomNode groupBoxNode, const QString &sectionName, const QString &tabName, bool append)
{
	kdebugf();

	QList<ConfigWidget *> result;
	if (!groupBoxNode.isElement())
	{
		kdebugf2();
		return result;
	}

	const QDomElement &groupBoxElement = groupBoxNode.toElement();
	if (groupBoxElement.tagName() != "group-box")
	{
		kdebugf2();
		return result;
	}

	const QString groupBoxName = groupBoxElement.attribute("name");
	if (groupBoxName.isEmpty())
	{
		kdebugf2();
		return result;
	}

	const QString groupBoxId = groupBoxElement.attribute("id");

	ConfigGroupBox *configGroupBoxWidget = configGroupBox(sectionName, tabName, groupBoxName, append);
	if (!configGroupBoxWidget)
	{
		kdebugf2();
		return result;
	}

	if (!groupBoxId.isEmpty())
		widgets[groupBoxId] = dynamic_cast<QWidget *>(configGroupBoxWidget->widget());

	const QDomNodeList &children = groupBoxElement.childNodes();
	int length = children.length();
	for (int i = 0; i < length; i++)
		if (append)
			result.append(appendUiElementFromDom(children.item(i), configGroupBoxWidget));
		else
			removeUiElementFromDom(children.item(i), configGroupBoxWidget);

	kdebugf2();
	return result;
}

ConfigWidget * ConfigurationWidget::appendUiElementFromDom(QDomNode uiElementNode, ConfigGroupBox *configGroupBox)
{
	kdebugf();

	if (!uiElementNode.isElement())
	{
		kdebugf2();
		return 0;
	}

	const QDomElement &uiElement = uiElementNode.toElement();
	const QString &tagName = uiElement.tagName();
	ConfigWidget *widget = 0;

	if (tagName == "line-edit")
		widget = new ConfigLineEdit(configGroupBox, dataManager);
	else if (tagName == "gg-password-edit")
		widget = new ConfigGGPasswordEdit(configGroupBox, dataManager);
	else if (tagName == "check-box")
		widget = new ConfigCheckBox(configGroupBox, dataManager);
	else if (tagName == "spin-box")
		widget = new ConfigSpinBox(configGroupBox, dataManager);
	else if (tagName == "combo-box")
		widget = new ConfigComboBox(configGroupBox, dataManager);
	else if (tagName == "hot-key-edit")
		widget = new ConfigHotKeyEdit(configGroupBox, dataManager);
	else if (tagName == "path-list-edit")
		widget = new ConfigPathListEdit(configGroupBox, dataManager);
	else if (tagName == "color-button")
		widget = new ConfigColorButton(configGroupBox, dataManager);
	else if (tagName == "select-font")
		widget = new ConfigSelectFont(configGroupBox, dataManager);
	else if (tagName == "syntax-editor")
		widget = new ConfigSyntaxEditor(configGroupBox, dataManager);
	else if (tagName == "action-button")
		widget = new ConfigActionButton(configGroupBox, dataManager);
	else if (tagName == "select-file")
		widget = new ConfigSelectFile(configGroupBox, dataManager);
	else if (tagName == "preview")
		widget = new ConfigPreview(configGroupBox, dataManager);
	else if (tagName == "slider")
		widget = new ConfigSlider(configGroupBox, dataManager);
	else if (tagName == "label")
		widget = new ConfigLabel(configGroupBox, dataManager);
	else if (tagName == "list-box")
		widget = new ConfigListWidget(configGroupBox, dataManager);
	else
	{
		kdebugf2();
		return 0;
	}

	if (!widget->fromDomElement(uiElement))
	{
		delete widget;
		kdebugf2();
		return 0;
	}

	QString id = uiElement.attribute("id");
	if (!id.isEmpty())
		widgets[id] = dynamic_cast<QWidget *>(widget);

	widget->show();

	kdebugf2();
	return widget;
}

void ConfigurationWidget::removeUiElementFromDom(QDomNode uiElementNode, ConfigGroupBox *configGroupBox)
{
	kdebugf();

	if (!uiElementNode.isElement())
	{
		kdebugf2();
		return;
	}

	const QDomElement &uiElement = uiElementNode.toElement();
	const QString &caption = uiElement.attribute("caption");

	foreach(QObject *child, configGroupBox->widget()->children())
	{
		ConfigWidget *configWidget = dynamic_cast<ConfigWidget *>(child);
		if (!configWidget)
			continue;

		if (configWidget->widgetCaption == caption)
		{
			delete configWidget;
			break;
		}
	}

	if (configGroupBox->empty())
		delete configGroupBox;

	kdebugf2();
}

QWidget * ConfigurationWidget::widgetById(const QString &id)
{
	if (widgets.contains(id))
		return widgets[id];

	return 0;
}

ConfigGroupBox * ConfigurationWidget::configGroupBox(const QString &section, const QString &tab, const QString &groupBox, bool create)
{
	ConfigSection *s = configSection(qApp->translate("@default", section.toAscii().data()));
	if (!s)
		return 0;

	return s->configGroupBox(qApp->translate("@default", tab.toAscii().data()), qApp->translate("@default", groupBox.toAscii().data()), create);
}

ConfigSection *ConfigurationWidget::configSection(const QString &name)
{
	return configSections[name];
}

ConfigSection *ConfigurationWidget::configSection(const QString &pixmap, const QString &name, bool create)
{
	if (configSections.contains(name))
		return configSections[name];

	if (!create)
		return 0;

	QListWidgetItem *newConfigSectionListWidgetItem = new QListWidgetItem(IconsManager::instance()->loadPixmap(pixmap), name, sectionsListWidget);

	QFontMetrics fontMetrics = sectionsListWidget->fontMetrics();
	// TODO: 48 = margins + scrollbar - get real scrollbar width
	int width = fontMetrics.width(name) + IconsManager::instance()->loadPixmap(pixmap).width() + 48;

	ConfigSection *newConfigSection = new ConfigSection(name, this, newConfigSectionListWidgetItem, container, pixmap);
	configSections[name] = newConfigSection;

	if (configSections.count() == 1)
		sectionsListWidget->setFixedWidth(width);

	if (configSections.count() > 1)
	{
		if (sectionsListWidget->width() < width)
			sectionsListWidget->setFixedWidth(width);
		left->show();
	}

	return newConfigSection;
}

void ConfigurationWidget::loadConfiguration(QObject *object)
{
	kdebugf();

	if (!object)
		return;

	const QObjectList children = object->children();
	foreach(QObject *child, children)
		loadConfiguration(child);

	ConfigWidget *configWidget = dynamic_cast<ConfigWidget *>(object);
	if (configWidget)
		configWidget->loadConfiguration();
}

void ConfigurationWidget::loadConfiguration()
{
	loadConfiguration(this);
}

void ConfigurationWidget::saveConfiguration(QObject *object)
{
	kdebugf();

	if (!object)
		return;

	const QObjectList children = object->children();
	foreach(QObject *child, children)
		saveConfiguration(child);

	ConfigWidget *configWidget = dynamic_cast<ConfigWidget *>(object);
	if (configWidget)
		configWidget->saveConfiguration();
}

void ConfigurationWidget::saveConfiguration()
{
	saveConfiguration(this);
}

void ConfigurationWidget::changeSection(const QString &newSectionName)
{
	if (!configSections.contains(newSectionName))
		return;

	ConfigSection *newSection = configSections[newSectionName];
	if (newSection == currentSection)
		return;

	if (currentSection)
		currentSection->hide();

	currentSection = newSection;
	newSection->show();
	newSection->activate();
}

void ConfigurationWidget::removedConfigSection(const QString &sectionName)
{
	configSections.remove(sectionName);

	for (unsigned int i = 0; i < sectionsListWidget->count(); i++)
		if (sectionsListWidget->item(i)->text() == tr(sectionName.toAscii().data()))
		{
			QListWidgetItem *item = sectionsListWidget->takeItem(i);
			delete item;
			break;
		}
}
