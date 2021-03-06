/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFile>

#include "configuration/configuration-file.h"

#include "debug.h"
#include "gui/windows/message-box.h"
#include "misc/misc.h"

#include "themes.h"

Themes::Themes(const QString &themename, const QString &configname)
	: QObject(), ThemesList(), ThemesPaths(), additional(),
	ConfigName(configname), Name(themename), ActualTheme("Custom"), entries()
{
	setPaths(QStringList());
}

QStringList Themes::getSubDirs(const QString &path, bool validate) const
{
	QDir dir(path);
	dir.setFilter(QDir::Dirs);
	QStringList dirs = dir.entryList();
	dirs.removeAll(".");
	dirs.removeAll("..");

	if (!validate)
		return dirs;

	QStringList subdirs;
	foreach(const QString &dir, dirs)
	{
		QString dirname = path + '/' + dir;
		if (validateDir(dirname))
			subdirs.append(dir);
	}
	return subdirs;
}

bool Themes::validateDir(const QString &path) const
{
	QFile f(path + '/' + ConfigName);
	if (f.exists())
		return true;

	QStringList subdirs = getSubDirs(path, false);
	if (!subdirs.isEmpty())
	{
		foreach(const QString &dir, subdirs)
		{
			f.setFileName(path + '/' + dir + '/' + ConfigName);
			if (!f.exists())
				return false;
		}

		return true;
	}

	return false;
}

const QStringList & Themes::themes() const
{
	return ThemesList;
}

void Themes::setTheme(const QString &theme)
{
	kdebugf();

	if (ThemesList.contains(theme) || (theme == "Custom"))
	{
		entries.clear();
		ActualTheme = theme;
		if (theme != "Custom")
		{
			PlainConfigFile theme_file(
				themePath() +  fixFileName(themePath(), ConfigName));
			entries = theme_file.getGroupSection(Name);
		}
		emit themeChanged(ActualTheme);
	}

	kdebugmf(KDEBUG_FUNCTION_END|KDEBUG_INFO, "end: theme: %s\n", qPrintable(ActualTheme));
}

const QString & Themes::theme() const
{
	return ActualTheme;
}

QString Themes::fixFileName(const QString &path, const QString &fn) const
{
	// check if original path is ok
	if(QFile::exists(path + '/' + fn))
		return fn;
	// maybe all lowercase?
	if(QFile::exists(path + '/' + fn.toLower()))
		return fn.toLower();
	// split for name and extension
	QString name = fn.section('.', 0, 0);
	QString ext = fn.section('.', 1);
	// maybe extension uppercase?
	if(QFile::exists(path + '/' + name + '.' + ext.toUpper()))
		return name + '.' + ext.toUpper();
	// we cannot fix it, return original
	return fn;
}

void Themes::setPaths(const QStringList &paths)
{
	kdebugf();
	ThemesList.clear();
	ThemesPaths.clear();
	additional.clear();
	QStringList temp = paths + defaultKaduPathsWithThemes();
	foreach(const QString &it, temp)
	{
		if (validateDir(it))
		{
			if (paths.indexOf(it) != -1)
				additional.append(it);
			ThemesPaths.append(it);
			ThemesList.append(it.section("/", -1, -1, QString::SectionSkipEmpty));
		}
// TODO: 0.6.5
// 		else
// 			MessageBox::msg(tr("<i>%1</i><br/>does not contain any theme configuration file").arg(it), false, "Warning");
	}
	emit pathsChanged(ThemesPaths);
	kdebugf2();
}

QStringList Themes::defaultKaduPathsWithThemes() const
{
	QStringList result;

	foreach(const QString &it, getSubDirs(dataPath("kadu/themes/" + Name)))
		result << dataPath("kadu/themes/" + Name + '/' + it + '/');

	foreach(const QString &it, getSubDirs(ggPath(Name)))
		result << ggPath(Name) + '/' + it + '/';

	return result;
}

const QStringList & Themes::paths() const
{
    return ThemesPaths;
}

const QStringList & Themes::additionalPaths() const
{
    return additional;
}

QString Themes::themePath(const QString &theme) const
{
	QString t = theme;
	if (theme.isEmpty())
		t = ActualTheme;
	if (t == "Custom")
		return QString::null;
	if (ThemesPaths.isEmpty())
		return "Custom";

	QRegExp r("(/" + t + "/)$");
	foreach (QString theme, ThemesPaths)
		if (-1 != r.indexIn(theme))
			return theme;

	return "Custom";;
}

QString Themes::getThemeEntry(const QString &name) const
{
	if (entries.contains(name))
		return entries[name];
	else
		return QString::null;
}
