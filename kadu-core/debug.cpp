/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debug.h"
#include "misc/misc.h"

int debug_mask;

#ifdef DEBUG_ENABLED

#include <QtCore/QMutex>

#include <stdarg.h>
#include <stdio.h>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/*
	Poniewa� debug() mo�e by� u�ywany w r��nych w�tkach,
	wi�c zastosowa�em semafor, aby unikn�� wy�wietlenia
	na ekranie czego� przez inny w�tek pomi�dzy
	poszczeg�lnymi wywo�aniami fprintf
*/
static QMutex debug_mutex;

static int last;
bool KADU_EXPORT showTimesInDebug = false;

void _kdebug_with_mask(int mask, const char* file, const int line, const char* format,...)
{
	if (debug_mask & mask)
	{
		debug_mutex.lock();

		if (showTimesInDebug)
		{
			time_t sec;
			int msec;
			getTime(&sec, &msec);
			int x = (sec % 1000) * 1000 + msec;
			fprintf(stderr, "KK <%d:%03d:%09d:%s:%i>\t", (int)sec, msec, x - last, file, line);
			last = x;
		}
		else
			fprintf(stderr, "KK <%s:%i>\t", file, line);

#ifdef Q_OS_WIN
		HANDLE hConsole=GetStdHandle(STD_ERROR_HANDLE);
		// read old attributes
		CONSOLE_SCREEN_BUFFER_INFO ci;
		GetConsoleScreenBufferInfo(hConsole, &ci);
		if (mask & KDEBUG_WARNING)
			SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);//niebieski
		else if (mask & KDEBUG_ERROR)
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);//���ty
		else if (mask & KDEBUG_PANIC)
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);//jasny czerwony
#else
		if (mask & KDEBUG_WARNING)
			fprintf(stderr, "\033[34m");//niebieski
		else if (mask & KDEBUG_ERROR)
			fprintf(stderr, "\033[33;1m");//���ty
		else if (mask & KDEBUG_PANIC)
			fprintf(stderr, "\033[31;1m");//jasny czerwony
#endif

		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		if (mask & (KDEBUG_PANIC|KDEBUG_ERROR|KDEBUG_WARNING))
#ifdef Q_OS_WIN
			SetConsoleTextAttribute(hConsole, ci.wAttributes);
#else
			fprintf(stderr, "\033[0m");
#endif
		fflush(stderr);
#ifdef Q_OS_WIN
		// ok, now send it to debugger without times
		char msg[1024];
		if(IsDebuggerPresent()){
			_snprintf(msg, 1024, "KK <%s:%i> ", file, line);
			OutputDebugString(msg);
			va_start(args, format);
			vsnprintf(msg, 1021, format, args);
			va_end(args);
			OutputDebugString(msg);
		}
#endif
		debug_mutex.unlock();
	}
}

#endif
