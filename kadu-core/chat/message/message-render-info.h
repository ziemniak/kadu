/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGE_RENDER_INFO
#define MESSAGE_RENDER_INFO

#include <QtCore/QDateTime>
#include <QtCore/QString>

#include "chat/message/message.h"

#include "contacts/contact.h"
#include "contacts/contact-list.h"

#include "protocols/protocol.h"

#include "emoticons.h"

#include "exports.h"

class KADUAPI MessageRenderInfo : public QObject
{
	Q_OBJECT

	Message MyMessage;
	QString HtmlMessageContent;

	QString BackgroundColor;
	QString FontColor;
	QString NickColor;

	int SeparatorSize;
	bool ShowServerTime;

public:
	static void registerParserTags();
	static void unregisterParserTags();

	MessageRenderInfo(const Message &msg);
	virtual ~MessageRenderInfo();

	void replaceLoadingImages(const QString &imageId, const QString &imagePath);

	const Message & message() const { return MyMessage; }
	Message & message() { return MyMessage; }

	QString htmlMessageContent() const { return HtmlMessageContent; }

	MessageRenderInfo & setSeparatorSize(int separatorSize);
	int separatorSize() const { return SeparatorSize; }

	MessageRenderInfo & setShowServerTime(bool noServerTime, int noServerTimeDiff);
	bool showServerTime() const { return ShowServerTime; }

	MessageRenderInfo & setBackgroundColor(const QString &backgroundColor);
	QString backgroundColor() const { return BackgroundColor; }

	MessageRenderInfo & setFontColor(const QString &fontColor);
	QString fontColor() const { return FontColor; }

	MessageRenderInfo & setNickColor(const QString &nickColor);
	QString nickColor() const { return NickColor; }

};

QString formatMessage(const QString &text, const QString &backgroundColor);

#endif // MESSAGE_RENDER_INFO
