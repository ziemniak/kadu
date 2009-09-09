/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QLayout>
#include <QtGui/QTreeWidget>
#include <QtGui/QPushButton>
#include <QtGui/qsplitter.h>
#include <QtGui/qstatusbar.h>
#include <QtGui/QCloseEvent>
#include <QtGui/QDockWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QGridLayout>
#include <QtGui/QLineEdit>
#include <QtCore/QList>
#include <QtGui/QMenu>
#include <QtGui/QProgressBar>
#include <QtGui/QRadioButton>
#include <QtGui/QTextDocument>


#include "gui/actions/actions.h"
#include "gui/widgets/chat-widget-manager.h"
#include "debug.h"
#include "emoticons.h"
#include "gui/windows/message-box.h"
#include "misc/misc.h"
#include "icons-manager.h"

#include "../../storage/history-storage.h"

#include "history-dialog.h"

MainListItem::MainListItem(QTreeWidget* parent, Chat *chat)
	: QTreeWidgetItem(parent), CurrentChat(chat)
{
	prepareText();	
}

MainListItem::MainListItem(QTreeWidgetItem* parent, Chat *chat)
	: QTreeWidgetItem(parent), CurrentChat(chat)
{
	prepareText();
}

void MainListItem::prepareText()
{
	QString name;
	unsigned int i = 0;
	ContactList contacts = CurrentChat->contacts().toContactList();
	unsigned int count = contacts.count();
	foreach(Contact uid, contacts)
	{
		name.append(uid.display());
		if (i++ < count - 1)
			name.append(", ");
	}
	setText(0, name);
}
 
DetailsListItem::DetailsListItem(QTreeWidget* parent, Chat *chat, QDate date)
	: QTreeWidgetItem(parent), Date(date), CurrentChat(chat)
{
	setText(0, prepareAltnick());
	setText(1, prepareTitle());
 	setText(2, date.toString("dd.MM.yyyy"));
	setText(3, prepareLength());
	setIcon(0, QIcon(IconsManager::instance()->loadIcon("WriteEmail")));
}

QString DetailsListItem::prepareAltnick()
{
	QString name;
	unsigned int i = 0;
	ContactList contacts = CurrentChat->contacts().toContactList();
	unsigned int count = contacts.count();
	foreach(Contact uid, contacts)
	{
		name.append(uid.display());
		if (i++ < count - 1)
			name.append(", ");
	}
	return name;
}

QString DetailsListItem::prepareTitle()
{
	QList<Message> messages = History::instance()->getMessages(CurrentChat, Date, 1);
	if (messages.size() == 0)
		return "";

	Message firstMessage = messages.first();
	ChatMessage first(firstMessage);
	QString title = first.unformattedMessage;

	QTextDocument doc;
	doc.setHtml(title);
	title = doc.toPlainText();
	int l = title.length();
	title.truncate(20);
	if (l > 20)
		title += " ...";
	return title;
}

QString DetailsListItem::prepareLength()
{
	return QString::number(History::instance()->getMessagesCount(CurrentChat, Date));
}

