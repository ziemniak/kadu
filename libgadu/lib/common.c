/* $Id: common.c,v 1.5 2002/09/13 00:30:38 chilek Exp $ */

/*
 *  (C) Copyright 2001-2002 Wojtek Kaniewski <wojtekka@irc.pl>,
 *                          Robert J. Wo�ny <speedy@ziew.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#ifndef _AIX
#  include <string.h>
#endif
#include <stdarg.h>
#ifdef sun
  #include <sys/filio.h>
#endif
#include <fcntl.h>
#include "config.h"
#include "libgadu.h"

#ifndef GG_DEBUG_DISABLE

FILE *gg_debug_file = NULL;

/*
 * gg_debug() // funkcja wewn�trzna
 *
 * wy�wietla komunikat o danym poziomie, o ile u�ytkownik sobie tego �yczy.
 *
 *  - level - poziom wiadomo�ci
 *  - format... - tre�� wiadomo�ci (kompatybilna z printf())
 */
void gg_debug(int level, const char *format, ...)
{
	va_list ap;
	
	if ((gg_debug_level & level)) {
		va_start(ap, format);
		vfprintf((gg_debug_file) ? gg_debug_file : stderr, format, ap);
		va_end(ap);
	}
}

#endif

/*
 * gg_saprintf() // funkcja pomocnicza
 *
 * robi dok�adnie to samo, co sprintf(), tyle �e alokuje sobie wcze�niej
 * miejsce na dane. powinno dzia�a� na tych maszynach, kt�re maj� funkcj�
 * vsnprintf() zgodn� z C99, jak i na wcze�niejszych.
 *
 *  - format... - tre�� taka sama jak w funkcji printf()
 *
 * zaalokowany bufor, kt�ry nale�y p�niej zwolni�, lub NULL
 * je�li nie uda�o si� wykona� zadania.
 */
char *gg_saprintf(const char *format, ...)
{
        va_list ap;
        int size = 0;
	const char *start;
	char *buf = NULL;

	start = format; 
        va_start(ap, format);

#ifndef HAVE_C99_VSNPRINTF
	{
		int res;
		char *tmp;
		
		size = 128;
		do {
			size *= 2;
			if (!(tmp = realloc(buf, size))) {
				free(buf);
				return NULL;
			}
			buf = tmp;
			res = vsnprintf(buf, size, format, ap);
		} while (res == size - 1 || res == -1);
	}
#else
	{
		char tmp[1];
		
		/* libce Solarisa przy buforze NULL zawsze zwracaj� -1, wi�c
		 * musimy poda� co� istniej�cego jako cel printf()owania. */
		size = vsnprintf(tmp, sizeof(tmp), format, ap);
		if (!(buf = malloc(size + 1)))
			return NULL;
	}
#endif

	va_end(ap);
	
	format = start;
	va_start(ap, format);
	
	vsnprintf(buf, size + 1, format, ap);
	
	va_end(ap);

	return buf;
}

/*
 * gg_get_line() // funkcja pomocnicza
 * 
 * podaje kolejn� lini� z bufora tekstowego. niszczy go bezpowrotnie, dziel�c
 * na kolejne stringi. zdarza si�, nie ma potrzeby pisania funkcji dubluj�cej
 * bufor �eby tylko mie� nieruszone dane wej�ciowe, skoro i tak nie b�d� nam
 * po�niej potrzebne. obcina `\r\n'.
 * 
 *  - ptr - wska�nik do zmiennej, kt�ra przechowuje aktualn� pozycj�
 *    w przemiatanym buforze
 * 
 * wska�nik do kolejnej linii tekstu lub NULL, je�li to ju� koniec bufora.
 */
char *gg_get_line(char **ptr)
{
        char *foo, *res;

        if (!ptr || !*ptr || !strcmp(*ptr, ""))
                return NULL;

        res = *ptr;

        if (!(foo = strchr(*ptr, '\n')))
                *ptr += strlen(*ptr);
        else {
                *ptr = foo + 1;
                *foo = 0;
                if (res[strlen(res) - 1] == '\r')
                        res[strlen(res) - 1] = 0;
        }

        return res;
}

