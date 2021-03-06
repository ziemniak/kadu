/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATION_WINDOW_H
#define CONFIGURATION_WINDOW_H

#include <QtGui/QDialog>

#include "configuration/configuration-window-data-manager.h"
#include "icons-manager.h"

class QWidget;
class ConfigSection;
class ConfigWidget;
class ConfigurationWidget;
class ConfigGroupBox;
class QHBoxLayout;
class QVBoxLayout;
class QDialogButtonBox;

/**
	@class ConfigurationWindow
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

class KADUAPI ConfigurationWindow : public QDialog
{
	Q_OBJECT

	QString Name;

	ConfigurationWidget *configurationWidget;

private slots:
	void updateAndCloseConfig();
	void updateConfig();

protected:
	virtual void keyPressEvent(QKeyEvent *e);

public:
	/**
		Tworzy okno konfiguracyjne o danej nazwie. Nazwa wykorzystywana
		jest przy zapamiętywaniu pozycji okna oraz jego ostatnio
		otwartej karty.
	 **/
	ConfigurationWindow(const QString &name, const QString &caption, ConfigurationWindowDataManager *dataManager);
	virtual ~ConfigurationWindow();

	QString name() { return Name; }
	ConfigurationWidget * widget() { return configurationWidget; }

	/**
		Jeżeli okno jest ukryte wczytuje wartości elementów z pliku
		konfiguracyjnego i pokazuje okno.
	 **/
	virtual void show();

signals:
	/**
		Sygnał emitowany po naciśnięciu Ok lub Apply ale przed zapisaniem
		wartości do pliku konfiguracyjnego. Nietypowe widgety konfiguracyjne
		powinny się podpiąć pod ten sygnał i po jego uzyskaniu zapisać
		nowe wartości do pliku.
	 **/
	void configurationWindowApplied();

	void configurationSaved();

};

#endif
