/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>

#include <errno.h>

#include "configuration/xml-configuration-file.h"
#include "misc/misc.h"

#include "debug.h"

#include "configuration-file.h"

QMutex GlobalMutex;

PlainConfigFile::PlainConfigFile(const QString &filename) : filename(filename), groups(), activeGroupName(), activeGroup(0)
{
	read();
}

PlainConfigFile::PlainConfigFile(const PlainConfigFile &c) : filename(c.filename), groups(c.groups), activeGroupName(), activeGroup(0)
{
}

PlainConfigFile &PlainConfigFile::operator=(const PlainConfigFile &c)
{
	filename = c.filename;
	groups = c.groups;
	activeGroupName = QString::null;
	activeGroup = 0;
	return *this;
}

void PlainConfigFile::read()
{
	kdebugmf(KDEBUG_FUNCTION_START, "%s\n", qPrintable(filename));
	QFile file(filename);
	QString line;

	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
		stream.setCodec(codec_latin2);
		while (!stream.atEnd())
		{
			line = stream.readLine().trimmed();
			if (line.startsWith("[") && line.endsWith("]"))
			{
				QString name=line.mid(1, line.length() - 2).trimmed();
				if (activeGroupName!=name)
				{
					activeGroupName=name;
					activeGroup=&groups[name];
				}
			}
			else if (activeGroupName.length())
			{
				QString name = line.section('=', 0, 0);
				QString value = line.right(line.length()-name.length()-1).replace("\\n", "\n");
				name = name.trimmed();

				if (line.contains('=') && !name.isEmpty() && !value.isEmpty())
					(*activeGroup)[name]=value;
			}
		}
		file.close();
	}
	kdebugf2();
}

//#include <sys/time.h>
void PlainConfigFile::write() const
{
	kdebugf();

/*	struct timeval t1,t2;
	gettimeofday(&t1, NULL);
	for(int j=0; j<100; ++j)
	{*/

	QFile file(filename);
	QString line;
	QStringList out;
	QString format1("[%1]\n");
	QString format2("%1=%2\n");

	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		kdebugm(KDEBUG_INFO, "file opened '%s'\n", qPrintable(file.fileName()));
		QTextStream stream(&file);
		stream.setCodec(codec_latin2);
		foreach(const QString &key, groups.keys())
		{
//			kdebugm(KDEBUG_DUMP, ">> %s\n", (i.key()));
			out.append(format1.arg(key));
			foreach(const QString &dataKey, groups[key].keys())
			{
				QString q = groups[key][dataKey];
				out.append(format2.arg(dataKey).arg(q.replace('\n', "\\n")));
//				kdebugm(KDEBUG_DUMP, ">>>>> %s %s\n", qPrintable(key()), qPrintable(q));
			}
			out.append("\n");
		}
		stream << out.join(QString::null);
		file.close();
	}
	else
	{
		fprintf(stderr, "cannot open '%s': %s\n", qPrintable(file.fileName()), qPrintable(file.errorString()));
		fflush(stderr);
	}

/*	}
	gettimeofday(&t2, NULL);
	kdebugm(KDEBUG_INFO, "czas: %ld\n", (t2.tv_usec-t1.tv_usec)+(t2.tv_sec*1000000)-(t1.tv_sec*1000000));
*/
	kdebugf2();
}

QStringList PlainConfigFile::getGroupList() const
{
	return QStringList(groups.keys());
}

void PlainConfigFile::sync() const
{
	write();
}

QMap<QString, QString>& PlainConfigFile::getGroupSection(const QString& name)
{
	kdebugf();
	return groups[name];
}

bool PlainConfigFile::changeEntry(const QString &group, const QString &name, const QString &value)
{
//	kdebugm(KDEBUG_FUNCTION_START, "PlainConfigFile::changeEntry(%s, %s, %s) %p\n", qPrintable(group), qPrintable(name), qPrintable(value), this);
	if (activeGroupName!=group)
	{
		activeGroupName=group;
		activeGroup=&(groups[group]);
	}
	bool ret=activeGroup->contains(name);
	(*activeGroup)[name]=value;
	//
	return ret;
}

QString PlainConfigFile::getEntry(const QString &group, const QString &name, bool *ok) const
{
//	kdebugm(KDEBUG_FUNCTION_START, "PlainConfigFile::getEntry(%s, %s) %p\n", qPrintable(group), qPrintable(name), this);
	if (activeGroupName!=group)
	{
		if (!groups.contains(group))
		{
			if (ok)
				*ok=false;
			return QString::null;
		}
		activeGroupName=group;
		activeGroup=&((QMap<QString, QString>)groups[group]);
	}
	if (ok)
		*ok=activeGroup->contains(name);
	if (activeGroup->contains(name))
		return (*activeGroup)[name];
	else
		return QString::null;
}