HistoryMainWidget::HistoryMainWidget(QWidget *parent, QWidget *window)
	: MainWindow(parent)
{
	kdebugf();
	QWidget *mvbox = new QWidget;
	QSplitter* right = new QSplitter(Qt::Vertical);
	QVBoxLayout *mvbox_lay = new QVBoxLayout;
	DetailsListView = new QTreeWidget(right);
	QStringList detailsLabels;
	detailsLabels << tr("Contact") << tr("Title") << tr("Date") << tr("Length");
	DetailsListView->setHeaderLabels(detailsLabels);
	DetailsListView->setRootIsDecorated(TRUE);
	ContentBrowser = new ChatMessagesView(0, right);
	ContentBrowser->setPruneEnabled(false);
///	ContentBrowser->setMargin(config_file.readNumEntry("General", "ParagraphSeparator"));

	dock = new QDockWidget(tr("Quick search"), this);
	QWidget *dockWidget = new QWidget;
	QHBoxLayout* dockLayout = new QHBoxLayout;
	dockLayout->setMargin(3);
	dockLayout->setSpacing(3);

	quickSearchPhraseEdit = new QLineEdit(dock);
	connect(quickSearchPhraseEdit, SIGNAL(textChanged(const QString &)), this, SLOT(quickSearchPhraseTyped(const QString &)));
	dockLayout->addWidget(quickSearchPhraseEdit);

	QPushButton *nextButton = new QPushButton(tr("Next"));
	dockLayout->addWidget(nextButton);

	QPushButton *prevButton = new QPushButton(tr("Previous"));
	dockLayout->addWidget(prevButton);

	dockWidget->setLayout(dockLayout);
	dock->setWidget(dockWidget);
	addDockWidget(Qt::BottomDockWidgetArea, dock);
	dock->hide();
/*	
	historySearchActionDescription = new ActionDescription(
		ActionDescription::TypeHistory, "historySearchAction",
		window, SLOT(searchActionActivated(QAction *, bool)),
		"LookupUserInfo", tr("&Search")
	);

	historyNextResultsActionDescription = new ActionDescription(
		ActionDescription::TypeHistory, "historyNextResultsAction",
		window, SLOT(searchNextActActivated(QAction *, bool)),
		"NextPageHistory", tr("&Next")
	);
	historyPrevResultsActionDescription = new ActionDescription(
		ActionDescription::TypeHistory, "historyPrevResultsAction",
		window, SLOT(searchPrevActActivated(QAction *, bool)),
		"PreviousPageHistory", tr("&Previous")
	);
*/


// 	historyAdvSearchActionDescription = new ActionDescription(
// 		ActionDescription::TypeHistory, "historyAdvSearchAction",
// 		window, SLOT(advSearchActivated(QAction *, bool)),
// 		"Configuration", tr("&Advanced search")
// 	);

///	ToolBar::addDefaultAction("SQL History toolbar", "historySearchAction", 1, true);
///	ToolBar::addDefaultAction("SQL History toolbar", "historyNextResultsAction", 2, true);
///	ToolBar::addDefaultAction("SQL History toolbar", "historyPrevResultsAction", 3, true);
// 	ToolBar::addDefaultAction("SQL History toolbar", "historyAdvSearchAction", 4, true);

// 	loadToolBarsFromConfig("sqlHistoryTopDockArea", Qt::TopToolBarArea);
// 	loadToolBarsFromConfig("sqlHistoryBottomDockArea", Qt::BottomToolBarArea);
// 	loadToolBarsFromConfig("sqlHistoryLeftDockArea", Qt::LeftToolBarArea);
// 	loadToolBarsFromConfig("sqlHistoryRightDockArea", Qt::RightToolBarArea);
	
	mvbox_lay->addWidget(right);
	mvbox_lay->setMargin(0);
	mvbox_lay->setSpacing(0);
	mvbox->setLayout(mvbox_lay);
	setCentralWidget(mvbox);
	//loadToolBarsFromConfig("history");
	statusBar()->showMessage(tr("Ready"));
	kdebugf2();
}

HistoryMainWidget::~HistoryMainWidget()
{
	kdebugf();
	//writeToolBarsToConfig("history");
	kdebugf2();
}


bool HistoryMainWidget::supportsActionType(ActionDescription::ActionType type)
{
	return (type == ActionDescription::TypeGlobal || type == ActionDescription::TypeChat || type == ActionDescription::TypeHistory);
}

void HistoryMainWidget::quickSearchPhraseTyped(const QString &text)
{
	kdebugf();
	if(!text.isEmpty())
		if(!ContentBrowser->findText(text))
			if(!ContentBrowser->findText(text, QWebPage::FindBackward))
				//zmienia� kolor t�a edita? jak?
				MessageBox::msg(tr("Nic ciekawego"), false, "Warning");
	kdebugf2();
}

