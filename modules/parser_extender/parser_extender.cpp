/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtCore/QDateTime>
#include <QtCore/QString>

#include "parser_extender.h"

#include "debug.h"
#include "modules.h"
#include "parser/parser.h"
#include "configuration/configuration-file.h"
#include "gui/windows/main-configuration-window.h"
#include "misc/path-conversion.h"

QDateTime started;
ParserExtender *parserExtender;

extern "C" int parser_extender_init()
{
	kdebugf();

	parserExtender = new ParserExtender();
	MainConfigurationWindow::registerUiFile(dataPath("kadu/modules/configuration/parser_extender.ui"));

	kdebugf2();
	return 0;
}


extern "C" void parser_extender_close()
{
	kdebugf();

	MainConfigurationWindow::unregisterUiFile(dataPath("kadu/modules/configuration/parser_extender.ui"));
	delete parserExtender;
	parserExtender = 0;

	kdebugf2();
}


/* 
 * Returns uptime counted from the start of the module 
 * @param mode - if 0 number of seconds is returned,
 *               if 1 formatted date is returned
 */
QString getKaduUptime(int mode)
{
	QString uptime = "0";
	
	uptime += "s ";
	if (QDateTime::currentDateTime() > started) 
	{
		int upTime = started.secsTo(QDateTime::currentDateTime());
		if (mode == 0)
		{
			uptime.setNum(upTime);
			uptime += "s ";
		}
		else 
		{
			int days = upTime/(60*60*24);
			upTime -= days * 60*60*24;
			int hours = upTime/(60*60);
			upTime -= hours * 60*60;
			int mins = upTime/(60);
			upTime -= mins * 60;
			int secs = upTime;
			
			QString s;
			uptime = s.setNum(days)+"d ";
			uptime += s.setNum(hours)+"h ";
			uptime += s.setNum(mins)+"m ";
			uptime += s.setNum(secs)+"s ";
		}
	}
	return uptime;
}

/* 
 * Returns system uptime
 * @param mode - if 0 number of seconds is returned,
 *               if 1 formatted date is returned
 */
QString getUptime(int mode)
{	
	QString uptime = "0";
	
	time_t upTime = 0;
	FILE *f;
	double duptime = 0;
	f = fopen("/proc/uptime", "r");
	fscanf(f, "%lf", &duptime);
	fclose(f);
	upTime = (time_t)duptime;
	
	QString s = "";
	if (mode == 0) {
	 	uptime = s.setNum(upTime) + "s ";
	}
	else 
	{
		int days = upTime/(60*60*24);
		upTime -= days * 60*60*24;
		int hours = upTime/(60*60);
		upTime -= hours * 60*60;
		int mins = upTime/(60);
		upTime -= mins * 60;
		int secs = upTime;
			
		uptime = s.setNum(days)+"d ";
		uptime += s.setNum(hours)+"h ";
		uptime += s.setNum(mins)+"m ";
		uptime += s.setNum(secs)+"s ";
	}
	return uptime;
}

/* Returns current time (without secs) */
QString parseTime(const Contact &contact)
{
    return QDateTime::currentDateTime().toString("h:mm");
}

/* Returns current time (with secs) */
QString parseLongTime(const Contact &contact)
{
    return QDateTime::currentDateTime().toString("hh:mm:ss");
}

/* Returns current date (without year) */
QString parseDate(const Contact &contact)
{
	return QDateTime::currentDateTime().toString("dd-MM");
}

/* Returns current date (with year) */
QString parseLongDate(const Contact &contact)
{
	return QDateTime::currentDateTime().toString("dd-MM-yyyy");
}

/* Returns time of module start (without seconds) */
QString parseStartTime(const Contact &contact)
{
	return started.toString("hh:mm");
}

/* Returns time of module start (with seconds) */
QString parseLongStartTime(const Contact &contact)
{
	return  started.toString("dd-MM-yy hh:mm:ss");
}

/* Returns uptime (seconds) */
QString parseUptime(const Contact &contact)
{
	return getUptime(0);
}

/* Returns uptime (formatted) */
QString parseLongUptime(const Contact &contact)
{
	return getUptime(1);
}

/* Returns Kadu uptime */
QString parseKaduUptime(const Contact &contact)
{
	return getKaduUptime(0);
}

/* Returns Kadu uptime (formatted) */
QString parseLongKaduUptime(const Contact &contact)
{
	return getKaduUptime(1);
}

ParserExtender::ParserExtender()
{
	if (config_file.readEntry("PowerKadu", "enable_parser_extender") == "true")
	{
		init();
		isStarted = true;
	}
	else 
		isStarted = false;
}

ParserExtender::~ParserExtender()
{
	if (config_file.readEntry("PowerKadu", "enable_parser_extender") == "true")
		close();
}

void ParserExtender::init()
{
	/* store the date of module start */
	started = QDateTime::currentDateTime();

	/* register tags */	
	Parser::registerTag("time", &parseTime);
	Parser::registerTag("time-long", &parseLongTime);
	Parser::registerTag("date", &parseDate);
	Parser::registerTag("date-long", &parseLongDate);
	Parser::registerTag("start", &parseStartTime);
	Parser::registerTag("start-long", &parseLongStartTime);
	Parser::registerTag("uptime", &parseUptime);
	Parser::registerTag("uptime-long", &parseLongUptime);
	Parser::registerTag("kuptime", &parseKaduUptime);
	Parser::registerTag("kuptime-long", &parseLongKaduUptime);
}

void ParserExtender::close()
{
	/* unregister tags */
	Parser::unregisterTag("time", &parseTime);
	Parser::unregisterTag("time-long", &parseLongTime);
	Parser::unregisterTag("date", &parseDate);
	Parser::unregisterTag("date-long", &parseLongDate);
	Parser::unregisterTag("start", &parseStartTime);
	Parser::unregisterTag("start-long", &parseLongStartTime);
	Parser::unregisterTag("uptime", &parseUptime);
	Parser::unregisterTag("uptime-long", &parseLongUptime);
	Parser::unregisterTag("kuptime", &parseKaduUptime);
	Parser::unregisterTag("kuptime-long", &parseLongKaduUptime);
}

void ParserExtender::configurationUpdated()
{
	if ((config_file.readEntry("PowerKadu", "enable_parser_extender") == "false") && isStarted)
	{
		close();
		isStarted = false;
	}
	else if ((config_file.readEntry("PowerKadu", "enable_parser_extender") == "true") && !isStarted)
	{
		init();
		isStarted = true;
	}
}
