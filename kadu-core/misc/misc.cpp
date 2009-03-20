/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "misc.h"

QFont *defaultFont;
QFontInfo *defaultFontInfo;

QTextCodec *codec_cp1250 = QTextCodec::codecForName("CP1250");
QTextCodec *codec_latin2 = QTextCodec::codecForName("ISO8859-2");

long int startTime, beforeExecTime, endingTime, exitingTime;
bool measureTime = false;

void saveWindowGeometry(const QWidget *w, const QString &section, const QString &name)
{
#ifdef Q_OS_MACX
	/* Dorr: on Mac make sure the window will not be greater than desktop */
	config_file.writeEntry(section, name,
		QApplication::desktop()->availableGeometry().intersected(w->geometry()));
#else
	config_file.writeEntry(section, name,w->geometry());
#endif
}

void loadWindowGeometry(QWidget *w, const QString &section, const QString &name, int defaultX, int defaultY, int defaultWidth, int defaultHeight)
{
	QRect rect = config_file.readRectEntry(section, name);
	if ((rect.height() == 0) || (rect.width() == 0))
	{
		rect.setRect(defaultX, defaultY, defaultWidth, defaultHeight);
	}
#ifdef Q_OS_MAC
	if (rect.y() < 20)
		rect.setY(20);
#endif
	w->setGeometry(rect);
}

QString ggPath(const QString &subpath)
{
	static QString path(QString::null);
	if (path == QString::null)
	{
		QString home;
#ifdef Q_OS_WIN
		// on win32 dataPath dont need real argv[0] so it's safe to use this
		// in such ugly way
		if(QFile::exists(dataPath("usbinst", ""))){
			path=dataPath("config/");
			KaduParser::globalVariables["KADU_CONFIG"] = path;
			return (path+subpath);
		}

		WCHAR *homepath=new WCHAR[MAX_PATH+1];
		if(!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL,  0,  homepath))){
			delete homepath;
			homepath=_wgetenv(L"HOMEPATH");
		}
		home=QString::fromUtf16((const ushort*)homepath);

#else
		struct passwd *pw;
		if ((pw = getpwuid(getuid())))
			home = QString::fromLocal8Bit(pw->pw_dir);
		else
			home = QString::fromLocal8Bit(getenv("HOME"));
#endif
		KaduParser::globalVariables["HOME"] = home;
		QString config_dir = QString::fromLocal8Bit(getenv("CONFIG_DIR"));
#ifdef Q_OS_MACX
		if (config_dir.isNull())
			path = QString("%1/Library/Kadu/").arg(home);
		else
			path = QString("%1/%2/Kadu/").arg(home).arg(config_dir);
#elif defined(Q_OS_WIN)
		if (config_dir.isNull())
			path = QString("%1\\Kadu\\").arg(home);
		else
			path = QString("%1\\%2\\Kadu\\").arg(home).arg(config_dir);
#else
		if (config_dir.isNull())
			path = QString("%1/.kadu/").arg(home);
		else
			path = QString("%1/%2/kadu/").arg(home).arg(config_dir);
#endif
		KaduParser::globalVariables["KADU_CONFIG"] = path;
	}

	return (path + subpath);
}

#ifndef Q_WS_WIN
//stat,getcwd
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//getenv
#include <stdlib.h>
//memcpy,strcat,strchr,strerror
#include <string.h>

#include <errno.h>
/*
	sprawdza czy wskazana �cie�ka jest linkiem symbolicznym	i je�eli jest,
	to pr�buje wyci�gn�� �cie�k� na kt�r� wskazuje ten link
	zwraca b��d tylko gdy wyst�pi jaki� b��d przy wywo�ywaniu readlink (co wskazuje na jaki� powa�ny b��d)
	uwaga: je�eli pliku nie ma, to funkcja zwraca _sukces_, bo plik nie jest linkiem
 */