HistoryDlg::HistoryDlg() : QWidget(NULL), isSearchInProgress(0), closeDemand(0), inSearchMode(0), chatsItem(0)
{
	kdebugf();
	setWindowTitle(tr("History"));
	setWindowIcon(IconsManager::instance()->loadIcon("History"));
	QGridLayout* grid = new QGridLayout(this);
	grid->setSpacing(0);
	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	QSplitter* left_splitter = new QSplitter(Qt::Vertical, splitter);
	MainListView = new QTreeWidget(left_splitter);
	QStringList mainLabels;
	mainLabels << "";
	MainListView->setHeaderLabels(mainLabels);
	MainListView->setRootIsDecorated(TRUE);

	QWidget *vbox = new QWidget(splitter);
	QVBoxLayout *vbox_lay = new QVBoxLayout();

	main = new HistoryMainWidget(splitter, this);
	vbox_lay->setMargin(0);
	vbox_lay->setSpacing(0);
	vbox_lay->addWidget(main);

	QList<int> sizes;
	sizes.append(100);
	sizes.append(300);
	splitter->setSizes(sizes);
	vbox->setLayout(vbox_lay);
	grid->addWidget(splitter, 0, 1, 0, 4);
	connect(MainListView, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(mainListItemClicked(QTreeWidgetItem*, int)));

	MainListView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(MainListView, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(showMainPopupMenu(QPoint)));
	main->getDetailsListView()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(main->getDetailsListView(), SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(showDetailsPopupMenu(QPoint)));
	
	connect(main->getDetailsListView(), SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(detailsListItemClicked(QTreeWidgetItem*, int)));
	loadWindowGeometry(this, "History", "HistoryWindowGeometry", 200, 200, 700, 500);
	maxLen = 36;//~	

	MainPopupMenu = new QMenu;
	MainPopupMenu->addAction(IconsManager::instance()->loadIcon("OpenChat"), tr("&Open chat"), this, SLOT(openChat()));
	MainPopupMenu->addAction(IconsManager::instance()->loadIcon("LookupUserInfo"), tr("&Search in directory"), this, SLOT(lookupUserInfo()));
	MainPopupMenu->addAction(IconsManager::instance()->loadIcon("ClearHistory"), tr("&Clear history"), this, SLOT(removeHistoryEntriesPerUser()));
	
	DetailsPopupMenu = new QMenu;
	DetailsPopupMenu->addAction(IconsManager::instance()->loadIcon("ClearHistory"), tr("&Remove entries"), this, SLOT(removeHistoryEntriesPerDate()));
	kdebugf2();
}

HistoryDlg::~HistoryDlg()
{
	// for valgrind
	QStringList searchActions;
	searchActions << "historySearchAction" << "historyNextResultsAction" << "historyPrevResultsAction" << "historyAdvSearchAction";
	foreach(QString act, searchActions)
	{
		ActionDescription *a = KaduActions[act];
		delete a;
	}
	saveWindowGeometry(this, "History", "HistoryDialogGeometry");
}

void HistoryDlg::globalRefresh()
{
	kdebugf();
	MainListView->clear();
	chatsItem = new QTreeWidgetItem(MainListView, QStringList(tr("Chats")));
	chatsItem->setExpanded(true);
	///QTreeWidgetItem *ft = new QTreeWidgetItem(MainListView, QStringList(tr("File transfers")));
	conferItem = new QTreeWidgetItem(MainListView, QStringList(tr("Conferences")));
	smsItem = new QTreeWidgetItem(MainListView, QStringList(tr("SMS")));
	statusItem = new QTreeWidgetItem(MainListView, QStringList(tr("Status")));
	statusItem->setExpanded(false);
	searchItem = new QTreeWidgetItem(MainListView, QStringList(tr("Search")));

	chatsItem->setIcon(0, IconsManager::instance()->loadIcon("OpenChat"));
	///ft->setIcon(0, icons_manager->loadIcon("SendFile"));
	smsItem->setIcon(0, IconsManager::instance()->loadIcon("Mobile"));
	conferItem->setIcon(0, IconsManager::instance()->loadIcon("ManageModules"));
	statusItem->setIcon(0, IconsManager::instance()->loadIcon("Busy"));
	searchItem->setIcon(0, IconsManager::instance()->loadIcon("LookupUserInfo"));

	QList<Chat *> chatsList = History::instance()->chatsList();
	
	MainListItem* mainItem;
	foreach (Chat *chat, chatsList)
	{
		if (chat->contacts().count() > 1)
			mainItem = new MainListItem(conferItem, chat);
		else
			mainItem = new MainListItem(chatsItem, chat);
		mainItem->setIcon(0, IconsManager::instance()->loadIcon("Online"));
	}

	kdebugf2();
}

void HistoryDlg::searchBranchRefresh()
{
	kdebugf();
/*
	//dodaje now� szukan� fraz�
	if(!previousSearchResults.isEmpty())
	{
	QTreeWidgetItem* searchSubItem = new QTreeWidgetItem(searchItem);
	searchSubItem->setText(0, previousSearchResults.last().pattern);
	searchSubItem->setIcon(0, QIcon(IconsManager::instance()->loadIcon("SendMessage")));
	}	
// 	searchItem->removeChild();
// 	foreach(HistorySearchResult result, previousSearchResults)
// // 	{
// // 		QTreeWidgetItem* searchSubItem = new QTreeWidgetItem(searchItem);
// // 		searchSubItem->setText(0, result.pattern);
// // 		searchSubItem->setIcon(0, QIcon(IconsManager::instance()->loadIcon("SendMessage")));		
// 	}
*/
	kdebugf2();
}

void HistoryDlg::showMainPopupMenu(const QPoint &pos)
{
	MainPopupMenu->popup(MainListView->mapToGlobal(pos));
}

void HistoryDlg::showDetailsPopupMenu(const QPoint &pos)
{
	DetailsPopupMenu->popup(main->getDetailsListView()->mapToGlobal(pos));
}

void HistoryDlg::openChat()
{
	kdebugf();
	MainListItem* chatItem = dynamic_cast<MainListItem *>(MainListView->currentItem());
	if (chatItem)
		ChatWidgetManager::instance()->openChatWidget(chatItem->chat(), true);

	kdebugf2();
}

void HistoryDlg::lookupUserInfo()
{
	kdebugf();
	MainListItem* uids_item = dynamic_cast<MainListItem*>(MainListView->currentItem());
	if (uids_item == NULL)
		return;
	//dirty chiaxor, ale na razie to tylko dla gg jest mo�liwe
// 	Contact user = (*uids_item->uidsList().begin());
//   	if (!user.usesProtocol("Gadu"))
// 		return;
// 
// 	SearchDialog *sd = new SearchDialog(kadu, user.ID("Gadu").toUInt());
// 	sd->show();
// 	sd->firstSearch();
	kdebugf2();
}

void HistoryDlg::removeHistoryEntriesPerUser()
{
	kdebugf();

	MainListItem* chatItem = dynamic_cast<MainListItem *>(MainListView->currentItem());
	if (!chatItem)
		return;

	if (MessageBox::ask(tr("You want to remove all history entries of selected users.\nAre you sure?\n"), "Warning"))
	{
		Chat *chat = chatItem->chat();
		if (chat)
			History::instance()->currentStorage()->clearChatHistory(chat);
		MainListView->removeItemWidget((*MainListView->selectedItems().begin()),0);
		globalRefresh();
		main->getDetailsListView()->clear();
		main->getContentBrowser()->clearMessages();
	}

	kdebugf2();
}

void HistoryDlg::removeHistoryEntriesPerDate()
{
	kdebugf();
	MainListItem* uids_item = dynamic_cast<MainListItem*>(MainListView->currentItem());
	if (uids_item == NULL)
		return;
	DetailsListItem* dateItem = dynamic_cast<DetailsListItem*>(main->getDetailsListView()->currentItem());
	if (dateItem == NULL)
		return;
	if (MessageBox::ask(tr("You want to remove history entries of selected users for selected date.\nAre you sure?\n"), "Warning"))
	{
// 		HistoryEntryType typeToRemove;
// 		if(MainListView->currentItem()->parent() == statusItem)
// 			typeToRemove = EntryTypeStatus;
// 		else if(MainListView->currentItem()->parent() == smsItem)
// 			typeToRemove = EntryTypeSms;
// 		else
// 			typeToRemove = EntryTypeMessage;
// 		sql_history->removeHistory(uids_item->uidsList(), dateItem->date(), typeToRemove);
// 		main->getDetailsListView()->removeItemWidget((*main->getDetailsListView()->selectedItems().begin()),0);
// 		globalRefresh();
// 		main->getDetailsListView()->clear();
// 		main->getContentBrowser()->clearMessages();
	}
	kdebugf2();
}

void HistoryDlg::mainListItemClicked(QTreeWidgetItem* item, int column)
{
	kdebugf();
	main->getDetailsListView()->clear();
	MainListItem* mainListItem = dynamic_cast<MainListItem*>(item);

	if (mainListItem == NULL || item == NULL)
		return;
	QList<QDate> chatDates = History::instance()->datesForChat(mainListItem->chat());
	foreach (QDate date, chatDates)
		new DetailsListItem(main->getDetailsListView(), mainListItem->chat(), date);

	QTreeWidgetItem* lastItem = main->getDetailsListView()->itemAt(0, main->getDetailsListView()->topLevelItemCount());
	main->getDetailsListView()->setCurrentItem(lastItem);
	detailsListItemClicked(lastItem, 0);
	main->getDetailsListView()->resizeColumnToContents(0); 
	main->getDetailsListView()->resizeColumnToContents(1);

	kdebugf2();
}

