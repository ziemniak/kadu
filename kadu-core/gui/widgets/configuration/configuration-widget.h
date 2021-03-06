/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATION_WIDGET_H
#define CONFIGURATION_WIDGET_H

#include <QtGui/QDialog>

#include "configuration/configuration-window-data-manager.h"
#include "icons-manager.h"

class QDomNode;
class QDialogButtonBox;
class QHBoxLayout;
class QListWidget;
class QWidget;
class QVBoxLayout;

class ConfigGroupBox;
class ConfigSection;
class ConfigWidget;

/**
	@class ConfigurationWidget
	@author Vogel
	@short Widget okna konfigruacyjnego.

	Okno konfiguracyjne tego typu zawiera widgety konfiuracyjne podzielone wg. 3 stopnioej hierarchii.
	Pierwszym stopniem są sekcje reprezentowane przez ListWidget'a z lewej strony okna (zawierającego
	ikony i opis tekstowy). Okno zawierające tylko jedną sekcję nie wyświetla ListWidget'a.
	Drugim stopniem są karty reprezentowane przez TabWidget'y, trzecim - grupy opisane przez GroupBox'y.

	Okno konfiguracyjne identyfikuje się przez jego nazwę podaną w konstruktorze
	(dzięki tej nazwie każde okno może osobno zapamiętać swoją pozycję i ostatnio
	otwartej karty).

	Okna mogą teoretycznie zawierać dowolne widgety. Każdy z nich, który dodatkowo
	dziedziczy z klasy ConfigWidget, traktowany jest w specjalny sposób. Jego
	metody loadConfiguration i saveConfiguration są wywoływane automatycznie
	przy otwieraniu okna i przy zapisywaniu konfiguracji, dzięki czemu nie jest
	potrzebne żadne 'ręczne' podpinanie się do tych akcji.

	W momencie zapisania konfiguracji wszystkie obiekty w programie będące instancajmi
	klasy @see ConfigurationAwareObject zostaną o tym poinformowane i będą
	mogły zaktualizować swój stan.

	Widgety w oknie mogą być tworzone na 2 sposoby. Pierwszym z nich jest
	pobranie GroupBoxa za pomocą funkcji @see configGroupBox i dodawanie
	do niego widgetów za pomocą jego funkcji addWidget i addWidgets.
	Drugą jest stworzenie plików XML *.ui, które są wczytywane i usuwane dynamicznie
	z pomocą metod @see appendUiFile i @see removeUiFile.

	W tym drugim przypadku stosuje się pliki *.ui o następującej strukturze:

	&lt;configuration-ui&gt;
		&lt;section caption="tytuł" icon="nazwa_ikony"&gt;
			&lt;tab caption="tytuł"&gt;
				&lt;group-box caption="tytuł" id="id"&gt;
					&lt;widget ... /&gt;
				&lt;/group-box&gt;
			&lt;/tab&gt;
		&lt;/section&gt;
	&lt;/configuration-ui&gt;

	Elementy zawierające atrybut id (nie wymagany) - czyli group-box i dowolny widget
	mogą zostać pobrane przez aplikacje za pomocą metody @see widgetById.
	Widgety z modułów powinny posiadać id w postaci: nazwaModułu/nazwaId.

	Atrybut catpion jest wymagany. Możliwe tagi widget są opisane w dokumentacji
	klas Config* (np.: ConfigComboBox).
 **/

class KADUAPI ConfigurationWidget : public QWidget
{
	Q_OBJECT

	QString Name;

	QWidget *left;
	QMap<QString, ConfigSection *> configSections;
	ConfigSection *currentSection;

	QMap<QString, QWidget *> widgets;

	QListWidget *sectionsListWidget;
	QWidget *container;

	void loadConfiguration(QObject *object);
	void saveConfiguration(QObject *object);

	ConfigSection *configSection(const QString &name);
	ConfigSection *configSection(const QString &icon, const QString &name, bool create = true);

	QList<ConfigWidget *> processUiFile(const QString &fileName, bool append = true);
	QList<ConfigWidget *> processUiSectionFromDom(QDomNode sectionNode, bool append = true);
	QList<ConfigWidget *> processUiTabFromDom(QDomNode tabNode, const QString &iconName, const QString &sectionName, bool append = true);
	QList<ConfigWidget *> processUiGroupBoxFromDom(QDomNode groupBoxNode, const QString &sectionName, const QString &tabName, bool append = true);
	ConfigWidget * appendUiElementFromDom(QDomNode uiElementNode, ConfigGroupBox *configGroupBox);
	void removeUiElementFromDom(QDomNode uiElementNode, ConfigGroupBox *configGroupBox);

private slots:
	void changeSection(const QString &newSectionName);

protected:
	ConfigurationWindowDataManager *dataManager;

public:
	/**
		Tworzy okno konfiguracyjne o danej nazwie. Nazwa wykorzystywana
		jest przy zapamiętywaniu pozycji okna oraz jego ostatnio
		otwartej karty.
	 **/
	ConfigurationWidget(ConfigurationWindowDataManager *dataManager, QWidget *parent = 0);
	virtual ~ConfigurationWidget();

	QString name() { return Name; }

	/**
		Pobiera GroupBox'a zawartego w następującej ścieżce XPath:
		/section[@caption=SECTION]/tab[@caption=TAB]/group-box[@caption=GROUPBOX]

		Jeżeli nie istnieje taki group-box to zostanie on stworzony, gdy create == true.
		Dla crate == false zostanie zwrócony NULL.
	 **/
	ConfigGroupBox * configGroupBox(const QString &section, const QString &tab, const QString &groupBox, bool create = true);

	/**
		Dodaje do okna konfiguracyjnego widgety wczytane z pliku fileName.
		Gdy load == true wczytuje ich wartości z pliku konfiguracyjnego. W przeciwnym
		wypadku wczytanie wartości nastąpi po wywołaniu metody @see show.
	 **/
	QList<ConfigWidget *> appendUiFile(const QString &fileName, bool load = true);
	/**
		Usuwa kontrolki wczytane z danego pliku *.ui.
	 **/
	void removeUiFile(const QString &fileName);

	/**
		Zwraca element opisane id="id" we wczytanych plikach *.ui. Możliwe
		dowolne widgety konfiguracyjne i elementy group-box.
	 **/
	QWidget *widgetById(const QString &id);

	// TODO: make private or sth
	void removedConfigSection(const QString &sectionName);

	void init();

	void loadConfiguration();
	void saveConfiguration();

signals:
	/**
		Sygnał emitowany po naciśnięciu Ok lub Apply ale przed zapisaniem
		wartości do pliku konfiguracyjnego. Nietypowe widgety konfiguracyjne
		powinny się podpiąć pod ten sygnał i po jego uzyskaniu zapisać
		nowe wartości do pliku.
	 **/
	void configurationWindowApplied();

};

#endif