void PlainConfigFile::writeEntry(const QString &group, const QString &name, const QVariant &value)
{
    changeEntry(group, name, value.toString());
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QString &value)
{
	changeEntry(group, name, value);
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const char *value)
{
	changeEntry(group, name, QString::fromLocal8Bit(value));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const int value)
{
	changeEntry(group, name, QString::number(value));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const double value)
{
	changeEntry(group, name, QString::number(value, 'f'));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const bool value)
{
	changeEntry(group, name, value ? "true" : "false");
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QRect &value)
{
	changeEntry(group, name, QString("%1,%2,%3,%4").arg(value.left()).arg(value.top()).
				arg(value.width()).arg(value.height()));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QSize &value)
{
	changeEntry(group, name, QString("%1,%2").arg(value.width()).arg(value.height()));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QColor &value)
{
	changeEntry(group, name, value.name());
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QFont &value)
{
	changeEntry(group, name, value.family() + ',' + QString::number(value.pointSize()));
}

void PlainConfigFile::writeEntry(const QString &group,const QString &name, const QPoint &value)
{
	changeEntry(group, name, QString("%1,%2").arg(value.x()).arg(value.y()));
}

template <class T>
T PlainConfigFile::readEntry(const QString &group, const QString &name, const T &def ) const
{
	QVariant string = qVariantFromValue( getEntry(group, name));
	if (string.canConvert<T>())
		return string.value<T>();
	return def;
}

QString PlainConfigFile::readEntry(const QString &group,const QString &name, const QString &def) const
{
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	return string;
}

unsigned int PlainConfigFile::readUnsignedNumEntry(const QString &group,const QString &name, unsigned int def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	unsigned int num = string.toUInt(&ok);
	if (!ok)
		return def;
	return num;
}

int PlainConfigFile::readNumEntry(const QString &group,const QString &name, int def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	int num = string.toInt(&ok);
	if (!ok)
		return def;
	return num;
}

double PlainConfigFile::readDoubleNumEntry(const QString &group,const QString &name, double def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	double num = string.toDouble(&ok);
	if (!ok)
		return def;
	return num;
}

bool PlainConfigFile::readBoolEntry(const QString &group,const QString &name, bool def) const
{
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	return string=="true";
}

QRect PlainConfigFile::readRectEntry(const QString &group,const QString &name, const QRect *def) const
{
	QString string = getEntry(group, name);

	if (string == QString::null)
		return def ? *def : QRect(0, 0, 0, 0);

	return stringToRect(string, def);
}

QSize PlainConfigFile::readSizeEntry(const QString &group,const QString &name, const QSize *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QSize size(0,0);
	int w, h;
	bool ok;

	if (string == QString::null)
		return def ? *def : size;
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 2)
		return def ? *def : size;
	w = stringlist[0].toInt(&ok); if (!ok) return def ? *def : size;
	h = stringlist[1].toInt(&ok); if (!ok) return def ? *def : size;
	size.setWidth(w);
	size.setHeight(h);
	return size;
}

QColor PlainConfigFile::readColorEntry(const QString &group,const QString &name, const QColor *def) const
{
	QColor col(0,0,0);
	QString str = getEntry(group, name);
	if (str==QString::null)
		return def ? *def : col;
	else
	{
		if (!str.contains(','))
			return QColor(str);

		//stary zapis kolor�w, w 0.5.0 mo�na b�dzie wywali�
		bool ok;
		QStringList stringlist = str.split(',', QString::SkipEmptyParts);
		if (stringlist.count() != 3)
			return def ? *def : col;
		int r = stringlist[0].toInt(&ok); if (!ok) return def ? *def : col;
		int g = stringlist[1].toInt(&ok); if (!ok) return def ? *def : col;
		int b = stringlist[2].toInt(&ok); if (!ok) return def ? *def : col;
		col.setRgb(r, g, b);
		return col;
	}
}


QFont PlainConfigFile::readFontEntry(const QString &group,const QString &name, const QFont *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QFont font;
	bool ok;

	if (string == QString::null)
		return def ? *def : QApplication::font();
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() < 2)
		return def ? *def : QApplication::font();
	font.setFamily(stringlist[0]);
	font.setPointSize(stringlist[1].toInt(&ok));
	if (!ok)
		return def ? *def : QApplication::font();
	return font;
}

