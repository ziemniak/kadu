/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtWebKit/QWebFrame>
//TODO:
#include "accounts/account.h"
#include "accounts/account_data.h"
#include "accounts/account_manager.h"

#include "chat/chat_message.h"
#include "chat/chat_styles_manager.h"

#include "gui/widgets/chat_messages_view.h"
#include "gui/widgets/chat_widget.h"
#include "gui/widgets/preview.h"

#include "protocols/protocol_factory.h"

#include "kadu_parser.h"
#include "misc.h"

#include "chat_engine_adium.h"

const char *AdiumChatStyleEngine::xhtmlBase =
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
		"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    	"<head>\n"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\n\" />\n"
        "<base href=\"file://%1\">\n"
		"<style id=\"KaduStyle\" type=\"text/css\" >\n"
		"	%5\n"
		"</style>\n"
		"<style id=\"baseStyle\" type=\"text/css\" >\n"
		"	@import url(\"main.css\");\n"
		"	*{ word-wrap:break-word; }\n"
		"</style>\n"
		"<style id=\"mainStyle\" type=\"text/css\" >\n"
		"	@import url(\"%4\");\n"
        "</style>\n"
		"</head>\n"
		"<body>\n"
		"%2\n"
		"<div id=\"Chat\"></div>\n"
		"%3\n"
		"</body>"
		"</html>\n";

void AdiumChatStyleEngine::pruneMessage(ChatMessagesView *view)
{
	if (!ChatStylesManager::instance()->cfgNoHeaderRepeat())
		view->page()->mainFrame()->evaluateJavaScript("kadu_removeFirstMessage()");	
}

void AdiumChatStyleEngine::clearMessages(ChatMessagesView *view)
{
	view->page()->mainFrame()->evaluateJavaScript("kadu_clearMessages()");
}

bool AdiumChatStyleEngine::isThemeValid(QString styleName)
{
	// Minimal Adium style layout
	QDir dir(styleName);
	if (!dir.cd("Contents/Resources/"))
		return false;

	QFileInfo fi(dir, "Incoming/Content.html");	
	if (!fi.isReadable())
		return false;
	
	fi.setFile(dir, "main.css");
	if (!fi.isReadable())
		return false;

	fi.setFile(dir, "Status.html");
	if (!fi.isReadable())
		return false;

	return true;
}

QStringList AdiumChatStyleEngine::styleVariants(QString styleName)
{
	QDir dir;
	QString styleBaseHref = ggPath() + "/syntax/chat/" + styleName + "/Contents/Resources/Variants/";
	if (!dir.exists(styleBaseHref))
		styleBaseHref = dataPath("kadu") + "/syntax/chat/" + styleName + "/Contents/Resources/Variants/";
	dir.setPath(styleBaseHref);
	dir.setNameFilters(QStringList("*.css"));
	return dir.entryList(); 
}

void AdiumChatStyleEngine::appendMessages(ChatMessagesView *view, QList<ChatMessage *> messages)
{
	foreach (ChatMessage *message, messages)
		appendMessage(view, message);
}

void AdiumChatStyleEngine::appendMessage(ChatMessagesView *view, ChatMessage *message)
{
	QString formattedMessageHtml;
	bool includeHeader = true;

	if (ChatStylesManager::instance()->cfgNoHeaderRepeat() && view->prevMessage())
	{
		includeHeader =
			(view->prevMessage()->type() != TypeSystem) &&
			((message->date().toTime_t() - view->prevMessage()->date().toTime_t() > (ChatStylesManager::instance()->cfgNoHeaderInterval() * 60)) ||
			 (message->sender() != view->prevMessage()->sender()));
	}
	switch (message->type())
	{
		case TypeReceived:
		{
			if(includeHeader)
				formattedMessageHtml = IncomingHtml;
			else
				formattedMessageHtml = NextIncomingHtml;
			break;
		}
		case TypeSent:
		{
			if(includeHeader)
				formattedMessageHtml = OutgoingHtml;
			else
				formattedMessageHtml = NextOutgoingHtml;
			break;
		}//TODO 0.6.6:
		case TypeSystem:
		{
			formattedMessageHtml = StatusHtml;
			break;
		}
	}
	
	formattedMessageHtml = replaceKeywords(view, formattedMessageHtml, message);
	formattedMessageHtml.replace("\n", " ");
	formattedMessageHtml.prepend("<span>");
	formattedMessageHtml.append("</span>");

	view->rememberScrollBarPosition();

	if (includeHeader)
		view->page()->mainFrame()->evaluateJavaScript("kadu_appendNextMessage(\'"+ formattedMessageHtml +"\')");
	else
		view->page()->mainFrame()->evaluateJavaScript("kadu_appendConsecutiveMessage(\'"+ formattedMessageHtml +"\')");

	view->prevMessage() = message;
}

