/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "history_dialog.h"

#include "config_file.h"
#include "debug.h"
#include "emoticons.h"
#include "gadu_images_manager.h"
#include "kadu_text_browser.h"
#include "misc.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qvbox.h>

UinsListViewText::UinsListViewText(QListView *parent, const UinsList &uins)
	: QListViewItem(parent), uins(uins)
{
//	kdebugf();
	QString name;

	if (uins.isEmpty())
		setText(0, "SMS");
	else
	{
		uint i = 0, uinsCount = uins.count();
		CONST_FOREACH(uin, uins)
		{
			if (userlist->contains("Gadu", QString::number(*uin)))
				name.append(userlist->byID("Gadu", QString::number(*uin)).altNick());
			else
				name.append(QString::number(*uin));
			if (i++ < uinsCount - 1)
				name.append(",");
		}
		setText(0, name);
	}
//	kdebugf2();
}

const UinsList &UinsListViewText::getUinsList() const
{
	return uins;
}

DateListViewText::DateListViewText(QListViewItem *parent, const HistoryDate &date)
	: QListViewItem(parent), date(date)
{
	setText(0, date.date.toString("yyyy.MM.dd"));
}

const HistoryDate &DateListViewText::getDate() const
{
	return date;
}

HistoryDialog::HistoryDialog(UinsList uins) : QDialog(NULL, "HistoryDialog"), uinslv(0), body(0),
	uins(uins), start(0), findrec(), closeDemand(false), finding(false), dateentries()
{
	kdebugf();
	history->convHist2ekgForm(uins);
	history->buildIndex(uins);

	setCaption(tr("History"));
	setWFlags(Qt::WDestructiveClose);

	QGridLayout *grid = new QGridLayout(this, 2, 5, 3, 3, "grid");

	QSplitter *splitter = new QSplitter(Qt::Horizontal, this, "splitter");

	uinslv = new QListView(splitter, "uinslv");
	uinslv->addColumn(tr("Uins"));
	uinslv->setRootIsDecorated(TRUE);

	QVBox *vbox = new QVBox(splitter, "vbox");
	body = new KaduTextBrowser(vbox, "body");
	body->setReadOnly(true);
	body->QTextEdit::setFont(config_file.readFontEntry("Look", "ChatFont"));
	if((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle")==EMOTS_ANIMATED)
		body->setStyleSheet(new AnimStyleSheet(body, emoticons->themePath()));
	else
		body->setStyleSheet(new StaticStyleSheet(body,emoticons->themePath()));

	body->setMargin(4);

	QHBox *btnbox = new QHBox(vbox, "btnbox");
	QPushButton *searchbtn = new QPushButton(tr("&Find"), btnbox, "searchbtn");
	QPushButton *searchnextbtn = new QPushButton(tr("Find &next"), btnbox, "searcgnextbtn");
	QPushButton *searchprevbtn = new QPushButton(tr("Find &previous"), btnbox, "searchprevbtn");

	QValueList<int> sizes;
	sizes.append(1);
	sizes.append(3);
	splitter->setSizes(sizes);
	grid->addMultiCellWidget(splitter, 0, 1, 0, 4);

	connect(uinslv, SIGNAL(expanded(QListViewItem *)), this, SLOT(uinsChanged(QListViewItem *)));
	connect(uinslv, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(dateChanged(QListViewItem *)));
	connect(searchbtn, SIGNAL(clicked()), this, SLOT(searchBtnClicked()));
	connect(searchnextbtn, SIGNAL(clicked()), this, SLOT(searchNextBtnClicked()));
	connect(searchprevbtn, SIGNAL(clicked()), this, SLOT(searchPrevBtnClicked()));

	loadGeometry(this, "History", "HistoryGeometry", 0, 30, 500, 400);

	findrec.type = 1;
	findrec.reverse = 0;
	findrec.actualrecord = -1;

	UinsListViewText *uinslvt, *selecteduinslvt = NULL;
	QListViewItem *datelvt;

	QValueList<UinsList> uinsentries = history->getUinsLists();

	CONST_FOREACH(uinsentry, uinsentries)
	{
		uinslvt = new UinsListViewText(uinslv, *uinsentry);
		uinslvt->setExpandable(TRUE);
		if ((*uinsentry).equals(uins) && !uins.isEmpty())
			selecteduinslvt = uinslvt;
	}

	uinslv->sort();
	if (selecteduinslvt)
	{
		selecteduinslvt->setOpen(TRUE);
		datelvt = selecteduinslvt->firstChild();
		if (datelvt)
		{
			while (datelvt->nextSibling())
				datelvt = datelvt->nextSibling();
			uinslv->setCurrentItem(datelvt);
			uinslv->setSelected(datelvt, TRUE);
			uinslv->ensureItemVisible(datelvt);
		}
	}
	kdebugf2();
}

void HistoryDialog::uinsChanged(QListViewItem *item)
{
	kdebugf();
	QValueList<HistoryDate> dateentries;
	if (item->depth() == 0)
	{
		uins = ((UinsListViewText *)item)->getUinsList();
		if (!item->childCount())
		{
			dateentries = history->getHistoryDates(uins);
			CONST_FOREACH(dateentry, dateentries)
				(new DateListViewText(item, *dateentry))->setExpandable(FALSE);
		}
	}
	kdebugf2();
}

void HistoryDialog::dateChanged(QListViewItem *item)
{
	kdebugf();
	int count, depth = item->depth();
	switch (depth)
	{
		case 1:
			uinsChanged(item->parent());
			start = ((DateListViewText *)item)->getDate().idx;
			item = item->nextSibling();
			break;
		case 0:
			uinsChanged(item);
			start = 0;
			item = item->firstChild();
			if (item)
				item = item->nextSibling();
			break;
	}
	if (depth < 2)
	{
		if (item)
			count = ((DateListViewText *)item)->getDate().idx - start;
		else
			count = history->getHistoryEntriesCount(uins) - start;
		showHistoryEntries(start, count);
	}
	kdebugf2();
}

void HistoryDialog::formatHistoryEntry(QString &text, const HistoryEntry &entry, QStringList &paracolors)
{
	kdebugf();
	QString bgcolor, textcolor;
	QString message;

	message = entry.message;
	message.replace("\n", "<br/>");

	if (entry.type & (HISTORYMANAGER_ENTRY_CHATSEND | HISTORYMANAGER_ENTRY_MSGSEND
		| HISTORYMANAGER_ENTRY_SMSSEND))
	{
		bgcolor = config_file.readColorEntry("Look","ChatMyBgColor").name();
		textcolor = config_file.readColorEntry("Look","ChatMyFontColor").name();
	}
	else
	{
		bgcolor = config_file.readColorEntry("Look","ChatUsrBgColor").name();
		textcolor = config_file.readColorEntry("Look","ChatUsrFontColor").name();
	}

	const static QString format("<p style=\"background-color: %1\"><img title=\"\" height=\"%3\" width=\"10000\" align=\"right\"><font color=\"%2\"><b>");

	text.append(format.arg(bgcolor).arg(textcolor).arg(4));
	paracolors.append(bgcolor);

	if (entry.type == HISTORYMANAGER_ENTRY_SMSSEND)
		text.append(entry.mobile + " SMS");
	else
	{
		QString nick;
		if (entry.type & (HISTORYMANAGER_ENTRY_CHATSEND | HISTORYMANAGER_ENTRY_MSGSEND))
			nick = config_file.readEntry("General","Nick");
		else
			nick = entry.nick;
		HtmlDocument::escapeText(nick);
		text.append(nick);
	}

	text.append(QString(" :: ") + printDateTime(entry.date));
	if (entry.type & (HISTORYMANAGER_ENTRY_CHATRCV | HISTORYMANAGER_ENTRY_MSGRCV))
		text.append(QString(" / S ") + printDateTime(entry.sdate));
	text.append("</b><br/>");
	if (entry.type & HISTORYMANAGER_ENTRY_STATUS)
	{
		switch (entry.status)
		{
			case GG_STATUS_AVAIL:
			case GG_STATUS_AVAIL_DESCR:
				text.append(tr("Online"));
				break;
			case GG_STATUS_BUSY:
			case GG_STATUS_BUSY_DESCR:
				text.append(tr("Busy"));
				break;
			case GG_STATUS_INVISIBLE:
			case GG_STATUS_INVISIBLE_DESCR:
				text.append(tr("Invisible"));
				break;
			case GG_STATUS_NOT_AVAIL:
			case GG_STATUS_NOT_AVAIL_DESCR:
				text.append(tr("Offline"));
				break;
		}
		if (!entry.description.isEmpty())
			text.append(QString(" (") + entry.description + ")");
		text.append(QString(" ip=") + entry.ip);
	}
	else
	{
		HtmlDocument doc;
		doc.parseHtml(message);
		doc.convertUrlsToHtml();

	 	if((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle")!=EMOTS_NONE && config_file.readBoolEntry("General", "ShowEmotHist"))
	 	{
			body->mimeSourceFactory()->addFilePath(emoticons->themePath());
			emoticons->expandEmoticons(doc, bgcolor);
		}
		GaduImagesManager::setBackgroundsForAnimatedImages(doc, bgcolor);

		text.append(doc.generateHtml());
	}
	text.append("</font></p>");
	kdebugf2();
}

void HistoryDialog::showHistoryEntries(int from, int count)
{
	kdebugf();
	QString text;
	QStringList paracolors;
	unsigned int i;

	bool noStatus = config_file.readBoolEntry("History", "DontShowStatusChanges");

	QValueList<HistoryEntry> entries = history->getHistoryEntries(uins, from, count);

	QValueList<HistoryEntry>::const_iterator entry = entries.constBegin();
	QValueList<HistoryEntry>::const_iterator lastEntry = entries.constEnd();
	for(; entry != lastEntry; ++entry)
		if ( ! (noStatus && (*entry).type & HISTORYMANAGER_ENTRY_STATUS))
		{
			formatHistoryEntry(text, *entry++, paracolors);
			break;
		}
	//z pierwszej wiadomo�ci usuwamy obrazek separatora
	text.remove(QRegExp("<img title=\"\" height=\"[0-9]*\" width=\"10000\" align=\"right\">"));

	for (; entry != lastEntry; ++entry)
		if ( ! (noStatus && (*entry).type & HISTORYMANAGER_ENTRY_STATUS))
			formatHistoryEntry(text, *entry, paracolors);

	body->setText(text);

	i = 0;
	CONST_FOREACH (paracolor, paracolors)
		body->setParagraphBackgroundColor(i++, *paracolor);

	kdebugf2();
}

void HistoryDialog::searchBtnClicked()
{
	kdebugf();

	HistorySearchDialog* hs = new HistorySearchDialog(this, uins);
//	hs->resetBtnClicked();
	hs->setDialogValues(findrec);
	if (hs->exec() == QDialog::Accepted)
	{
		findrec = hs->getDialogValues();
		findrec.actualrecord = -1;
		searchHistory();
	}
	delete hs;
	kdebugf2();
}

void HistoryDialog::searchNextBtnClicked()
{
	kdebugf();
	findrec.reverse = false;
	searchHistory();
	kdebugf2();
}

void HistoryDialog::searchPrevBtnClicked()
{
	kdebugf();
	findrec.reverse = true;
	searchHistory();
	kdebugf2();
}

const QString &HistoryDialog::gaduStatus2symbol(unsigned int status)
{
	static const QString sym[]={QString("avail"), QString("busy"), QString("invisible"), QString("notavail")};
	switch (status)
	{
		case GG_STATUS_AVAIL:
		case GG_STATUS_AVAIL_DESCR:
			return sym[0];
		case GG_STATUS_BUSY:
		case GG_STATUS_BUSY_DESCR:
			return sym[1];
		case GG_STATUS_INVISIBLE:
		case GG_STATUS_INVISIBLE_DESCR:
			return sym[2];
		default:
			return sym[3];
	}
}

void HistoryDialog::setDateListViewText(const QDateTime &datetime)
{
	kdebugf();
	QListViewItem *actlvi;
	actlvi = uinslv->firstChild();
	while (actlvi && !((UinsListViewText *)actlvi)->getUinsList().equals(uins))
		actlvi = actlvi->nextSibling();
	if (actlvi)
	{
		actlvi->setOpen(TRUE);
		actlvi = actlvi->firstChild();
		while (actlvi && ((DateListViewText *)actlvi)->getDate().date.date() != datetime.date())
			actlvi = actlvi->nextSibling();
		if (actlvi)
		{
			uinslv->setCurrentItem(actlvi);
//			body->setSelection(0, 0, 1, 10);
		}
	}
	kdebugf2();
}

void HistoryDialog::searchHistory()
{
	kdebugf();
	int start, end, count, total, len;
	unsigned int i;
	QDateTime fromdate, todate;
	QValueList<HistoryEntry> entries;
	unsigned int entriesCount;
	QRegExp rxp;

	count = history->getHistoryEntriesCount(uins);
	if (findrec.fromdate.isNull())
		start = 0;
	else
		start = history->getHistoryEntryIndexByDate(uins, findrec.fromdate);
	if (findrec.todate.isNull())
		end = count - 1;
	else
		end = history->getHistoryEntryIndexByDate(uins, findrec.todate, true);
	kdebugmf(KDEBUG_INFO, "start = %d, end = %d\n", start, end);
	if (start > end || (start == end && (start == -1 || start == count)))
		return;
	if (start == -1)
		start = 0;
	if (end == count)
		--end;
	entries = history->getHistoryEntries(uins, start, 1);
	fromdate = entries[0].date;
	entries = history->getHistoryEntries(uins, end, 1);
	todate = entries[0].date;
	kdebugmf(KDEBUG_INFO, "start = %s, end = %s\n",
		fromdate.toString("dd.MM.yyyy hh:mm:ss").latin1(),
		todate.toString("dd.MM.yyyy hh:mm:ss").latin1());
	if (findrec.actualrecord == -1)
		findrec.actualrecord = findrec.reverse ? end : start;
	if ((findrec.actualrecord >= end && !findrec.reverse)
		|| (findrec.actualrecord <= start && findrec.reverse))
		return;
	if (findrec.reverse)
		total = findrec.actualrecord - start + 1;
	else
		total = end - findrec.actualrecord + 1;
	kdebugmf(KDEBUG_INFO, "findrec.type = %d\n", findrec.type);
	rxp.setPattern(findrec.data);
	setEnabled(false);
	finding = true;
	if (findrec.reverse)
		do
		{
			len = total > 1000 ? 1000 : total;
			entries = history->getHistoryEntries(uins, findrec.actualrecord - len + 1, len);
			entriesCount = entries.count();
			//ehh, szkoda, �e w Qt nie ma reverse iterator�w...
			QValueList<HistoryEntry>::const_iterator entry = entries.fromLast();
			QValueList<HistoryEntry>::const_iterator firstEntry = entries.begin();
			bool end;
			i = 0;
			do
			{
				if ((findrec.type == 1 &&
					((*entry).type & HISTORYMANAGER_ENTRY_ALL_MSGS)
					&& (*entry).message.contains(rxp)) ||
					(findrec.type == 2 &&
					((*entry).type & HISTORYMANAGER_ENTRY_STATUS)
					&& findrec.data == gaduStatus2symbol((*entry).status)))
				{
					setDateListViewText((*entry).date);
					//showHistoryEntries(findrec.actualrecord - i,
					//	findrec.actualrecord - i + 99 < count ? 100
					//	: count - findrec.actualrecord + i);
					HistoryDialog::start = findrec.actualrecord - i;
					break;
				}
				end = entry == firstEntry;
				if (!end)
					--entry;
				++i;
			}while (!end);
			findrec.actualrecord -= i + (i < entriesCount);
			total -= i + (i < entriesCount);
			kdebugmf(KDEBUG_INFO, "actualrecord = %d, i = %d, total = %d\n",
				findrec.actualrecord, i, total);
			qApp->processEvents();
		} while (total > 0 && i == entriesCount && !closeDemand);
	else
		do
		{
			len = total > 1000 ? 1000 : total;
			entries = history->getHistoryEntries(uins, findrec.actualrecord, len);
			entriesCount = entries.count();
			i = 0;
			CONST_FOREACH(entry, entries)
			{
				if ((findrec.type == 1 && ((*entry).type & HISTORYMANAGER_ENTRY_ALL_MSGS)
					&& (*entry).message.contains(rxp)) ||
					(findrec.type == 2 &&
					((*entry).type & HISTORYMANAGER_ENTRY_STATUS) &&
					findrec.data == gaduStatus2symbol((*entry).status)))
				{
					setDateListViewText((*entry).date);
					//showHistoryEntries(findrec.actualrecord + i,
					//	findrec.actualrecord + 99 < count ? 100
					//	: count - findrec.actualrecord - i);
					HistoryDialog::start = findrec.actualrecord + i;
					break;
				}
				++i;
			}
			findrec.actualrecord += i + (i < entriesCount);
			total -= i + (i < entriesCount);
			kdebugmf(KDEBUG_INFO, "actualrecord = %d, i = %d, total = %d\n",
				findrec.actualrecord, i, total);
			qApp->processEvents();
		} while (total > 0 && i == entriesCount && !closeDemand);
	if (closeDemand)
	{
		reject();
		kdebugf2();
		return;
	}
	if (findrec.actualrecord < 0)
		findrec.actualrecord = 0;
	setEnabled(true);
	finding = false;
	kdebugf2();
}

void HistoryDialog::closeEvent(QCloseEvent *e)
{
	saveGeometry(this, "History", "HistoryGeometry");

	if (finding)
	{
		e->ignore();
		closeDemand = true;
	}
	else
		e->accept();
}