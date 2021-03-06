/***************************************************************************
*                                                                         *
*    This program is free software; you can redistribute it and/or modify *
*    it under the terms of the GNU General Public License as published by *
*    the Free Software Foundation; either version 2 of the License, or    *
*    (at your option) any later version.                                  *
*                                                                         *
* **************************************************************************/

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include "word_fix.h"

#include "chat_manager-old.h"
#include "custom_input.h"
#include "debug.h"
#include "html_document.h"
#include "usergroup.h"
#include "misc/misc.h"
#include "config_file.h"

WordFix *wordFix;

extern "C" int word_fix_init()
{
	kdebugf();
	wordFix = new WordFix();
	MainConfigurationWindow::registerUiFile(dataPath("kadu/modules/configuration/word_fix.ui"), wordFix);
	kdebugf2();
	return 0;
}


extern "C" void word_fix_close()
{
	kdebugf();
	MainConfigurationWindow::unregisterUiFile(dataPath("kadu/modules/configuration/word_fix.ui"), wordFix);
	delete wordFix;
	wordFix = NULL;
	kdebugf2();
}


WordFix::WordFix()
{
	kdebugf();

	connect(chat_manager, SIGNAL(chatWidgetCreated(ChatWidget *, time_t)), this, SLOT(chatCreated(ChatWidget *, time_t)));
	connect(chat_manager, SIGNAL(chatWidgetDestroying(ChatWidget *)), this, SLOT(chatDestroying(ChatWidget *)));

	for (uint i = 0; i < chat_manager->chats().count(); i++)
		connectToChat(chat_manager->chats()[i]);

	// Loading list
	QString data = config_file.readEntry("word_fix", "WordFix_list", "");
	QStringList list = QStringList::split("\t\t", data);
	if (!list.count())
	{
		QFile defList(dataPath("kadu/modules/data/word_fix/wf_default_list.data"));
		if (defList.open(IO_ReadOnly))
		{
			QTextStream s(&defList);
			QStringList pair;
			while (!s.atEnd())
			{
				pair = QStringList::split('|', s.readLine());
				if (pair.count() <= 0)
					continue;

				wordsList[pair[0]] = pair[1];
			}
			defList.close();
		}
		else
		{
			kdebug(QString("Can't open file: %1").arg(defList.name()));
		}
	}
	else
	{
		for ( uint i = 0; i < list.count(); i++ )
		{
			QStringList sp = QStringList::split('\t', list[i]);
			wordsList[sp[0]] = sp[1];
		}
	}

	kdebugf2();
}

WordFix::~WordFix()
{
	kdebugf();
	disconnect(chat_manager, SIGNAL(chatWidgetCreated(ChatWidget *, time_t)), this, SLOT(chatCreated(ChatWidget *, time_t)));
	disconnect(chat_manager, SIGNAL(chatWidgetDestroying(ChatWidget *)), this, SLOT(chatDestroying(ChatWidget *)));

	for ( uint i = 0; i < chat_manager->chats().count(); i++ )
		disconnectFromChat(chat_manager->chats()[i]);

	kdebugf2();
}

void WordFix::chatCreated(ChatWidget *chat, time_t time)
{
	kdebugf();
	connectToChat(chat);
	kdebugf2();
}

void WordFix::chatDestroying(ChatWidget *chat)
{
	kdebugf();
	disconnectFromChat(chat);
	kdebugf2();
}

void WordFix::connectToChat(const ChatWidget *chat)
{
	kdebugf();
	connect(chat, SIGNAL(messageSendRequested(ChatWidget*)), this, SLOT(sendRequest(ChatWidget*)));
	kdebugf2();
}

void WordFix::disconnectFromChat(const ChatWidget *chat)
{
	kdebugf();
	disconnect(chat, SIGNAL(messageSendRequested(ChatWidget*)), this, SLOT(sendRequest(ChatWidget*)));
	kdebugf2();
}

