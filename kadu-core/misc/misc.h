#ifndef KADU_MISC_H
#define KADU_MISC_H

#include "base.h"
#include "choose-description.h"
#include "image-widget.h"
#include "token-dialog.h"
#include "create-notifier.h"
#include "image-dialog.h"

/*
	Zmienia �cie�k� relatywn� do katalogu z ustawieniami gg
	na �cie�k� bezwzgledn� uwzgl�dniaj�c zmienne �rodowiskowe
	$HOME i $CONFIG_DIR
*/
KADUAPI QString ggPath(const QString &subpath = QString::null);

/*
	zwraca �cie�k� do pliku f
	je�eli drugi parametr nie jest == 0, to funkcja pr�buje najpierw ustali�
	�cie�k� na podstawie argv0, kt�re ma by� r�wne argv[0] oraz zmiennej PATH
*/
KADUAPI QString dataPath(const QString &f = QString::null, const char *argv0 = 0);

KADUAPI QString libPath(const QString &f = QString::null);

KADUAPI QString cp2unicode(const QByteArray &);
KADUAPI QByteArray unicode2cp(const QString &);
KADUAPI QString latin2unicode(const QByteArray &);
KADUAPI QByteArray unicode2latin(const QString &);
KADUAPI QString unicode2std(const QString &);

// TODO: why not use nromal QUrl::encode ?
//zamienia kodowanie polskich znak�w przekonwertowane z utf-8 przy pomocy QUrl::encode na kodowanie latin-2
KADUAPI QString unicodeUrl2latinUrl(const QString &buf);
//zamienia polskie znaki na format latin2 "url" (czyli do postaci %XY)
KADUAPI QString unicode2latinUrl(const QString &buf);

QString printDateTime(const QDateTime &datetime);
QString timestamp(time_t = 0);
QDateTime currentDateTime();
KADUAPI QString pwHash(const QString &text);
QString translateLanguage(const QApplication *application, const QString &locale, const bool l2n);

KADUAPI void openWebBrowser(const QString &link);
KADUAPI void openMailClient(const QString &mail);
void openGGChat(const QString &gg);

QString versionToName(const unsigned int version);

void stringHeapSort(QStringList &c);
QStringList toStringList(const QString &e1, const QString &e2=QString(), const QString &e3=QString(), const QString &e4=QString(), const QString &e5=QString());

void KADUAPI saveWindowGeometry(const QWidget *w, const QString &section, const QString &name);
void KADUAPI loadWindowGeometry(QWidget *w, const QString &section, const QString &name, int defaultX, int defaultY, int defaultWidth, int defaultHeight);

QRect stringToRect(const QString &value, const QRect *def = NULL);
QString rectToString(const QRect& rect);

//usuwa znaki nowego wiersza, tagi htmla (wszystko co da si� dopasowa� jako <.*>)
QString toPlainText(const QString &text);

void KADUAPI getTime(time_t *sec, int *msec);

extern KADUAPI QFont *defaultFont;
extern KADUAPI QFontInfo *defaultFontInfo;

extern KADUAPI QTextCodec *codec_cp1250;
extern KADUAPI QTextCodec *codec_latin2;

/*
class PixmapPreview : public QLabel, public Q3FilePreview
{
	public:
		PixmapPreview();
		void previewUrl(const Q3Url& url);
};*/

QList<int> toIntList(const QList<QVariant> &in);
QList<QVariant> toVariantList(const QList<int> &in);

/*
	zast�pstwo dla arga w QString, kt�re podmienia kolejne %[1-4] w miejscu

	w QStringu efektem:
		QString("%1 odst�p %2").arg("pierwszy %1 tekst").arg("drugi tekst") jest "pierwszy drugi tekst tekst odst�p %2"
	a chcieliby�my �eby by�o
		"pierwszy %1 tekst odst�p drugi tekst"
	co robi w�a�nie ta funkcja
*/
KADUAPI QString narg(const QString &s, const QString &arg1, const QString &arg2,
				const QString &arg3=QString(), const QString &arg4=QString());

KADUAPI QString narg(const QString &s, const QString &arg1, const QString &arg2,
				const QString &arg3, const QString &arg4,
				const QString &arg5, const QString &arg6=QString(),
				const QString &arg7=QString(),const QString &arg8=QString(),
				const QString &arg9=QString());

/**
	uog�lniony narg(const QString&, const QString &, const QString &, const QString &, const QString &)
	na wi�ksz� liczb� parametr�w
	count musi by� <=9
	tab - tablica count wska�nik�w do QString
**/
KADUAPI QString narg(const QString &s, const QString **tab, int count);

KADUAPI void printBacktrace(const QString &header = QString::null);

// private
extern KADUAPI long int startTime, beforeExecTime, endingTime, exitingTime;
extern KADUAPI bool measureTime;

#endif