/*
 * gg_connect() // funkcja pomocnicza
 *
 * ��czy si� z serwerem. pierwszy argument jest typu (void *), �eby nie
 * musie� niczego inkludowa� w libgadu.h i nie psu� jaki� g�upich zale�no�ci
 * na dziwnych systemach.
 *
 *  - addr - adres serwera (struct in_addr *)
 *  - port - port serwera
 *  - async - asynchroniczne po��czenie
 *
 * deskryptor gniazda lub -1 w przypadku b��du (kod b��du w zmiennej errno).
 */
int gg_connect(void *addr, int port, int async)
{
	int sock, one = 1;
	struct sockaddr_in sin;
	struct in_addr *a = addr;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_connect(%s, %d, %d);\n", inet_ntoa(*a), port, async);
	
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		gg_debug(GG_DEBUG_MISC, "-- socket() failed. errno = %d (%s)\n", errno, strerror(errno));
		return -1;
	}

#ifdef ASSIGN_SOCKETS_TO_THREADS
	gg_thread_socket(0, sock);
#endif

	if (async) {
#ifdef FIONBIO
		if (ioctl(sock, FIONBIO, &one) == -1) {
#else
		if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
#endif
			gg_debug(GG_DEBUG_MISC, "-- ioctl() failed. errno = %d (%s)\n", errno, strerror(errno));
			close(sock);
			return -1;
		}
	}

	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = a->s_addr;
	
	if (connect(sock, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
		if (errno && (!async || errno != EINPROGRESS)) {
			gg_debug(GG_DEBUG_MISC, "-- connect() failed. errno = %d (%s)\n", errno, strerror(errno));
			close(sock);
			return -1;
		}
		gg_debug(GG_DEBUG_MISC, "-- connect() in progress\n");
	}
	
	return sock;
}

/*
 * gg_read_line() // funkcja pomocnicza
 *
 * czyta jedn� lini� tekstu z gniazda
 *
 *  - sock - deskryptor gniazda
 *  - buf - wska�nik do bufora
 *  - length - d�ugo�� bufora
 *
 * je�li trafi na b��d odczytu, zwraca NULL. inaczej zwraca buf.
 */
char *gg_read_line(int sock, char *buf, int length)
{
	int ret;

	for (; length > 1; buf++, length--) {
		do {
			if ((ret = read(sock, buf, 1)) < 1 && errno != EINTR) {
				gg_debug(GG_DEBUG_MISC, "-- gg_read_line(), eof reached\n");
				*buf = 0;
				return NULL;
			}
		} while (ret == -1 && errno == EINTR);

		if (*buf == '\n') {
			buf++;
			break;
		}
	}

	*buf = 0;
	return buf;
}

/*
 * gg_chomp() // funkcja pomocnicza
 *
 * ucina "\r\n" lub "\n" z ko�ca linii.
 *
 *  - line - linia do przyci�cia
 */
void gg_chomp(char *line)
{
	if (!line || strlen(line) < 1)
		return;

	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = 0;
	if (line[strlen(line) - 1] == '\r')
		line[strlen(line) - 1] = 0;
}


/*
 * gg_urlencode() // funkcja wewn�trzna
 *
 * zamienia podany tekst na ci�g znak�w do formularza http. przydaje si�
 * przy r�nych us�ugach katalogu publicznego.
 *
 *  - str - ci�g znak�w do zakodowania
 *
 * zaalokowany bufor, kt�ry nale�y p�niej zwolni� albo NULL
 * w przypadku b��du.
 */
char *gg_urlencode(const char *str)
{
	char *q, *buf, hex[] = "0123456789abcdef";
	const char *p;
	int size = 0;

	if (!str)
		str = strdup("");

	for (p = str; *p; p++, size++) {
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')))
			size += 2;
	}

	if (!(buf = malloc(size + 1)))
		return NULL;

	for (p = str, q = buf; *p; p++, q++) {
		if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))
			*q = *p;
		else {
			*q++ = '%';
			*q++ = hex[*p >> 4 & 15];
			*q = hex[*p & 15];
		}
	}

	*q = 0;

	return buf;
}

/*
 * gg_http_hash() // funkcja wewn�trzna
 *
 * funkcja licz�ca hash dla adresu e-mail, has�a i paru innych.
 *
 *  - format... - format kolejnych parametr�w ('s' je�li dany parametr jest
 *                ci�giem znak�w lub 'u' je�li numerem GG)
 *
 * hash wykorzystywany przy rejestracji i wszelkich manipulacjach w�asnego
 * wpisu w katalogu publicznym.
 */