void AdiumChatStyleEngine::refreshView(ChatMessagesView *view)
{
	QString styleBaseHtml = QString(xhtmlBase).arg(BaseHref)
		.arg(replaceKeywords(view, HeaderHtml))
		.arg(replaceKeywords(view, FooterHtml))
		.arg("Variants/" + StyleVariantName)
		.arg(ChatStylesManager::instance()->mainStyle());
	view->setHtml(styleBaseHtml);

	view->page()->mainFrame()->evaluateJavaScript(jsCode);

	view->rememberScrollBarPosition();

	foreach (ChatMessage *message, view->messages())
		appendMessage(view, message);


}

AdiumChatStyleEngine::AdiumChatStyleEngine()
{
	// Load required javascript funtions
	QFile file(dataPath("kadu") + "/scripts/adium_style_scripts.js");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
		jsCode = file.readAll();

}

QString AdiumChatStyleEngine::readThemePart(QString part)
{
	QFile fileAccess;
	QString resultHtml = QString::null;
	if(QFile::exists(part))
	{

		fileAccess.setFileName(part);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream stream(&fileAccess);
		stream.setCodec(QTextCodec::codecForName("UTF-8"));
		resultHtml = stream.readAll();

		fileAccess.close();
	}
	return resultHtml;
}

void AdiumChatStyleEngine::loadTheme(const QString &styleName, const QString &variantName)
{
	QDir dir;
	CurrentStyleName = styleName;
	StyleVariantName = variantName;
	BaseHref = ggPath() + "/syntax/chat/" + styleName + "/Contents/Resources/";
	if (!dir.exists(BaseHref))
		BaseHref = dataPath("kadu") + "/syntax/chat/" + styleName + "/Contents/Resources/";

	IncomingHtml = readThemePart(BaseHref + "Incoming/Content.html");

	if (dir.exists("Incoming/NextContent.html"))
		NextIncomingHtml = readThemePart(BaseHref + "Incoming/NextContent.html");
	else
		NextIncomingHtml = IncomingHtml;

	if (dir.exists("Outgoing/Content.html"))
		OutgoingHtml = readThemePart(BaseHref + "Outgoing/Content.html");
	else
		OutgoingHtml = IncomingHtml;

	if (dir.exists("Outgoing/NextContent.html"))
		NextOutgoingHtml = readThemePart(BaseHref + "Outgoing/Content.html");
	else
		NextOutgoingHtml = OutgoingHtml;

	HeaderHtml = readThemePart(BaseHref + "Header.html");
	FooterHtml = readThemePart(BaseHref + "Footer.html");

	StatusHtml = readThemePart(BaseHref + "Status.html");
}

//TODO:
void AdiumChatStyleEngine::prepareStylePreview(Preview *preview, QString styleName, QString variantName)
{

}

bool AdiumChatStyleEngine::removeStyle(const QString &styleName)
{

}

