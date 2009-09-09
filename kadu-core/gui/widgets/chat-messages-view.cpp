/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>
#include <QtWebKit/QWebFrame>

#include "accounts/account.h"
#include "accounts/account-manager.h"
#include "configuration/configuration-file.h"
#include "chat/chat.h"
#include "chat/chat-message.h"
#include "chat/chat-styles-manager.h"
#include "chat/html-messages-renderer.h"
#include "chat/style-engines/chat-style-engine.h"
#include "protocols/services/chat-image-service.h"

#include "debug.h"

#include "chat-messages-view.h"

ChatMessagesView::ChatMessagesView(Chat *chat, QWidget *parent) : KaduTextBrowser(parent),
	LastScrollValue(0), LastLine(false), CurrentChat(chat)
{
	Renderer = new HtmlMessagesRenderer(CurrentChat, this);

	// TODO: for me with empty styleSheet if has artifacts on scrollbars...
	// maybe Qt bug?
	setStyleSheet("QWidget { }");
	setFocusPolicy(Qt::NoFocus);
	setMinimumSize(QSize(100,100));
	setPage(Renderer->webPage());
	settings()->setAttribute(QWebSettings::JavascriptEnabled, true);

	connectChat();
	connect(this, SIGNAL(loadFinished(bool)), this, SLOT(scrollToLine()));

	ChatStylesManager::instance()->chatViewCreated(this);
}

ChatMessagesView::~ChatMessagesView()
{
 	ChatStylesManager::instance()->chatViewDestroyed(this);

	disconnectChat();
}

void ChatMessagesView::connectChat()
{
	if (!CurrentChat || !CurrentChat->account())
		return;

	ChatImageService *chatImageService = CurrentChat->account()->protocol()->chatImageService();
	if (chatImageService)
		connect(chatImageService, SIGNAL(imageReceived(const QString &, const QString &)),
				this, SLOT(imageReceived(const QString &, const QString &)));
}

void ChatMessagesView::disconnectChat()
{
	if (!CurrentChat || !CurrentChat->account())
		return;
	
	ChatImageService *chatImageService = CurrentChat->account()->protocol()->chatImageService();
	if (chatImageService)
		disconnect(chatImageService, SIGNAL(imageReceived(const QString &, const QString &)),
				this, SLOT(imageReceived(const QString &, const QString &)));
}

void ChatMessagesView::setChat(Chat *chat)
{
	disconnectChat();
	CurrentChat = chat;
	connectChat();

	Renderer->setChat(CurrentChat);
}

void ChatMessagesView::setPruneEnabled(bool enable)
{
	Renderer->setPruneEnabled(enable);
}

void ChatMessagesView::pageUp()
{
	QKeyEvent event(QEvent::KeyPress, 0x01000016, Qt::NoModifier);
	keyPressEvent(&event);
}

void ChatMessagesView::pageDown()
{
	QKeyEvent event(QEvent::KeyPress, 0x01000017, Qt::NoModifier);
	keyPressEvent(&event);
}

void ChatMessagesView::imageReceived(const QString &messageId, const QString &messagePath)
{
	rememberScrollBarPosition();
	Renderer->replaceLoadingImages(messageId, messagePath);
}

void ChatMessagesView::updateBackgroundsAndColors()
{
	Renderer->updateBackgroundsAndColors();
}

void ChatMessagesView::repaintMessages()
{
	rememberScrollBarPosition();
	Renderer->refresh();
}

void ChatMessagesView::appendMessage(Message message)
{
	ChatMessage *chatMessage = new ChatMessage(message);
	appendMessage(chatMessage);
}

void ChatMessagesView::appendMessage(ChatMessage *message)
{
	kdebugf();

//	connect(message, SIGNAL(statusChanged(Message::Status)),
//			 this, SLOT(repaintMessages()));

	rememberScrollBarPosition();

	Renderer->appendMessage(message);
}

void ChatMessagesView::appendMessages(QList<Message> messages)
{
	kdebugf2();
	
	foreach (Message message, messages)
		appendMessage(message);
}

void ChatMessagesView::appendMessages(QList<ChatMessage *> messages)
{
	kdebugf2();

//	foreach (ChatMessage *message, messages)
//		connect(message, SIGNAL(statusChanged(Message::Status)),
//				this, SLOT(repaintMessages()));
	rememberScrollBarPosition();

	Renderer->appendMessages(messages);
}

void ChatMessagesView::clearMessages()
{
	Renderer->clearMessages();
}

unsigned int ChatMessagesView::countMessages()
{
	return Renderer->messages().count();
}

void ChatMessagesView::resizeEvent(QResizeEvent *e)
{
 	LastScrollValue = page()->currentFrame()->scrollBarValue(Qt::Vertical);
 	LastLine = (LastScrollValue == page()->currentFrame()->scrollBarMaximum(Qt::Vertical));

 	KaduTextBrowser::resizeEvent(e);

	scrollToLine();
}

void ChatMessagesView::rememberScrollBarPosition()
{
	LastScrollValue = page()->currentFrame()->scrollBarValue(Qt::Vertical);
	LastLine = (LastScrollValue == page()->currentFrame()->scrollBarMaximum(Qt::Vertical));
}

void ChatMessagesView::scrollToLine()
{
 	if (LastLine)
 		page()->currentFrame()->setScrollBarValue(Qt::Vertical, page()->currentFrame()->scrollBarMaximum(Qt::Vertical));
 	else
 		page()->currentFrame()->setScrollBarValue(Qt::Vertical, LastScrollValue);
}
