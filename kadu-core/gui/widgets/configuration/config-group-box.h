/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIG_GROUP_BOX_H
#define CONFIG_GROUP_BOX_H

#include "misc/misc.h"

class ConfigTab;
class QGridLayout;
class QWidget;
class QGroupBox;
class QLayout;

/**
	@class ConfigGroupBox
	@author Vogel
	@short GroupBox w oknie konfiguracji

	GroupBox w oknie konfiguracji definiowany jest przez następujący tag:
	<code>
		&lt;group-box caption="tytuł" id="id"&gr;
			...
		&lt;/group-box&gt;
	</code>

	GroupBox może znajdować się tylko wewnątrz tagu tab. W jego wnętrzu
	mogą zawierać się dowolne tagi widgetów konfigruacyjnych.

	Dodatkowo, GroupBox'a można stworzyć (lub, jeżeli istnieje, uzyskać)
	wywołując funkcję configGroupBox(section, tab, groupBox) z okna konfiguracyjnego.
	Do tak uzyskanego GroupBox'a można dodawać dowolne widgety (@see addWidget,
	@see addWidgets).
 **/

class KADUAPI ConfigGroupBox
{
	QString name;
	ConfigTab *configTab;

	QGroupBox *groupBox;
	QWidget *container;
	QGridLayout *gridLayout;

public:
	ConfigGroupBox(const QString &name, ConfigTab *configTab, QGroupBox *groupBox);
	~ConfigGroupBox();

	QWidget * widget() { return container; }

	/**
		Dodaje widget do GroupBoxa.
		@param widget dodawany widget (nie może być NULL)
		@param fullSpace kiedy true, dodany widget zajmuje całą szerokość GroupBox'a,
			w przeciwnym wypadku tylko prawą jego część
	 **/
	void addWidget(QWidget *widget, bool fullSpace = false);
	/**
		Dodaje 2 widget do GroupBoxa, jeden po lewej stronie, drugi po prawej.
		@param widget1 widget dodawany z lewej strony (zazwyczaj etykieta)
		@param widget2 widget dodawany z prawej strony
	 **/
	void addWidgets(QWidget *widget1, QWidget *widget2);

	void insertWidget(int pos, QWidget *widget, bool fullSpace = false);

	void insertWidgets(int pos, QWidget *widget1, QWidget *widget2);

	/**
		Zwraca true, gdy GroupBox nie posiada w sobie żadnych widgetów.
		@return true, gdy GroupBox nie posiada w sobie żadnych widgetów
	 **/
	bool empty();

};

#endif