void PlainConfigFile::removeVariable(const QString &group, const QString &name)
{
	if (activeGroupName != group)
	{
		activeGroupName = group;
		activeGroup = &(groups[group]);
	}

	if (activeGroup->contains(name))
		activeGroup->remove(name);
}

QPoint PlainConfigFile::readPointEntry(const QString &group,const QString &name, const QPoint *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QPoint point(0,0);
	int x, y;
	bool ok;

	if (string == QString::null)
		return def ? *def : point;
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 2)
		return def ? *def : point;
	x = stringlist[0].toInt(&ok); if (!ok) return def ? *def : point;
	y = stringlist[1].toInt(&ok); if (!ok) return def ? *def : point;
	point.setX(x);
	point.setY(y);
	return point;
}

void PlainConfigFile::addVariable(const QString &group, const QString &name, const QString &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const char *defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const int defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const double defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const bool defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const QRect &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const QSize &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const QColor &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const QFont &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void PlainConfigFile::addVariable(const QString &group, const QString &name, const QPoint &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}




ConfigFile::ConfigFile(const QString &filename) : filename(filename)
{
}

void ConfigFile::sync() const
{
	xml_config_file->sync();
}

bool ConfigFile::changeEntry(const QString &group, const QString &name, const QString &value)
{
	GlobalMutex.lock();

//	kdebugm(KDEBUG_FUNCTION_START, "ConfigFile::changeEntry(%s, %s, %s) %p\n", qPrintable(group), qPrintable(name), qPrintable(value), this);
	QDomElement root_elem = xml_config_file->rootElement();
	QDomElement deprecated_elem = xml_config_file->accessElement(root_elem, "Deprecated");
	QDomElement config_file_elem = xml_config_file->accessElementByProperty(
		deprecated_elem, "ConfigFile", "name", filename.section("/", -1));
	QDomElement group_elem = xml_config_file->accessElementByProperty(
		config_file_elem, "Group", "name", group);
	QDomElement entry_elem = xml_config_file->accessElementByProperty(
		group_elem, "Entry", "name", name);
	entry_elem.setAttribute("value", value);

	GlobalMutex.unlock();

	return true;
}

QString ConfigFile::getEntry(const QString &group, const QString &name, bool *ok) const
{
	GlobalMutex.lock();

	bool resOk;
	QString result = QString::null;

//	kdebugm(KDEBUG_FUNCTION_START, "ConfigFile::getEntry(%s, %s) %p\n", qPrintable(group), qPrintable(name), this);
	{
		QDomElement root_elem = xml_config_file->rootElement();
		QDomElement deprecated_elem = xml_config_file->findElement(root_elem, "Deprecated");
		if (!deprecated_elem.isNull())
		{
			QDomElement config_file_elem = xml_config_file->findElementByProperty(
				deprecated_elem, "ConfigFile", "name", filename.section("/", -1));
			if (!config_file_elem.isNull())
			{
				QDomElement group_elem = xml_config_file->findElementByProperty(
					config_file_elem, "Group", "name", group);
				if (!group_elem.isNull())
				{
					QDomElement entry_elem =
						xml_config_file->findElementByProperty(
							group_elem, "Entry", "name", name);
					if (!entry_elem.isNull())
					{
						resOk = true;
						result = entry_elem.attribute("value");
					}
				}
			}
		}
	}

	resOk = false;
	if (ok)
		*ok = resOk;

	GlobalMutex.unlock();
	return result;
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QString &value)
{
	changeEntry(group, name, value);
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const char *value)
{
	changeEntry(group, name, QString::fromLocal8Bit(value));
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const int value)
{
	changeEntry(group, name, QString::number(value));
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const double value)
{
	changeEntry(group, name, QString::number(value, 'f'));
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const bool value)
{
	changeEntry(group, name, value ? "true" : "false");
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QRect &value)
{
	changeEntry(group, name, rectToString(value));
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QSize &value)
{
	changeEntry(group, name, QString("%1,%2").arg(value.width()).arg(value.height()));
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QColor &value)
{
	changeEntry(group, name, value.name());
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QFont &value)
{
	changeEntry(group, name, value.toString());
}

void ConfigFile::writeEntry(const QString &group,const QString &name, const QPoint &value)
{
	changeEntry(group, name, QString("%1,%2").arg(value.x()).arg(value.y()));
}

QString ConfigFile::readEntry(const QString &group,const QString &name, const QString &def) const
{
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	return string;
}

unsigned int ConfigFile::readUnsignedNumEntry(const QString &group,const QString &name, unsigned int def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	unsigned int num = string.toUInt(&ok);
	if (!ok)
		return def;
	return num;
}

int ConfigFile::readNumEntry(const QString &group,const QString &name, int def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	int num = string.toInt(&ok);
	if (!ok)
		return def;
	return num;
}

double ConfigFile::readDoubleNumEntry(const QString &group,const QString &name, double def) const
{
	bool ok;
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	double num = string.toDouble(&ok);
	if (!ok)
		return def;
	return num;
}

bool ConfigFile::readBoolEntry(const QString &group,const QString &name, bool def) const
{
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def;
	return string=="true";
}

QRect ConfigFile::readRectEntry(const QString &group,const QString &name, const QRect *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QRect rect(0,0,0,0);
	int l, t, w, h;
	bool ok;

	if (string == QString::null)
		return def ? *def : rect;
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 4)
		return def ? *def : rect;
	l = stringlist[0].toInt(&ok); if (!ok) return def ? *def : rect;
	t = stringlist[1].toInt(&ok); if (!ok) return def ? *def : rect;
	w = stringlist[2].toInt(&ok); if (!ok) return def ? *def : rect;
	h = stringlist[3].toInt(&ok); if (!ok) return def ? *def : rect;
	rect.setRect(l, t, w, h);
	return rect;
}

