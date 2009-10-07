/*****************************************************************************
	Adium time formatter
	This class is based on ChatWindowStyle and WeekDate classes from qutIM
	instant	messenger (see: http://www.qutim.org/)

	Copyright (c) 2008-2009 by Rustam Chakin
		      2008-2009 by Nigmatullin Ruslan

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#include <QtCore/QLocale>

#include "os/generic/system-info.h"

#include "adium-time-formatter.h"

void WeekDate::setDate(const QDate &date)
{
	m_week = date.weekNumber(&m_year);
	m_day = date.dayOfWeek();
}

AdiumTimeFormatter::AdiumTimeFormatter(QObject *parent) : QObject(parent)
{
}

inline void AdiumTimeFormatter::appendStr(QString &str, const QString &res, int length)
{
	length -= res.length();
	while (length-- > 0)
		str += QChar(' ');
	str += res;
}

void AdiumTimeFormatter::appendInt(QString &str, int number, int length)
{
	int n = number;
	length--;
	while (n /= 10)
		length--;

	while (length-- > 0)
		str += QChar('0');
	str += QString::number(number);
}

#define TRIM_LENGTH(NUM) \
	while (length > NUM) \
	{ \
		finishStr(str, week_date, date, time, c, NUM); \
		length -= NUM; \
	}

// http://unicode.org/reports/tr35/tr35-6.html#Date_Format_Patterns
void AdiumTimeFormatter::finishStr(QString &str, const WeekDate &week_date, const QDate &date, const QTime &time, QChar c, int length)
{
	if (length <= 0)
		return;

	switch (c.unicode())
	{
		case L'G':
		{
			bool ad = date.year() > 0;
			if (length < 4)
				str += ad ? "AD" : "BC";
			else if (length == 4)
				str += ad ? "Anno Domini" : "Before Christ";
			else
				str += ad ? "A" : "B";
			break;
		}
		case L'y':
			if (length == 2)
				appendInt(str, date.year() % 100, 2);
			else
				appendInt(str, date.year(), length);
			break;
		case L'Y':
			appendInt(str, week_date.year(), length);
			break;
		case L'u':
		{
			int year = date.year();
			if (year < 0)
				year++;
			appendInt(str, date.year(), length);
			break;
		}
		case L'q':
		case L'Q':
		{
			int q = (date.month() + 2) / 3;
			if (length < 3)
				appendInt(str, q, length);
			else if (length == 3)
			{
				str += "Q";
				str += QString::number(q);
			}
			else
			{
				switch (q)
				{
					case 1:
						str += tr("1st quarter");
						break;
					case 2:
						str += tr("2nd quarter");
						break;
					case 3:
						str += tr("3rd quarter");
						break;
					case 4:
						str += tr("4th quarter");
						break;
					default:
						break;
				}
			}
			break;
		}
		case L'M':
		case L'L':
			if (length < 3)
				appendInt(str, date.month(), length);
			else if (length == 3)
				str += QDate::shortMonthName(date.month());
			else if (length == 4)
				str += QDate::longMonthName(date.month());
			else
				str += QDate::shortMonthName(date.month()).at(0);
			break;
		case L'w':
			TRIM_LENGTH(2);
			appendInt(str, length, week_date.week());
			break;
		case L'W':
			while (length-- > 0)
				str += QString::number((date.day() + 6) / 7);
			break;
		case L'd':
			TRIM_LENGTH(2);
			appendInt(str, date.day(), length);
			break;
		case L'D':
			TRIM_LENGTH(3);
			appendInt(str, date.dayOfYear(), length);
			break;
		case L'F':
			while (length-- > 0)
				str += QString::number(1);
			break;
		case L'g':
			appendInt(str, date.toJulianDay(), length);
			break;
		case L'c':
		case L'e':
			if (length < 3)
			{
				appendInt(str, date.dayOfWeek(), length);
				break;
			}
		case L'E':
			if (length < 4)
				str += QDate::shortDayName(date.dayOfWeek());
			else if (length == 4)
				str += QDate::longDayName(date.dayOfWeek());
			else
				str += QDate::shortDayName(date.dayOfWeek()).at(0);
			break;
		case L'a':
			str += time.hour() < 12 ? "AM" : "PM";
			break;
		case L'H':
			TRIM_LENGTH(2);
			appendInt(str, time.hour(), length);
			break;
		case L'h':
			TRIM_LENGTH(2);
			appendInt(str, time.hour() % 12, length);
			break;
		case L'K':
			TRIM_LENGTH(2);
			appendInt(str, time.hour() - 1, length);
			break;
		case L'k':
			TRIM_LENGTH(2);
			appendInt(str, time.hour() % 12 - 1, length);
			break;
		case L'm':
			TRIM_LENGTH(2);
			appendInt(str, time.minute(), length);
			break;
		case L's':
			TRIM_LENGTH(2);
			appendInt(str, time.second(), length);
			break;
		case L'S':
			str += QString::number(time.msec() / 1000.0, 'f', length).section('.', 1);
			break;
		case L'A':
			appendInt(str, QTime(0,0).msecsTo(time), length);
			break;
		case L'v':
			// I don't understand the difference
		case L'z':
			if (length < 4)
				str += SystemInfo::instance()->timezone();
			else
				// There should be localized name, but I don't know how get it
				str += SystemInfo::instance()->timezone();
			break;
		case L'Z':
		{
			if (length == 4)
				str += "GMT";
			int offset = SystemInfo::instance()->timezoneOffset();
			if (offset < 0)
				str += '+';
			else
				str += '-';
			appendInt(str, qAbs((offset/60)*100 + offset%60), 4);
			break;
		}
		default:
			while (length-- > 0)
				str += c;
			break;
	}
}

QString AdiumTimeFormatter::convertTimeDate(const QString &mac_format, const QDateTime &datetime)
{
	QDate date = datetime.date();
	QTime time = datetime.time();
	QString str;
	if (mac_format.contains('%'))
	{
		const QChar *chars = mac_format.constData();
		bool is_percent = false;
		int length = 0;
		bool error = false;
		while ((*chars).unicode() && !error)
		{
			if (is_percent)
			{
				is_percent = false;
				switch ((*chars).unicode())
				{
					case L'%':
						str += *chars;
						break;
					case L'a':
						appendStr(str, QDate::shortDayName(date.dayOfWeek()), length);
						break;
					case L'A':
						appendStr(str, QDate::longDayName(date.dayOfWeek()), length);
						break;
					case L'b':
						appendStr(str, QDate::shortMonthName(date.day()), length);
						break;
					case L'B':
						appendStr(str, QDate::longMonthName(date.day()), length);
						break;
					case L'c':
						appendStr(str, QLocale::system().toString(datetime), length);
						break;
					case L'd':
						appendInt(str, date.day(), length > 0 ? length : 2);
						break;
					case L'e':
						appendInt(str, date.day(), length);
						break;
					case L'F':
						appendInt(str, time.msec(), length > 0 ? length : 3);
						break;
					case L'H':
						appendInt(str, time.hour(), length > 0 ? length : 2);
						break;
					case L'I':
						appendInt(str, time.hour() % 12, length > 0 ? length : 2);
						break;
					case L'j':
						appendInt(str, date.dayOfYear(), length > 0 ? length : 3);
						break;
					case L'm':
						appendInt(str, date.month(), length > 0 ? length : 2);
						break;
					case L'M':
						appendInt(str, time.minute(), length > 0 ? length : 2);
						break;
					case L'p':
						appendStr(str, time.hour() < 12 ? "AM" : "PM", length);
						break;
					case L'S':
						appendInt(str, time.second(), length > 0 ? length : 2);
						break;
					case L'w':
						appendInt(str, date.dayOfWeek(), length);
						break;
					case L'x':
						appendStr(str, QLocale::system().toString(date), length);
						break;
					case L'X':
						appendStr(str, QLocale::system().toString(time), length);
						break;
					case L'y':
						appendInt(str, date.year() % 100, length > 0 ? length : 2);
						break;
					case L'Y':
						appendInt(str, date.year(), length > 0 ? length : 4);
						break;
					case L'Z':
						// It should be localized, isn't it?..
						appendStr(str, SystemInfo::instance()->timezone(), length);
						break;
					case L'z':
					{
						int offset = SystemInfo::instance()->timezoneOffset();
						appendInt(str, (offset/60)*100 + offset%60, length > 0 ? length : 4);
						break;
					}
					default:
						if ((*chars).isDigit())
						{
							is_percent = true;
							length *= 10;
							length += (*chars).digitValue();
						}
						else
							error = true;
				}
			}
			else if (*chars == '%')
			{
				length = 0;
				is_percent = true;
			}
			else
				str += *chars;
			chars++;
		}
		if (!error)
			return str;

		str = QString();
	}

	WeekDate week_date(date);
	QChar last;
	QChar cur;
	int length = 0;
	bool quote = false;
	const QChar *chars = mac_format.constData();
	forever
	{
		cur = *chars;
		if (cur == '\'')
		{
			if (*(chars+1) == '\'')
			{
				chars++;
				str += cur;
			}
			else
			{
				if (!quote)
					finishStr(str, week_date, date, time, last, length);
				quote = !quote;
			}
			length = 0;
		}
		else if (quote)
			str += cur;
		else
		{
			if (cur == last)
				length++;
			else
			{
				finishStr(str, week_date, date, time, last, length);
				length = 1;
			}
		}
		if (!chars->unicode())
			break;

		last = cur;
		chars++;
	}
	return str;
}
