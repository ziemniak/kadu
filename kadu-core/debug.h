#ifndef DEBUG_H
#define DEBUG_H

#include "exports.h"

/* pierwsze 8 bitow zarezerwowane jest dla libgadu */

/* start funkcji */
#define KDEBUG_FUNCTION_START (1<<8)
/* koniec funkcji */
#define KDEBUG_FUNCTION_END   (1<<9)

/* zwykle inforamcje */
#define KDEBUG_INFO           (1<<10)
/* niegrozne ostrzezenie */
#define KDEBUG_WARNING        (1<<11)
/* powazny blad, ale sytuacja jest do obejscia */
#define KDEBUG_ERROR          (1<<12)
/* blad fatalny, mozliwe ze program sie wywroci */
#define KDEBUG_PANIC          (1<<13)

/* duzo danych do wypisania */
#define KDEBUG_DUMP           (1<<14)
/* informacja dotyczaca obslugi sieci */
#define KDEBUG_NETWORK        (1<<15)

/* 2^31-1 - wszystkie komunikaty */
#define KDEBUG_ALL 2147483647

/*
	<<< kdebug >>>
	Wy�wietla komunikat debuguj�cy na konsoli.
	Sk�adnia jak w printf.
*/
#ifdef DEBUG_ENABLED
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#define kdebug(format, ...) \
	_kdebug_with_mask(KDEBUG_ALL, __FILE__, __LINE__, format, __VA_ARGS__)
#define kdebugm(mask, format, ...) \
	_kdebug_with_mask(mask, __FILE__, __LINE__, format, __VA_ARGS__)
#define kdebugmf(mask, format, ...) \
	_kdebug_with_mask(mask, __FILE__, __LINE__, "%s: " format, __PRETTY_FUNCTION__, __VA_ARGS__)
#else
#define kdebug(format, args...) \
	_kdebug_with_mask(KDEBUG_ALL, __FILE__, __LINE__, format, ##args)
#define kdebugm(mask, format, args...) \
	_kdebug_with_mask(mask, __FILE__, __LINE__, format, ##args)
#define kdebugmf(mask, format, args...) \
	_kdebug_with_mask(mask, __FILE__, __LINE__, "%s: " format, __PRETTY_FUNCTION__, ##args)
#endif // _MSC_VER
#else
#define kdebug(format, ...)
#define kdebugm(mask, format, ...)
#define kdebugmf(mask, format, ...)
#endif

/*
	<<< kdebugf >>>
	Wy�wietla komunikat debuguj�cy zawieraj�cy
	nazw� aktualnie wykonywanej funkcji.
	Z za�o�enia makro to powinno by� wywo�ane
	w pierwszej linii ka�dej funkcji. Dzi�ki
	temu mo�na b�dzie w przysz�o�ci �ledzi�
	dzia�anie programu.
*/	
#define kdebugf() \
	kdebugm(KDEBUG_FUNCTION_START, "%s\n", __PRETTY_FUNCTION__)

#define kdebugf2() \
	kdebugm(KDEBUG_FUNCTION_END, "%s end\n", __PRETTY_FUNCTION__)

/*
	Funkcja pomocnicza. Nie u�ywa�.
*/
#ifdef DEBUG_ENABLED
KADUAPI void _kdebug_with_mask(int mask, const char *file, const int line, const char *format, ...)
#ifndef _MSC_VER
 __attribute__((format (printf, 4, 5)))
#endif
;
#endif
extern KADUAPI int debug_mask;

#endif
