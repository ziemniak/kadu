/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATION_WINDOW_WIDGETS_H
#define CONFIGURATION_WINDOW_WIDGETS_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qvbox.h>

#include "color_button.h"
#include "hot_key.h"
#include "path_list_edit.h"
#include "preview.h"
#include "select_file.h"
#include "select_font.h"
#include "syntax_editor.h"

class ConfigGroupBox;

class ConfigWidget
{
protected:
	ConfigGroupBox *parentConfigGroupBox;
	QString widgetCaption;

	virtual void createWidgets() = 0;

public:
	ConfigWidget(ConfigGroupBox *parentConfigGroupBox);
	ConfigWidget(const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox);
	virtual ~ConfigWidget() {}

	virtual void loadConfiguration() = 0;
	virtual void saveConfiguration() = 0;

	virtual bool fromDomElement(QDomElement domElement) ;
};

class ConfigWidgetValue : public ConfigWidget
{
protected:
	QString section;
	QString item;

public:
	ConfigWidgetValue(ConfigGroupBox *parentConfigGroupBox);
	ConfigWidgetValue(const QString &widgetCaption, const QString &section, const QString &item, ConfigGroupBox *parentConfigGroupBox);
	virtual ~ConfigWidgetValue() {}

	virtual bool fromDomElement(QDomElement domElement);
};

class ConfigLineEdit : public QLineEdit, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigLineEdit(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigLineEdit(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigLineEdit() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigGGPasswordEdit : public ConfigLineEdit
{
public:
	ConfigGGPasswordEdit(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigGGPasswordEdit(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigGGPasswordEdit() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigCheckBox : public QCheckBox, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigCheckBox(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigCheckBox(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigCheckBox() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigSpinBox : public QSpinBox, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigSpinBox(const QString &section, const QString &item, const QString &widgetCaption,
		int minValue, int maxValue, int step, ConfigGroupBox *parentConfigGroupBox, const char *name = 0);
	ConfigSpinBox(ConfigGroupBox *parentConfigGroupBox, const char *name = 0);
	virtual ~ConfigSpinBox() {};

	virtual void loadConfiguration();
	virtual void saveConfiguration();

	virtual bool fromDomElement(QDomElement domElement);
};

class ConfigComboBox : public QComboBox, public ConfigWidgetValue
{
	QStringList itemValues;
	QStringList itemCaptions;

protected:
	virtual void createWidgets();

public:
	ConfigComboBox(const QString &section, const QString &item, const QString &widgetCaption, const QStringList &itemValues, const QStringList &itemCaptions,
		ConfigGroupBox *parentConfigGroupBox, const char *name = 0);
	ConfigComboBox(ConfigGroupBox *parentConfigGroupBox, const char *name = 0);
	virtual ~ConfigComboBox() {};

	void setItems(const QStringList &itemValues, const QStringList &itemCaptions);
	QString currentItemValue() { return itemValues[currentItem()]; }

	virtual void loadConfiguration();
	virtual void saveConfiguration();

	virtual bool fromDomElement(QDomElement domElement);
};

class ConfigHotKeyEdit : public HotKeyEdit, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigHotKeyEdit(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigHotKeyEdit(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigHotKeyEdit() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigPathListEdit : public PathListEdit, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigPathListEdit(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigPathListEdit(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigPathListEdit() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigColorButton : public ColorButton, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigColorButton(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigColorButton(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigColorButton() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigSelectFont : public SelectFont, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigSelectFont(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigSelectFont(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigSelectFont() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();
};

class ConfigSyntaxEditor : public SyntaxEditor, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigSyntaxEditor(const QString &section, const QString &item, const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigSyntaxEditor(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigSyntaxEditor() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();

	virtual bool fromDomElement(QDomElement domElement);
};

class ConfigActionButton : public QPushButton, public ConfigWidget
{
protected:
	virtual void createWidgets();

public:
	ConfigActionButton(const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigActionButton(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigActionButton() {}

	virtual void loadConfiguration() {};
	virtual void saveConfiguration() {};
};

class ConfigSelectFile : public SelectFile, public ConfigWidgetValue
{
protected:
	virtual void createWidgets();

public:
	ConfigSelectFile(const QString &section, const QString &item, const QString &widgetCaption, const QString &type,
		ConfigGroupBox *parentConfigGroupBox,  char *name = 0);
	ConfigSelectFile(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigSelectFile() {}

	virtual void loadConfiguration();
	virtual void saveConfiguration();

	virtual bool fromDomElement(QDomElement domElement);
};

class ConfigPreview : public Preview, public ConfigWidget
{
protected:
	virtual void createWidgets();

public:
	ConfigPreview(const QString &widgetCaption, ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	ConfigPreview(ConfigGroupBox *parentConfigGroupBox, char *name = 0);
	virtual ~ConfigPreview() {}

	virtual void loadConfiguration() {};
	virtual void saveConfiguration() {};
};

#endif // CONFIGURATION_WINDOW_WIDGETS_H