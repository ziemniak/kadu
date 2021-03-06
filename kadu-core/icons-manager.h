/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KADU_ICONS_MANAGER_H
#define KADU_ICONS_MANAGER_H

#include <QtCore/QMap>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

#include "configuration/configuration-aware-object.h"

#include "themes.h"
#include "exports.h"

class KADUAPI IconsManager : public Themes, ConfigurationAwareObject
{
	Q_OBJECT
	Q_DISABLE_COPY(IconsManager)

	IconsManager();

	static IconsManager *Instance;

	QMap<QString, QPixmap> pixmaps;
	QMap<QString, QIcon> icons;

public:

	/**
		Zwraca pe�n� �cie�k� do ikony z aktualnego zestawu
		lub bezposrednio name je�li jest to pe�na �cie�ka.
		@param name nazwa ikony z zestawu lub sciezka do pliku
		(jesli zawiera znak '/' to jest interpretowana jako
		sciezka).
	**/
	QString iconPath(const QString &name) const;

	/**
		�aduje ikon� z aktualnego zestawu lub z podanego pliku.
		@param name nazwa ikony z zestawu lub �cie�ka do pliku
		(je�li zawiera znak '/' to jest interpretowana jako
		�cie�ka).
	**/
	const QPixmap & loadPixmap(const QString &name);
	const QIcon & loadIcon(const QString &name);

	QSize getIconsSize();

	static IconsManager * instance();	

public: // TODO: fix, see Kadu::Kadu
	virtual void configurationUpdated();

public slots:
	void clear();

signals:
	void themeChanged();

};

#endif //KADU_ICONS_MANAGER_H