QSize ConfigFile::readSizeEntry(const QString &group,const QString &name, const QSize *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QSize size(0,0);
	int w, h;
	bool ok;

	if (string == QString::null)
		return def ? *def : size;
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 2)
		return def ? *def : size;
	w = stringlist[0].toInt(&ok); if (!ok) return def ? *def : size;
	h = stringlist[1].toInt(&ok); if (!ok) return def ? *def : size;
	size.setWidth(w);
	size.setHeight(h);
	return size;
}

QColor ConfigFile::readColorEntry(const QString &group,const QString &name, const QColor *def) const
{
	QColor col(0,0,0);
	QString str = getEntry(group, name);
	if (str==QString::null)
		return def ? *def : col;
	else
	{
		if (!str.contains(','))
			return QColor(str);

		//stary zapis kolor�w, w 0.5.0 mo�na b�dzie wywali�
		bool ok;
		QStringList stringlist = str.split(',', QString::SkipEmptyParts);
		if (stringlist.count() != 3)
			return def ? *def : col;
		int r = stringlist[0].toInt(&ok); if (!ok) return def ? *def : col;
		int g = stringlist[1].toInt(&ok); if (!ok) return def ? *def : col;
		int b = stringlist[2].toInt(&ok); if (!ok) return def ? *def : col;
		col.setRgb(r, g, b);
		return col;
	}
}


QFont ConfigFile::readFontEntry(const QString &group,const QString &name, const QFont *def) const
{
	QString string = getEntry(group, name);
	if (string == QString::null)
		return def ? *def : QApplication::font();
	QFont font;
	if(font.fromString(string))
		return font;
	return def ? *def : QApplication::font();
}

QPoint ConfigFile::readPointEntry(const QString &group,const QString &name, const QPoint *def) const
{
	QString string = getEntry(group, name);
	QStringList stringlist;
	QPoint point(0,0);
	int x, y;
	bool ok;

	if (string == QString::null)
		return def ? *def : point;
	stringlist = string.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 2)
		return def ? *def : point;
	x = stringlist[0].toInt(&ok); if (!ok) return def ? *def : point;
	y = stringlist[1].toInt(&ok); if (!ok) return def ? *def : point;
	point.setX(x);
	point.setY(y);
	return point;
}

void ConfigFile::removeVariable(const QString &group, const QString &name)
{
	GlobalMutex.lock();

	QDomElement root_elem = xml_config_file->rootElement();
	QDomElement deprecated_elem = xml_config_file->accessElement(root_elem, "Deprecated");
	QDomElement config_file_elem = xml_config_file->accessElementByProperty(
		deprecated_elem, "ConfigFile", "name", filename.section("/", -1));
	QDomElement group_elem = xml_config_file->accessElementByProperty(
		config_file_elem, "Group", "name", group);
	QDomElement entry_elem = xml_config_file->accessElementByProperty(
		group_elem, "Entry", "name", name);
	group_elem.removeChild(entry_elem);

	GlobalMutex.unlock();
}

void ConfigFile::addVariable(const QString &group, const QString &name, const QString &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const char *defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const int defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const double defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const bool defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const QRect &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const QSize &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const QColor &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const QFont &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}
void ConfigFile::addVariable(const QString &group, const QString &name, const QPoint &defvalue)
{
	if (getEntry(group, name).isEmpty())
		writeEntry(group,name,defvalue);
}

ConfigFile *config_file_ptr;
