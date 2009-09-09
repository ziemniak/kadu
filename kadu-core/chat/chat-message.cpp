/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "chat/message/formatted-message-part.h"
#include "configuration/configuration-file.h"
#include "parser/parser.h"

#include "misc/misc.h"

#include "chat-message.h"

QString formatMessage(const QString &text, const QString &backgroundColor)
{
	HtmlDocument htmlDocument;
	htmlDocument.parseHtml(text);
	htmlDocument.convertUrlsToHtml();
	htmlDocument.convertMailToHtml();
	EmoticonsManager::instance()->expandEmoticons(htmlDocument, backgroundColor, (EmoticonsStyle)config_file.readNumEntry("Chat", "EmoticonsStyle"));
// 	GaduImagesManager::setBackgroundsForAnimatedImages(htmlDocument, backgroundColor);

	return htmlDocument.generateHtml();
}

static QString getMessage(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage)
		return formatMessage(chatMessage->unformattedMessage, chatMessage->backgroundColor);
	else
		return "";
}

static QString getBackgroundColor(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage)
		return chatMessage->backgroundColor;
	else
		return "";
}

static QString getFontColor(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage)
		return chatMessage->fontColor;
	else
		return "";
}

static QString getNickColor(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage)
		return chatMessage->nickColor;
	else
		return "";
}

static QString getSentDate(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage && chatMessage->showServerTime())
		return printDateTime(chatMessage->message().sendDate());
	else
		return "";
}

static QString getReceivedDate(const QObject * const object)
{
	const ChatMessage * const chatMessage = dynamic_cast<const ChatMessage * const>(object);
	if (chatMessage)
		return printDateTime(chatMessage->message().receiveDate());
	else
		return "";
}

static QString getSeparator(const QObject * const object)
{
	int separatorSize = dynamic_cast<const ChatMessage * const>(object)->separatorSize();

	if (separatorSize)
		return "<div style=\"margin: 0; margin-top:" + QString::number(separatorSize) + "px\"></div>";
	else
		return "";
}

void ChatMessage::registerParserTags()
{
	Parser::registerObjectTag("message", getMessage);
	Parser::registerObjectTag("backgroundColor", getBackgroundColor);
	Parser::registerObjectTag("fontColor", getFontColor);
	Parser::registerObjectTag("nickColor", getNickColor);
	Parser::registerObjectTag("sentDate", getSentDate);
	Parser::registerObjectTag("receivedDate", getReceivedDate);
	Parser::registerObjectTag("separator", getSeparator);
}

void ChatMessage::unregisterParserTags()
{
	Parser::unregisterObjectTag("message", getMessage);
	Parser::unregisterObjectTag("backgroundColor", getBackgroundColor);
	Parser::unregisterObjectTag("fontColor", getFontColor);
	Parser::unregisterObjectTag("nickColor", getNickColor);
	Parser::unregisterObjectTag("sentDate", getSentDate);
	Parser::unregisterObjectTag("receivedDate", getReceivedDate);
	Parser::unregisterObjectTag("separator", getSeparator);
}

ChatMessage::ChatMessage(const Message &msg) :
		MyMessage(msg), ShowServerTime(true)
{
	switch (msg.type())
	{
		case Message::TypeSent:
			backgroundColor = config_file.readEntry("Look", "ChatMyBgColor");
			fontColor = config_file.readEntry("Look", "ChatMyFontColor");
			nickColor = config_file.readEntry("Look", "ChatMyNickColor");
			break;

		case Message::TypeReceived:
			backgroundColor = config_file.readEntry("Look", "ChatUsrBgColor");
			fontColor = config_file.readEntry("Look", "ChatUsrFontColor");
			nickColor = config_file.readEntry("Look", "ChatUsrNickColor");
			break;
	}

	this->unformattedMessage = MyMessage.content();

	this->unformattedMessage.replace("\r\n", "<br/>");
	this->unformattedMessage.replace("\n",   "<br/>");
	this->unformattedMessage.replace("\r",   "<br/>");
	this->unformattedMessage.replace(QChar::LineSeparator, "<br />");

	connect(&MyMessage, SIGNAL(statusChanged(Message::Status)),
			this, SIGNAL(statusChanged(Message::Status)));

// 	convertCharacters(unformattedMessage, backgroundColor,
// 		(EmoticonsStyle)config_file.readNumEntry("Chat", "EmoticonsStyle"));
}

void ChatMessage::replaceLoadingImages(const QString &imageId, const QString &imagePath)
{
	unformattedMessage = FormattedMessagePart::replaceLoadingImages(unformattedMessage, imageId, imagePath);
}

void ChatMessage::setShowServerTime(bool noServerTime, int noServerTimeDiff)
{
	ShowServerTime = (MyMessage.sendDate().isValid()
			&& (!noServerTime || (abs(MyMessage.receiveDate().toTime_t() - MyMessage.sendDate().toTime_t())) > noServerTimeDiff));
}

void ChatMessage::setColorsAndBackground(const QString &backgroundColor, const QString &nickColor, const QString &fontColor)
{
		if (message().type() == Message::TypeSystem)
			return;

		this->backgroundColor = backgroundColor;
		this->nickColor = nickColor;
		this->fontColor = fontColor;
}