static bool delinkify(char *path, int maxlen)
{
	kdebugmf(KDEBUG_FUNCTION_START, "%s\n", path);
	struct stat st;
	if (lstat(path, &st) == -1)
	{
		kdebugf2();
		return true;
	}
	kdebugm(KDEBUG_INFO, "mode: %o\n", st.st_mode);
	if (!S_ISLNK(st.st_mode))
	{
		kdebugf2();
		return true;
	}
	char *path2 = new char[maxlen];
	ssize_t bytesFilled = readlink(path, path2, maxlen - 1);
	if (bytesFilled == -1)
	{
		fprintf(stderr, "readlink error: '%s'\n", strerror(errno));
		fflush(stderr);
		delete [] path2;
		kdebugf2();
		return false;
	}
	path2[bytesFilled] = 0;
	memcpy(path, path2, bytesFilled + 1);
	delete [] path2;
	kdebugf2();
	return true;
}
#endif


/*
	funkcja poszukuje binarki programu na podstawie argv[0] oraz zmiennej PATH
	je�eli j� znajdzie, to zapisuje �cie�k� pod adres wskazany przez path
	(o maksymalnej d�ugo�ci len) oraz zwraca path, kt�ry zaka�czany jest znakiem '/'
	je�eli binarka nie zostanie znaleziona, to zwracany jest NULL
	w obu przypadkach gwarantowane jest, �e path ko�czy si� znakiem 0
	(len musi by� > 2)
*/
#ifndef Q_OS_WIN
static char *findMe(const char *argv0, char *path, int len)
{
	kdebugf();
	struct stat buf;
	char *lastslash;

	char *current;
	char *previous;
	int l;


	if (argv0[0] == '.' && argv0[1] == '/') //�cie�ka wzgl�dem bie��cego katalogu (./)
	{
		if (getcwd(path, len - 2) == NULL)
		{
			path[0] = 0;
			kdebugf2();
			return NULL;
		}
		strncat(path, argv0 + 1, len - 1);
		path[len - 1] = 0;
		if (!delinkify(path, len))
		{
			kdebugf2();
			return NULL;
		}
		lastslash = strrchr(path, '/');
		lastslash[1] = 0;
		kdebugf2();
		return path;
	}

	if (argv0[0] == '.' && argv0[1] == '.' && argv0[2] == '/') //�cie�ka wzgl�dem bie��cego katalogu (../)
	{
		if (getcwd(path, len - 2)==NULL)
		{
			path[0] = 0;
			kdebugf2();
			return NULL;
		}
		strncat(path, "/", len - 1);
		strncat(path, argv0, len - 1);
		path[len - 1] = 0;
		if (!delinkify(path, len))
		{
			kdebugf2();
			return NULL;
		}
		lastslash = strrchr(path, '/');
		lastslash[1] = 0;
		kdebugf2();
		return path;
	}

	if (argv0[0] == '/') //�cie�ka bezwzgl�dna
	{
		strncpy(path, argv0, len - 1);
		path[len - 1] = 0;
		if (!delinkify(path, len))
		{
			kdebugf2();
			return NULL;
		}
		lastslash = strrchr(path, '/');
		lastslash[1] = 0;
		kdebugf2();
		return path;
	}

	previous = getenv("PATH"); //szukamy we wszystkich katalogach, kt�re s� w PATH
	while((current = strchr(previous, ':')))
	{
		l = current - previous;
		if (l > len - 2)
		{
			path[0] = 0;
			kdebugf2();
			return NULL;
		}

		memcpy(path, previous, l);
		path[l] = '/';
		path[l + 1] = 0;
		strncat(path, argv0, len);
		path[len - 1] = 0;
		if (!delinkify(path, len))
		{
			kdebugf2();
			return NULL;
		}
		if (stat(path, &buf) != -1)
		{
			if (path[l - 1] == '/')
				path[l] = 0;
			else
				path[l + 1] = 0;
			kdebugf2();
			return path;
		}
		previous = current + 1;
	}
	//nie znale�li�my dot�d (bo szukali�my ':'), wi�c mo�e w pozosta�ej cz��ci co� si� znajdzie?
	strncpy(path, previous, len - 2);
	path[len - 2] = 0;

	l = strlen(path);
	path[l] = '/';
	path[l + 1] = 0;
	strncat(path, argv0, len);
	path[len - 1] = 0;
	if (!delinkify(path, len))
	{
		kdebugf2();
		return NULL;
	}
	if (stat(path, &buf) != -1)
	{
		if (path[l - 1] == '/')
			path[l] = 0;
		else
			path[l + 1] = 0;
		kdebugf2();
		return path;
	}
	else
	{
		path[0] = 0;
		kdebugf2();
		return NULL;
	}
}
#endif