int gg_http_hash(const char *format, ...)
{
	unsigned int a, c;
	va_list ap;
	int b = -1, i, j;

	va_start(ap, format);

	if (!format)
		return 0;
	
	for (j = 0; j < strlen(format); j++) {
		unsigned char *arg, buf[16];

		if (format[j] == 'u') {
			snprintf(buf, sizeof(buf), "%d", va_arg(ap, uin_t));
			arg = buf;
		} else {
			if (!(arg = va_arg(ap, unsigned char*)))
				arg = "";
		}	

		i = 0;
		while ((c = (int) arg[i++]) != 0) {
			a = (c ^ b) + (c << 8);
			b = (a >> 24) | (a << 8);
		}
	}

	return (b < 0 ? -b : b);
}

/*
 * gg_gethostbyname() // funkcja pomocnicza
 *
 * odpowiednik gethostbyname() u�ywaj�cy gethostbyname_r(), gdy potrzebna
 * jest wielobie�no��. chwilowo korzysta ze zwyk�ego gethostbyname().
 *
 *  - hostname - nazwa serwera
 *
 * zaalokowany bufor, kt�ry nale�y zwolni� lub NULL w przypadku b��du.
 */
struct hostent *gg_gethostbyname(const char *hostname)
{
	/* XXX u�y� gethostbyname_r() */

	struct hostent *hp, *hp2;

	if (!(hp = gethostbyname(hostname)))
		return NULL;

	if (!(hp2 = calloc(1, sizeof(*hp))))
		return NULL;

	memcpy(hp2, hp, sizeof(*hp));

	return hp2;
}

#ifdef ASSIGN_SOCKETS_TO_THREADS

typedef struct gg_thread {
	int id;
	int socket;
	struct gg_thread * next;
} gg_thread;

struct gg_thread * gg_threads = 0;

/*
 * gg_thread_socket() // funkcja pomocnicza, tylko dla win32
 *
 * zwraca deskryptor gniazda, kt�re by�o ostatnio tworzone dla w�tku
 * o podanym identyfikatorze.
 *
 * je�li na win32 przy po��czeniach synchronicznych zapami�tamy w jakim
 * w�tku uruchomili�my funkcj�, kt�ra si� z czymkolwiek ��czy, to z osobnego
 * w�tku mo�emy anulowa� po��czenie poprzez gg_thread_socket(watek,-1);
 * 
 * - thread_id - id w�tku. je�li jest r�wne 0, brany jest aktualny w�tek,
 *               je�li r�wne -1, usuwa wpis o podanym sockecie.
 * - socket - deskryptor gniazda. je�li r�wne 0, zwraca deskryptor gniazda
 *            dla podanego w�tku, je�li r�wne -1, usuwa wpis, je�li co�
 *            innego, ustawia dla podanego w�tku dany numer deskryptora.
 *
 * je�li socket jest r�wne 0, zwraca deskryptor gniazda dla podanego w�tku.
 */
int gg_thread_socket(int thread_id, int socket)
{
	char close = thread_id==-1||socket==-1;
        gg_thread * wsk = gg_threads;
        gg_thread ** p_wsk = &gg_threads;

        if (!thread_id) thread_id = GetCurrentThreadId();
        while (wsk) {
        	if ((thread_id==-1 && wsk->socket==socket)
        	  || wsk->id==thread_id) {
       			if (close) {
                        	closesocket(wsk->socket);
         			*p_wsk=wsk->next;
         			free(wsk);       // socket zostaje usuniety
         			return 1;
                        } else if (!socket) {return wsk->socket; // Socket zostaje zwrocony
                        } else {wsk->socket=socket; return socket;} // Socket zostaje ustawiony
               }
               p_wsk = &(wsk->next);
               wsk = wsk->next;
        }
        if (close && socket!=-1) {closesocket(socket);}
        if (close || !socket) return 0;
        // Dodaje nowy element
        wsk = malloc(sizeof(gg_thread));
        wsk->id = thread_id;
        wsk->socket = socket;
        wsk->next = 0;
        *p_wsk = wsk;
        return socket;
}

#endif /* ASSIGN_SOCKETS_TO_THREADS */


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */
