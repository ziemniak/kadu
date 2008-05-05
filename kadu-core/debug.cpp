/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debug.h"

int debug_mask;

#ifdef DEBUG_ENABLED

#include <QMutex>

#include <sys/time.h>

#include <stdarg.h>
#include <stdio.h>

/*
	Poniewa� debug() mo�e by� u�ywany w r�nych w�tkach,
	wi�c zastosowa�em semafor, aby unikn�� wy�wietlenia
	na ekranie czego� przez inny w�tek pomi�dzy
	poszczeg�lnymi wywo�aniami fprintf
*/
static QMutex debug_mutex;

static int last;
static struct timeval tv;
static struct timezone tz;
bool showTimesInDebug = false;

void _kdebug_with_mask(int mask, const char* file, const int line, const char* format,...)
{
	if (debug_mask & mask)
	{
		debug_mutex.lock();

		if (showTimesInDebug)
		{
			gettimeofday(&tv, &tz);
			int x = (tv.tv_sec % 1000) * 1000000 + tv.tv_usec;
			fprintf(stderr, "KK <%ld:%06ld:%09d:%s:%i>\t", tv.tv_sec, tv.tv_usec, x - last, file, line);
			last = x;
		}
		else
			fprintf(stderr, "KK <%s:%i>\t", file, line);

		if (mask & KDEBUG_WARNING)
			fprintf(stderr, "\033[34m");//niebieski
		else if (mask & KDEBUG_ERROR)
			fprintf(stderr, "\033[33;1m");//��ty
		else if (mask & KDEBUG_PANIC)
			fprintf(stderr, "\033[31;1m");//jasny czerwony

		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		if (mask & (KDEBUG_PANIC|KDEBUG_ERROR|KDEBUG_WARNING))
			fprintf(stderr, "\033[0m");
		fflush(stderr);
		debug_mutex.unlock();
	}
}

#endif