static QString lib_path;
static QString data_path;

QString libPath(const QString &f)
{
#ifdef Q_OS_WIN
	QString fp=f;
	if(fp.startsWith("kadu")) fp.remove(0, 4);
	return lib_path + fp;
#else
	return lib_path + f;
#endif
}

QString dataPath(const QString &p, const char *argv0)
{
	QString path=p;

	if (argv0 != 0)
	{
#ifdef Q_OS_MACX
		char cpath[1024];
		if (findMe(argv0, cpath, 1024)==NULL)
		{
			fprintf(stderr, "we've got real problem here ;)\n");
			fflush(stderr);
			exit(10);
		}
		else
		{
			data_path = QString(cpath) + "../../";
			lib_path = QString(cpath) + "../../";
		}
#elif defined(Q_OS_WIN)
		WCHAR epath[MAX_PATH+1];
		GetModuleFileNameW(NULL, epath, MAX_PATH);

		data_path=QString::fromUtf16((const ushort*)epath);
		data_path.resize(data_path.lastIndexOf('\\')+1);
		lib_path=data_path;
#else
		QString datadir(DATADIR);
		QString bindir(BINDIR);
		QString libdir(LIBDIR);

		//je�eli �cie�ki nie ko�cz� si� na /share i /bin oraz gdy bez tych ko�c�wek
		//�cie�ki si� nie pokrywaj�, to znaczy �e kto� ustawi� r�cznie DATADIR lub BINDIR
		if (!datadir.endsWith("/share") || !bindir.endsWith("/bin") || !libdir.endsWith("/lib") ||
			datadir.left(datadir.length() - 6) != bindir.left(bindir.length() - 4) ||
			bindir.left(bindir.length() - 4) != libdir.left(libdir.length() - 4))
		{
			data_path = datadir + '/';
			lib_path = libdir + '/';
		}
		else
		{
			char cpath[1024];
			if (findMe(argv0, cpath, 1024) == NULL)
			{
				data_path = datadir + '/';
				lib_path = libdir + '/';
			}
			else
			{
				data_path = QString(cpath) + "../share/";
				lib_path = QString(cpath) + "../lib/";
			}
		}
#endif
		KaduParser::globalVariables["DATA_PATH"] = data_path;
		KaduParser::globalVariables["LIB_PATH"] = lib_path;
	}
	if (data_path.isEmpty())
	{
		kdebugm(KDEBUG_PANIC, "dataPath() called _BEFORE_ initial dataPath(\"\",argv[0]) (static object uses dataPath()?) !!!\n");
		printBacktrace("dataPath(): constructor of static object uses dataPath");
	}

#ifdef Q_OS_WIN
	// on windows remove kadu from path
	if(path.startsWith("kadu")) path.remove(0, 4);
#endif

	kdebugm(KDEBUG_INFO, "%s%s\n", qPrintable(data_path), qPrintable(path));

	return data_path + path;
}

QString cp2unicode(const QByteArray &buf)
{
	return codec_cp1250->toUnicode(buf);
}

QByteArray unicode2cp(const QString &buf)
{
	return codec_cp1250->fromUnicode(buf);
}

QString latin2unicode(const QByteArray &buf)
{
	return codec_latin2->toUnicode(buf);
}

QByteArray unicode2latin(const QString &buf)
{
	return codec_latin2->fromUnicode(buf);
}

QString unicode2std(const QString &buf)
{
	QString tmp = buf;
	tmp.replace(QChar(0x0119), 'e');
	tmp.replace(QChar(0x00f3), 'o');
	tmp.replace(QChar(0x0105), 'a');
	tmp.replace(QChar(0x015b), 's');
	tmp.replace(QChar(0x0142), 'l');
	tmp.replace(QChar(0x017c), 'z');
	tmp.replace(QChar(0x017a), 'z');
	tmp.replace(QChar(0x0107), 'c');
	tmp.replace(QChar(0x0144), 'n');
	tmp.replace(QChar(0x0118), 'E');
	tmp.replace(QChar(0x00d3), 'O');
	tmp.replace(QChar(0x0104), 'A');
	tmp.replace(QChar(0x015a), 'S');
	tmp.replace(QChar(0x0141), 'L');
	tmp.replace(QChar(0x017b), 'Z');
	tmp.replace(QChar(0x0179), 'Z');
	tmp.replace(QChar(0x0106), 'C');
	tmp.replace(QChar(0x0143), 'N');
	return tmp;
}

QString unicode2latinUrl(const QString &buf)
{
	QString tmp = buf;
	tmp.replace(QChar(0x0119), "%EA");
	tmp.replace(QChar(0x00f3), "%F3");
	tmp.replace(QChar(0x0105), "%B1");
	tmp.replace(QChar(0x015b), "%B6");
	tmp.replace(QChar(0x0142), "%B3");
	tmp.replace(QChar(0x017c), "%BF");
	tmp.replace(QChar(0x017a), "%BC");
	tmp.replace(QChar(0x0107), "%E6");
	tmp.replace(QChar(0x0144), "%F1");
	tmp.replace(QChar(0x0118), "%CA");
	tmp.replace(QChar(0x00d3), "%D3");
	tmp.replace(QChar(0x0104), "%A1");
	tmp.replace(QChar(0x015a), "%A6");
	tmp.replace(QChar(0x0141), "%A3");
	tmp.replace(QChar(0x017b), "%AF");
	tmp.replace(QChar(0x0179), "%AC");
	tmp.replace(QChar(0x0106), "%C3");
	tmp.replace(QChar(0x0143), "%D1");
	return tmp;
}

//looks magically, does it? :D
QString unicodeUrl2latinUrl(const QString &buf)
{
	QString tmp = buf;
	tmp.replace("%C4%99", "%EA"); //�
	tmp.replace("%C3%B3", "%F3"); //�
	tmp.replace("%C4%85", "%B1"); //�
	tmp.replace("%C5%9B", "%B6"); //�
	tmp.replace("%C5%82", "%B3"); //�
	tmp.replace("%C5%BC", "%BF"); //�
	tmp.replace("%C5%BA", "%BC"); //�
	tmp.replace("%C4%87", "%E6"); //�
	tmp.replace("%C5%84", "%F1"); //�
	tmp.replace("%C4%98", "%CA"); //�
	tmp.replace("%C3%93", "%D3"); //�
	tmp.replace("%C4%84", "%A1"); //�
	tmp.replace("%C5%9A", "%A6"); //�
	tmp.replace("%C5%81", "%A3"); //�
	tmp.replace("%C5%BB", "%AF"); //�
	tmp.replace("%C5%B9", "%AC"); //�
	tmp.replace("%C4%86", "%C3"); //�
	tmp.replace("%C5%83", "%D1"); //�
	return tmp;
}

QString printDateTime(const QDateTime &datetime)
{
	QString ret;
	QDateTime current_date;
	unsigned int delta;

	current_date.setTime_t(time(NULL));
//	current_date.setTime(QTime(0, 0));

	delta = datetime.daysTo(current_date);
	ret = datetime.toString("hh:mm:ss");

	if (delta != 0)
	{
		if (config_file.readBoolEntry("Look", "NiceDateFormat"))
		{
			if (delta == 1) // 1 day ago
				ret.prepend(qApp->translate("@default", "Yesterday at "));
			else if (delta < 7) // less than week ago
			{
				ret.prepend(datetime.toString(qApp->translate("@default", "dddd at ")));
				ret[0] = ret[0].toUpper(); // looks ugly lowercase ;)
			}
			else if ((delta > 7) && (delta < 14))
			{
				int tmp = delta % 7;
				if (tmp == 0)
					ret.prepend(qApp->translate("@default", "week ago at "));
				else if (tmp == 1)
					ret.prepend(qApp->translate("@default", "week and day ago at "));
				else
					ret.prepend(qApp->translate("@default", "week and %2 days ago at ").arg(delta%7));
			}
			else if (delta < 6*7)
			{
				int tmp = delta % 7;
				if (tmp == 0)
					ret.prepend(qApp->translate("@default", "%1 weeks ago at ").arg(delta/7));
				else if (tmp == 1)
					ret.prepend(qApp->translate("@default", "%1 weeks and day ago at ").arg(delta/7));
				else
					ret.prepend(qApp->translate("@default", "%1 weeks and %2 days ago at ").arg(delta/7).arg(delta%7));
			}
			else
				ret.prepend(datetime.toString(qApp->translate("@default", "d MMMM yyyy at ")));
		}
		else
			ret.append(datetime.toString(" (dd.MM.yyyy)"));
	}
	return ret;
}

QString timestamp(time_t customtime)
{
	QString buf;
	QDateTime date;
	time_t t;

	t = time(NULL);

	date.setTime_t(t);
	buf.append(printDateTime(date));

	if (customtime)
	{
		date.setTime_t(customtime);
		buf.append(QString(" / S ") + printDateTime(date));
	}

	return buf;
}

QDateTime currentDateTime(void)
{
	time_t t;
	QDateTime date;

	t = time(NULL);
	date.setTime_t(t);
	return date;
}

QStringList toStringList(const QString &e1, const QString &e2, const QString &e3, const QString &e4, const QString &e5)
{
	QStringList list(e1);
	if (e2!=QString::null)
		list<<e2;
	if (e3!=QString::null)
		list<<e3;
	if (e4!=QString::null)
		list<<e4;
	if (e5!=QString::null)
		list<<e5;
	return list;
}

QString pwHash(const QString &text)
{
	QString newText = text;
	for (unsigned int i = 0, textLength = text.length(); i < textLength; ++i)
		newText[i] = QChar(text[i].unicode() ^ i ^ 1);
	return newText;
}

QString translateLanguage(const QApplication *application, const QString &locale, const bool l2n)
{
	static const int langSize = 5;
	static const char local[][3] = {"en",
		"de",
		"fr",
		"it",
		"pl"};

	static const char name[][sizeof("English") /*length of the longest*/] = {
		QT_TR_NOOP("English"),
		QT_TR_NOOP("German"),
		QT_TR_NOOP("French"),
		QT_TR_NOOP("Italian"),
		QT_TR_NOOP("Polish")};

	for (int i = 0; i < langSize; ++i)
	{
		if (l2n)
		{
			if (locale.mid(0, 2) == local[i])
				return application->translate("@default", name[i]);
		}
		else
			if (locale == application->translate("@default", name[i]))
				return local[i];
	}
	if (l2n)
		return application->translate("@default", QT_TR_NOOP("English"));
	else
		return "en";
}

void openWebBrowser(const QString &link)
{
	kdebugf();

	QString webBrowser = config_file.readEntry("Chat", "WebBrowser", QString::null);
	if (webBrowser.isEmpty())
	{
		if(!QDesktopServices::openUrl(QUrl(link))){
			MessageBox::msg(qApp->translate("@default", QT_TR_NOOP("Web browser was not specified. Visit the configuration section")), false, "Warning");
			kdebugmf(KDEBUG_INFO, "Web browser NOT specified.\n");
			return;
		}
		else {
			kdebugf2();
			return;
		}
	}
	if (!webBrowser.contains("%1"))
		webBrowser.append(" \"" + link + '"');
	else
		webBrowser.replace("%1", link);

	QProcess *browser = new QProcess(qApp);
	browser->start(webBrowser);

	if (!browser->waitForStarted())
		MessageBox::msg(qApp->translate("@default", QT_TR_NOOP("Could not spawn Web browser process. Check if the Web browser is functional")), false, "Critical");

	kdebugf2();
}

void openMailClient(const QString &mail)
{
	kdebugf();
	QString email = mail;

	QString mailClient = config_file.readEntry("Chat", "MailClient", QString::null);
	if (mailClient.isEmpty())
	{
		if(!mail.startsWith("mailto:"))
			email="mailto:"+mail;

		if(!QDesktopServices::openUrl(email)){
			MessageBox::msg(qApp->translate("@default", QT_TR_NOOP("Mail client was not specified. Visit the configuration section")), false, "Warning");
			kdebugmf(KDEBUG_INFO, "Mail client NOT specified.\n");
			return;
		}
		else
		{
			kdebugf2();
			return;
		}
	}

	if (email.startsWith("mailto:"))
		email.remove(0, 7); // usuwamy "mailto:", je�li zosta�o dodane jako fragment adresu

	if (mailClient.contains("%1"))
		mailClient.replace("%1", email);
	else
		mailClient.append(email);

	QProcess *mailer = new QProcess(qApp);
	mailer->start(mailClient);

	if (!mailer->waitForStarted())
		MessageBox::msg(qApp->translate("@default", QT_TR_NOOP("Could not spawn Mail client process. Check if the Mail client is functional")), false, "Critical");

	kdebugf2();
}

void openGGChat(const QString &gg)
{
	kdebugf();

	QString gadu = gg;
	if (gadu.startsWith("gg:"))
	{
		gadu.remove(0, 3);
		gadu.remove(QRegExp("/*"));
	}

	Account *account = AccountManager::instance()->defaultAccount();
	ContactList contacts(ContactManager::instance()->byId(account, gadu));
	chat_manager->openPendingMsgs(contacts);

	kdebugf2();
}

QString versionToName(const unsigned int version)
{
	kdebugf();

	QString name;
	switch (version)
	{
		case 0x20: name = "GG 6.0b129"; break;
		case 0x21: name = "GG 6.0b133"; break;
		case 0x22: name = "GG 6.0b140"; break;
		case 0x24: name = "GG 6.1b155/7.6b1359"; break;
		case 0x25: name = "GG 7.0b1"; break;
		case 0x26: name = "GG 7.0b20"; break;
		case 0x27: name = "GG 7.0b22"; break;
		case 0x28: name = "GG 7.5b2201"; break;
		case 0x29: name = "GG 7.6b1688"; break;
		case 0x2a: name = "GG 7.7b3315"; break;
		case 0x2d: name = "GG 8.0b5443"; break;
		default: name = "Unknown"; break;
	}
	return name;

	kdebugf2();
}

//internal usage
static void stringHeapSortPushDown(QString *heap, int first, int last)
{
	int r = first;
	while ( r <= last / 2 ) {
		if ( last == 2 * r ) {
			if ( heap[2 * r].localeAwareCompare(heap[r])<0 )
				qSwap( heap[r], heap[2 * r] );
			r = last;
		} else {
			if ( heap[2 * r].localeAwareCompare( heap[r] )<0 && !(heap[2 * r + 1].localeAwareCompare(heap[2 * r])<0) ) {
				qSwap( heap[r], heap[2 * r] );
				r *= 2;
			} else if ( heap[2 * r + 1].localeAwareCompare( heap[r] )<0 && heap[2 * r + 1].localeAwareCompare( heap[2 * r] )<0 ) {
				qSwap( heap[r], heap[2 * r + 1] );
				r = 2 * r + 1;
			} else {
				r = last;
			}
		}
    }
}

//internal usage
static void stringHeapSortHelper( QStringList::iterator b, QStringList::iterator e, QString, uint n )
{
	QStringList::iterator insert = b;
	QString *realheap = new QString[n];
	QString *heap = realheap - 1;
	int size = 0;
	for( ; insert != e; ++insert ) {
		heap[++size] = *insert;
		int i = size;
		while( i > 1 && heap[i].localeAwareCompare(heap[i / 2])<0 ) {
			qSwap( heap[i], heap[i / 2] );
			i /= 2;
		}
	}

	for( uint i = n; i > 0; --i ) {
		*b++ = heap[1];
		if ( i > 1 ) {
			heap[1] = heap[i];
			stringHeapSortPushDown( heap, 1, (int)i - 1 );
		}
	}

	delete[] realheap;
}

void stringHeapSort(QStringList &c)
{
	if (c.begin() == c.end())
		return;
	stringHeapSortHelper(c.begin(), c.end(), *(c.begin()), (uint)c.count());
}

// -----------------------
//      TokenDialog
// -----------------------

// PixmapPreview::PixmapPreview() : QLabel(NULL)
// {
// }
/*
void PixmapPreview::previewUrl(const Q3Url& url)
{
	QString path = url.path();
	QPixmap pix( path );
	if (pix.isNull())
		setText(qApp->translate("PixmapPreview", "This is not an image"));
	else
	{
		QMatrix mx;
		mx.scale(
			double(width())/double(pix.width()),
			double(height())/double(pix.height()));
		pix = pix.xForm(mx);
		setPixmap(pix);
	}
}*/


QList<int> toIntList(const QList<QVariant> &in)
{
	QList<int> out;
	foreach(const QVariant &it, in)
		out.append(it.toInt());
	return out;
}

QList<QVariant> toVariantList(const QList<int> &in)
{
	QList<QVariant> out;
	foreach(const int &it, in)
		out.append(QVariant(it));
	return out;
}