void WordFix::sendRequest(ChatWidget* chat)
{
	kdebugf();

	if (!config_file.readBoolEntry("PowerKadu", "enable_word_fix", false))
		return;

	// Reading chat input to html document.
	HtmlDocument doc;
	doc.parseHtml(chat->edit()->text());

	// Parsing and replacing.
	for (int i = 0; i < doc.countElements(); i++)
	{
		if (!doc.isTagElement(i))
			doReplace(doc.elementText(i));
	}

	// Putting back corrected text.
	chat->edit()->setText(doc.generateHtml());

	kdebugf2();
}

void WordFix::doReplace(QString &text)
{
	kdebugf();

	// Make text as " "+text+" ", to match words at the start and end of text.
	QString txt = text;
	txt = " "+txt+" ";

	// Replacing
	QString key;
	for (uint i = 0; i < wordsList.keys().count(); i++)
	{
		key = wordsList.keys()[i];
		txt.replace(
				QRegExp(
						"[\\s\\!\\.\\,\\(\\)\\[\\]\\;\\:\\\"\\']{1}"
						+key
						+"[\\s\\!\\.\\,\\(\\)\\[\\]\\;\\:\\\"\\']{1}"
					),
				" "+wordsList[key]+" "
			);
	}

	// Cutting off " " from start and end.
	txt = txt.mid(1, txt.length()-2);
	text = txt;
	kdebugf2();
}

void WordFix::wordSelected()
{
	kdebugf();

	QList<QTreeWidgetItem *> items = list->selectedItems();
	QTreeWidgetItem* item = NULL;

	if (items.isEmpty())
	{
		changeButton->setEnabled(false);
		deleteButton->setEnabled(false);
		return;
	}
	else
	{
		changeButton->setEnabled(true);
		deleteButton->setEnabled(true);
	}

	item = items[0];
	wordEdit->setText(item->text(0));
	valueEdit->setText(item->text(1));

	kdebugf2();
}

void WordFix::changeSelected()
{
	kdebugf();

	QTreeWidgetItem* item = list->currentItem();
	if (!item)
		return;

	QString wordStr = wordEdit->text(),
			valueStr = valueEdit->text();

	item->setText(0, wordStr);
	item->setText(1, valueStr);

	wordEdit->setText("");
	valueEdit->setText("");

	changeButton->setEnabled(false);
	deleteButton->setEnabled(false);
	kdebugf2();
}

void WordFix::deleteSelected()
{
	kdebugf();

	QTreeWidgetItem* item = list->currentItem();
	if (!item)
		return;

	QString wordStr = item->text(0);
	delete item;

	wordEdit->setText("");
	valueEdit->setText("");

	changeButton->setEnabled(false);
	deleteButton->setEnabled(false);
	list->setCurrentItem(0);

	kdebugf2();
}

void WordFix::addNew()
{
	kdebugf();

	QString wordStr = wordEdit->text(),
			valueStr = valueEdit->text();

	if (wordStr.isEmpty())
		return;

	if (list->findItems(wordStr, 0, Qt::MatchExactly).isEmpty())
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(list);
		item->setText(0, wordStr);
		item->setText(1, valueStr);
		list->sortItems(0, Qt::AscendingOrder);
		list->setCurrentItem(item);
	}
	
	wordEdit->setText("");
	valueEdit->setText("");

	changeButton->setEnabled(false);
	deleteButton->setEnabled(false);

	kdebugf2();
}

void WordFix::moveToNewValue()
{
	kdebugf();

	// sprawdz czy podane slowo znajduje sie na liscie i jesli tak to umozliw edycje
	QList<QTreeWidgetItem *> items = list->findItems(wordEdit->text(), 0, Qt::MatchExactly);
	if (!items.isEmpty()) {
		QTreeWidgetItem *item = items[0];
		list->setCurrentItem(item);
		valueEdit->setText(item->text(1));
		changeButton->setEnabled(true);
		deleteButton->setEnabled(true);
		addButton->setEnabled(false);
	} else {
		changeButton->setEnabled(false);
		deleteButton->setEnabled(false);
		addButton->setEnabled(true);
	}

	valueEdit->setFocus();
	kdebugf2();
}