void HistoryDlg::detailsListItemClicked(QTreeWidgetItem* item, int column)
{
	kdebugf();
	DetailsListItem* detailsListItem = dynamic_cast<DetailsListItem*>(item);
	if (detailsListItem == NULL || item == NULL)
		return;

	QList<Message> chat_messages = History::instance()->getMessages(detailsListItem->chat(), detailsListItem->date());
	main->getContentBrowser()->setChat(detailsListItem->chat());
	main->getContentBrowser()->clearMessages();
	main->getContentBrowser()->appendMessages(chat_messages);
	kdebugf2();
}

void HistoryDlg::searchActionActivated(QAction* sender, bool toggled)
{
	kdebugf();	
	advSearchWindow = new HistorySearchDialog(this);	
// 	advSearchWindow->show();
	if (advSearchWindow->exec() == QDialog::Accepted)
	{
		searchHistory();
// 		main->getContentBrowser()->findText(searchParameters.pattern);
	}
	delete advSearchWindow;
	kdebugf2();
}

void HistoryDlg::searchNextActActivated(QAction* sender, bool toggled)
{
	kdebugf();
// 	if(!main->getContentBrowser()->findText(searchParameters.pattern))
// 		MessageBox::msg(tr("There ar no more matches"), false, "Warning");
	kdebugf2();
}

void HistoryDlg::searchPrevActActivated(QAction* sender, bool toggled)
{
	kdebugf();
// 	if(!main->getContentBrowser()->findText(searchParameters.pattern, QWebPage::FindBackward))
// 		MessageBox::msg(tr("There ar no more previous matches"), false, "Warning");
	kdebugf2();
}

void HistoryDlg::setSearchParameters(HistorySearchParameters& params)
{
	searchParameters = params;
}

void HistoryDlg::searchHistory()
{
	kdebugf();
/*	MainListItem* uids_item = dynamic_cast<MainListItem*>(MainListView->currentItem());
	if (uids_item == NULL)
		return;
	if(MainListView->currentItem()->parent() == statusItem)
		searchParameters.currentType = EntryTypeStatus;
	else if(MainListView->currentItem()->parent() == smsItem)
		searchParameters.currentType = EntryTypeSms;
	else
		searchParameters.currentType = EntryTypeMessage;
	main->statusBar()->showMessage(tr("Searching history. Please wait..."));
	main->getDetailsListView()->clear();
	isSearchInProgress = true;
	setEnabled(false);
	HistorySearchResult currentResult;// = sql_history->searchHistory(uids_item->uidsList(), searchParameters);
	if(currentResult.detailsItems.isEmpty())
	{
		main->getDetailsListView()->clear();
		MessageBox::msg(tr("Sorry, nothing found."), false, "Warning");
	}
	else
	foreach(HistorySearchDetailsItem dItem, currentResult.detailsItems)
		new DetailsListItem(main->getDetailsListView(), dItem.altNick, dItem.title, dItem.date, QString::number(dItem.length), uids_item->uidsList());	
	previousSearchResults.append(currentResult);
	isSearchInProgress = false;
	inSearchMode = true;
	main->statusBar()->showMessage(tr("Ready"));
	searchBranchRefresh();
	setEnabled(true);

///TODO: detailsChanged odpala� na pierwszym itemie z detailsTreeWidgeta
	//detailsListItemClicked(main->getDetailsListView()->items().last(), 0);
	main->getContentBrowser()->findText(searchParameters.pattern);*/
	kdebugf2();
}

void HistoryDlg::show(ContactSet users)
{
	if (!History::instance()->currentStorage())
	{
		MessageBox::msg(tr("There is no history storage module loaded!"), false, "Warning");
		return;
	}
	selectedUsers = users;
	globalRefresh();
	QWidget::show();
}

void HistoryDlg::keyPressEvent(QKeyEvent *e)
{
	if (e->matches(QKeySequence::Find))
		if (main->getDockWidget()->isHidden())
			main->getDockWidget()->show();
		else
			main->getDockWidget()->hide();	
}

void HistoryDlg::closeEvent(QCloseEvent *e)
{
	saveWindowGeometry(this, "History", "HistoryWindowGeometry");
	if (isSearchInProgress)
	{
		e->ignore();
		closeDemand = true;
	}
	else
		e->accept();
}

HistorySearchDetailsItem::HistorySearchDetailsItem(QString altNick, QString title, QDate date, int length) : altNick(altNick), date(date), title(title), length(length)
{
}

HistorySearchResult::HistorySearchResult() : detailsItems(QList<HistorySearchDetailsItem>())
{
}


HistoryDlg* historyDialog = NULL;