QRegExp clean_regexp;
QString toPlainText(const QString &text)
{
	kdebugm(KDEBUG_INFO, "rich: %s\n", qPrintable(text));
	if (clean_regexp.isEmpty())
	{
		clean_regexp = QRegExp("<.*>");
		clean_regexp.setMinimal(true);
	}
	QString copy=text;
	copy.replace("\r\n", " ");
	copy.replace("\n",   " ");
	copy.replace("\r",   " ");
	copy.remove(clean_regexp);
	HtmlDocument::unescapeText(copy);
	kdebugm(KDEBUG_INFO, "plain: %s\n", qPrintable(copy));
	return copy;
}

QRect stringToRect(const QString &value, const QRect *def)
{
	QStringList stringlist;
	QRect rect(0,0,0,0);
	int l, t, w, h;
	bool ok;

	stringlist = value.split(',', QString::SkipEmptyParts);
	if (stringlist.count() != 4)
		return def ? *def : rect;
	l = stringlist[0].toInt(&ok); if (!ok) return def ? *def : rect;
	t = stringlist[1].toInt(&ok); if (!ok) return def ? *def : rect;
	w = stringlist[2].toInt(&ok); if (!ok) return def ? *def : rect;
	h = stringlist[3].toInt(&ok); if (!ok) return def ? *def : rect;
	rect.setRect(l, t, w, h);

	return rect;
}

QString rectToString(const QRect& rect)
{
	return QString("%1,%2,%3,%4").arg(rect.left()).arg(rect.top()).arg(rect.width()).arg(rect.height());
}


QString narg(const QString &s, const QString **tab, int count)
{
	kdebugf();
	QString out;
	const QChar *d = s.unicode();
	const QChar *dend = d + s.length();
	int j = 0;
	char maxc = '0' + count;
	if (count > 9)
		return QString::null;

	while (d != dend)
	{
		if (*d == '%' && d + 1 < dend && *(d + 1) >= '1' && *(d + 1) <= maxc)
		{
			out.append(QString(d - j, j));
			++d;
			out.append(*(tab[d->digitValue() - 1]));
			j = 0;
		}
		else
			++j;
		++d;
	}
	out.append(QString(d - j, j));
//	kdebugm(KDEBUG_DUMP, "out: '%s'\n", qPrintable(out));
	kdebugf2();

	return out;
}

QString narg(const QString &s, const QString &arg1, const QString &arg2,
				const QString &arg3, const QString &arg4,
				const QString &arg5, const QString &arg6,
				const QString &arg7, const QString &arg8,
				const QString &arg9)
{
	const QString *tab[9]={&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9};
	return narg(s, tab, 9);
}

QString narg(const QString &s, const QString &arg1, const QString &arg2, const QString &arg3, const QString &arg4)
{
	const QString *tab[4]={&arg1, &arg2, &arg3, &arg4};
	return narg(s, tab, 4);
}

void getTime(time_t *sec, int *msec)
{
#ifdef Q_OS_WIN
	struct _timeb timebuffer;

	_ftime(&timebuffer);

	*sec = timebuffer.time;
	*msec = timebuffer.millitm * 1000;
#else
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	*sec=tv.tv_sec;
	*msec=tv.tv_usec * 1000;
#endif
}

#ifdef HAVE_EXECINFO
#include <execinfo.h>
#endif

void printBacktrace(const QString &header)
{
	if (header.isEmpty())
		fprintf(stderr, "\nbacktrace:\n");
	else
		fprintf(stderr, "\nbacktrace: ('%s')\n", qPrintable(header));
#ifdef HAVE_EXECINFO
	void *bt_array[100];
	char **bt_strings;
	int num_entries;
	if ((num_entries = backtrace(bt_array, 100)) < 0) {
		fprintf(stderr, "could not generate backtrace\n");
		return;
	}
	if ((bt_strings = backtrace_symbols(bt_array, num_entries)) == NULL) {
		fprintf(stderr, "could not get symbol names for backtrace\n");
		return;
	}
	fprintf(stderr, "======= BEGIN OF BACKTRACE =====\n");
	for (int i = 0; i < num_entries; ++i)
		fprintf(stderr, "[%d] %s\n", i, bt_strings[i]);
	fprintf(stderr, "======= END OF BACKTRACE  ======\n");
	free(bt_strings);
#else
	fprintf(stderr, "backtrace not available\n");
#endif
	fflush(stderr);
}