// Some parts of the code below are borrowed from Kopete project (http://kopete.kde.org/)
QString AdiumChatStyleEngine::replaceKeywords(ChatMessagesView *view, QString &style)
{
	QString result = style;
	
	QString name;
//TODO: get Chat name (contacts' nicks?)
/*	if (!view->chat()->contacts().isEmpty())
	{
		// Replace %chatName% //TODO. Find way to dynamic uptade this tag (add id ?)
		int uinsSize = view->chat()->contacts().count();
		if (uinsSize > 1)
		{
			int i = 0;

			foreach(const Contact &contact, view->chat()->contacts())
			{
				name.append(KaduParser::parse("%a", AccountManager::instance()->defaultAccount(), contact, false));

				if (++i < uinsSize)
					name.append(", ");
			}
		}
		else
		{
			name = view->chat()->contacts()[0].display();
		}
	}*/
	result.replace(QString("%chatName%"), name);
	// Replace %sourceName%. TODO: Do sth with it!
	result.replace(QString("%sourceName%"), AccountManager::instance()->defaultAccount()->data()->name());
	// Replace %destinationName%
	result.replace(QString("%destinationName%"), name);
	// For %timeOpened%, display the date and time. TODO: get real time 
	result.replace(QString("%timeOpened%"), printDateTime(QDateTime::currentDateTime()));

	// Look for %timeOpened{X}% TODO: get real time
	QRegExp timeRegExp("%timeOpened\\{([^}]*)\\}%");
	int pos=0;
	while((pos=timeRegExp.indexIn(result, pos)) != -1)
		result.replace(pos, timeRegExp.cap(0).length(), QDateTime::currentDateTime().toString(timeRegExp.cap(1)));
	// Get avatars. TODO: We don't support avatars at this time
	QString photoIncoming;
	QString photoOutgoing;

	photoIncoming = QString("file://") + BaseHref + QString("Incoming/buddy_icon.png");
	photoOutgoing = QString("file://") + BaseHref + QString("Outgoing/buddy_icon.png");

	result.replace(QString("%incomingIconPath%"), photoIncoming);
	result.replace(QString("%outgoingIconPath%"), photoOutgoing);

	return result;
}
QString AdiumChatStyleEngine::replaceKeywords(ChatMessagesView *view, QString &source, ChatMessage *message)
{
	QString result = source;
	QString nick, contactId, service, protocolIcon, nickLink;

	// Replace sender (contact nick)
	result.replace(QString("%sender%"), message->sender().display());
	// Replace time
	result.replace(QString("%time%"), printDateTime(message->sdate()));
	// Replace %screenName% (contact ID) TODO:
	result.replace(QString("%senderScreenName%"), message->sender().id(message->sender().prefferedAccount()));
	// Replace service name (protocol name) TODO:
	result.replace(QString("%service%"), view->chat()->currentProtocol()->protocolFactory()->displayName());
	// Replace protocolIcon (sender statusIcon)
	result.replace(QString("%senderStatusIcon%"), "");

	// Look for %time{X}%
	QRegExp timeRegExp("%time\\{([^}]*)\\}%");
	int pos = 0;
	while((pos = timeRegExp.indexIn(result , pos)) != -1)
		result.replace(pos, timeRegExp.cap(0).length(), message->sdate().toString(timeRegExp.cap(1)));

	// Look for %textbackgroundcolor{X}%
	// TODO: highlight background color: use the X value.
	QRegExp textBackgroundRegExp("%textbackgroundcolor\\{([^}]*)\\}%");
	int textPos=0;
	while((textPos=textBackgroundRegExp.indexIn(result, textPos)) != -1)
		result.replace(textPos, textBackgroundRegExp.cap(0).length(), "inherit");

	// Replace userIconPath TODO: add avatars support
	QString photoPath;
	if(message->type() == TypeReceived)
	{
   		result.replace(QString("%messageClasses%"), "message incoming");
		photoPath = QString("file://") + BaseHref + QString("Incoming/buddy_icon.png");
	}
	else
	{
   		result.replace(QString("%messageClasses%"), "message outgoing");
		photoPath = QString("file://") + BaseHref + QString("Outgoing/buddy_icon.png");
	}
	result.replace(QString("%userIconPath%"), photoPath);

	//Message direction ("rtl"(Right-To-Left) or "ltr"(Left-to-right))
	result.replace(QString("%messageDirection%"), "ltr");

	// Replace contact's color
	const QString colorName = message->nickColor;
	QString lightColorName;	
	QRegExp senderColorRegExp("%senderColor(?:\\{([^}]*)\\})?%");
	textPos = 0;
	while((textPos = senderColorRegExp.indexIn(result, textPos)) != -1)
	{
		int light = 100;
		bool doLight = false;
		if(senderColorRegExp.numCaptures()>=1)
			light = senderColorRegExp.cap(1).toUInt(&doLight);

		if (doLight && lightColorName.isNull())
			lightColorName = QColor(colorName).light(light).name();

		result.replace(textPos ,senderColorRegExp.cap(0).length(), doLight ? lightColorName : colorName);
	}

	// Replace message TODO: do sth with formatMessage
	QString messageText = QString("<span>") + formatMessage(message->unformattedMessage, message->backgroundColor) + QString("</span>");
	result.replace(QString("%message%"), messageText);

	return result;
}