void WordFix::mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow)
{
	kdebugf();

	ConfigGroupBox *groupBox = mainConfigurationWindow->configGroupBox("Chat", "Words fix", "Words fix");

	QWidget *w = new QWidget(groupBox->widget());
	
	QGridLayout *layout = new QGridLayout(w);
	layout->setSpacing(5);
	layout->setMargin(5);

	list = new QTreeWidget(w);
	layout->addWidget(list, 0, 1);
	layout->addMultiCellWidget(list, 0, 0, 0, 1);

	wordEdit = new QLineEdit(w);
	layout->addWidget(new QLabel(tr("A word to be replaced"), w), 1, 0);
	layout->addWidget(wordEdit, 1, 1);
	
	valueEdit = new QLineEdit(w);
	layout->addWidget(new QLabel(tr("Value to replace with"), w), 2, 0);
	layout->addWidget(valueEdit, 2, 1);

	QWidget *hbox = new QWidget(w);
	addButton = new QPushButton(tr("Add"), hbox);
	changeButton = new QPushButton(tr("Change"), hbox);
	deleteButton = new QPushButton(tr("Delete"), hbox);
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(addButton);
	hlayout->addWidget(changeButton);
	hlayout->addWidget(deleteButton);
	hbox->setLayout(hlayout);
	layout->addWidget(hbox, 3, 1);

	groupBox->addWidgets(0, w);

	connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(wordSelected()));
	connect(changeButton, SIGNAL(clicked()), this, SLOT(changeSelected()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteSelected()));
	connect(addButton, SIGNAL(clicked()), this, SLOT(addNew()));
	connect(wordEdit, SIGNAL(returnPressed()), this, SLOT(moveToNewValue()));
	connect(valueEdit, SIGNAL(returnPressed()), this, SLOT(addNew()));

	changeButton->setEnabled(false);
	deleteButton->setEnabled(false);

	list->setAllColumnsShowFocus(true);
	list->setColumnCount(2);

	QStringList headers;
	headers << tr("Word") << tr("Replace with");
	list->setHeaderLabels(headers);

	list->setColumnWidth(0, 250);
	list->setColumnWidth(1, 246 - list->verticalScrollBar()->width());

	QTreeWidgetItem* item;
	QList<QTreeWidgetItem *> items;
	for (uint i = 0; i < wordsList.keys().count(); i++)
	{
		QString wordStr = wordsList.keys()[i];
		item = new QTreeWidgetItem( list );
		item->setText( 0, wordStr );
		item->setText( 1, wordsList[wordStr]);
		items.append(item);
	}
	list->insertTopLevelItems(0, items);

	connect(mainConfigurationWindow, SIGNAL(configurationWindowApplied()), this, SLOT(configurationApplied()));

	kdebugf2();
}

void WordFix::configurationApplied()
{
	kdebugf();

	wordsList.clear();
	QTreeWidgetItem* item = list->itemAt(0, 0);
	QString wordStr = item->text(0),
			valueStr = item->text(1);

	wordsList[wordStr] = valueStr;

	while ((item = list->itemBelow(item)))
	{
		wordStr = item->text(0);
		valueStr = item->text(1);
		wordsList[wordStr] = valueStr;
	}

	saveList();

	kdebugf2();
}

void WordFix::saveList()
{
	kdebugf();
	
	QStringList list;
	for ( uint i = 0; i < wordsList.keys().count(); i++ )
	{
		QString word = wordsList.keys()[i];
		list.append(word+"\t"+wordsList[word]);
	}
	config_file.writeEntry("word_fix", "WordFix_list", list.join("\t\t"));

	kdebugf2();
}
