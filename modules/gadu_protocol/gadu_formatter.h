/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GADU_FORMATTER_H
#define GADU_FORMATTER_H

#include "gadu.h"

class Message;

class GaduFormater
{
	static void appendToMessage(Message &result, UinType sender, const QString &content, struct gg_msg_richtext_format &format,
		struct gg_msg_richtext_color &color, struct gg_msg_richtext_image &image, bool receiveImages);

public:
	static unsigned int computeFormatsSize(const Message &message);
	static unsigned char * createFormats(const Message &message, unsigned int &size);

	static Message createMessage(UinType sender, const QString &content, unsigned char * formats, unsigned int size, bool receiveImages);

};

#endif // GADU_FORMATTER_H