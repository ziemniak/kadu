/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QRegExp>

#include "configuration/configuration-file.h"

#include "debug.h"
#include "misc/misc.h"

#include "html_document.h"

HtmlDocument::HtmlDocument() : Elements()
{
}

void HtmlDocument::escapeText(QString &text)
{
	//UWAGA: &amp; MUSI by� na pocz�tku!
	text.replace("&", "&amp;");
	text.replace("<", "&lt;");
	text.replace(">", "&gt;");
	text.replace("\"", "&quot;");
	text.replace("'", "&apos;");
	text.replace("  ", "&nbsp; ");
}

void HtmlDocument::unescapeText(QString &text)
{
	//UWAGA: &amp; MUSI by� na ko�cu!
	text.replace("<span style='color:#000000;'>", "<span>");
	text.replace("&nbsp;", " ");
	text.replace("&lt;", "<");
	text.replace("&gt;", ">");
	text.replace("&quot;", "\"");
	text.replace("&apos;", "'");
	text.replace("&amp;", "&");
}

void HtmlDocument::addElement(Element e)
{
	unescapeText(e.text);
	Elements.append(e);
}

void HtmlDocument::addTag(const QString &text)
{
	Element e;
	e.text = text;
	e.tag = true;
	Elements.append(e);
}

void HtmlDocument::addText(const QString &text)
{
	Element e;
	e.text = text;
	e.tag = false;
	Elements.append(e);
}

void HtmlDocument::insertTag(const int pos, const QString &text)
{
	Element e;
	e.text = text;
	e.tag = true;
	Elements.insert(pos,e);
}

void HtmlDocument::insertText(const int pos, const QString &text)
{
	Element e;
	e.text = text;
	e.tag = false;
	Elements.append(e);
	Elements.insert(pos,e);
}

void HtmlDocument::parseHtml(const QString &html)
{
//	kdebugm(KDEBUG_FUNCTION_START | KDEBUG_INFO, "%s\n", qPrintable(html));
	Element e;
	e.tag = false;
	int pos1, pos2;
	int len = html.length();
	for(unsigned int i = 0, htmllength = html.length(); i < htmllength; ++i)
	{
		const QChar &ch = html[i];
		switch (ch.toAscii())
		{
			case '<':
				if (!e.tag)
				{
					if (!e.text.isEmpty())
						addElement(e);
					e.tag = true;
					e.text = ch;
				}
				break;
			case '>':
				if (e.tag)
				{
					e.text += ch;
					addElement(e);
					e.tag = false;
					e.text = QString::null;
				}
				break;
			default:
				pos1 = html.indexOf('>', i + 1);
				if (pos1 == -1)
					pos1 = len;
				pos2 = html.indexOf('<', i + 1);
				if (pos2 == -1)
					pos2 = len;
				if (pos2 < pos1)
					pos1 = pos2;
				e.text += html.mid(i, pos1 - i);
				i = pos1 - 1;
		}
	}
	if (!e.text.isEmpty())
		addElement(e);
}

QString HtmlDocument::generateHtml() const
{
	QString html,tmp;
	foreach(const Element &e, Elements)
	{
		tmp = e.text;
		if (!e.tag)
			escapeText(tmp);
		html += tmp;
	}
	return html;
}

int HtmlDocument::countElements() const
{
	return Elements.size();
}

bool HtmlDocument::isTagElement(int index) const
{
	return Elements[index].tag;
}

const QString & HtmlDocument::elementText(int index) const
{
	return Elements[index].text;
}

QString & HtmlDocument::elementText(int index)
{
	return Elements[index].text;
}

void HtmlDocument::setElementValue(int index, const QString &text, bool tag)
{
	Element& e=Elements[index];
	e.text=text;
	e.tag=tag;
}

void HtmlDocument::splitElement(int &index, int start, int length)
{
	Element& e=Elements[index];
	if(start>0)
	{
		Element pre;
		pre.tag=e.tag;
		pre.text=e.text.left(start);
		Elements.insert(index, pre);
		++index;
	}
	if(uint(start+length)<e.text.length())
	{
		Element post;
		post.tag=e.tag;
		post.text=e.text.right(e.text.length()-(start+length));
		if(uint(index+1)<Elements.size())
			Elements.insert(index+1, post);
		else
			Elements.append(post);
	}
	e.text=e.text.mid(start,length);
}

QRegExp *HtmlDocument::url_regexp = 0;

const QRegExp &HtmlDocument::urlRegExp()
{
	if (!url_regexp)
	// TODO: WTF? fix it!!!!!!!!!!!!!!!!!!!!!!!
		url_regexp = new QRegExp(latin2unicode("(http://|https://|www\\.|ftp://|ftp\\.|sftp://|smb://|file:/|rsync://|svn://|svn\\+ssh://)[a-zA-Z0-9�󱶳�����ӡ������\\*\\-\\._/~?=&#\\+%\\(\\)\\[\\]:;,!@\\\\]*"));
	return *url_regexp;
}

void HtmlDocument::convertUrlsToHtml()
{
	QRegExp r = urlRegExp();
	for(int i = 0; i < countElements(); ++i)
	{
		if(isTagElement(i))
			continue;

		QString text = elementText(i);
		int p = r.indexIn(text);
		if (p < 0)
			continue;

		int l = r.matchedLength();
		int lft = config_file.readNumEntry("Chat", "LinkFoldTreshold");

		QString link;
		QString displayLink = text.mid(p, l);
		QString aLink = displayLink;

		aLink.replace("%20", "%2520"); // Opera's bug workaround - allows opening links with spaces
		if (!aLink.contains("://"))
			aLink.prepend("http://");

		if ((l - p > lft) && config_file.readBoolEntry("Chat", "FoldLink"))
			link = "<a href=\"" + aLink + "\">" + text.mid(p, p+(lft/2)) + "..." + text.mid(l-(lft/2), lft/2) + "</a>";
		else
			link = "<a href=\"" + aLink + "\">" + displayLink + "</a>";

		splitElement(i, p, l);
		setElementValue(i, link, true);
	}
}

QRegExp *HtmlDocument::mail_regexp = 0;

const QRegExp &HtmlDocument::mailRegExp()
{
	if (!mail_regexp)
		mail_regexp = new QRegExp(latin2unicode("(mailto:){0,1}[a-zA-Z0-9_\\.\\-]+@[a-zA-Z0-9\\-\\.]+\\.[a-zA-Z]{2,4}"));
	return *mail_regexp;
}

void HtmlDocument::convertMailToHtml()
{
	QRegExp m = mailRegExp();
	for (int i = 0; i < countElements(); ++i)
	{
		if (isTagElement(i))
			continue;

		QString text = elementText(i);
		int p = m.indexIn(text);
		if (p < 0)
			continue;
		unsigned int l = m.matchedLength();
		QString mail = text.mid(p, l);
		
		splitElement(i, p, l);
		setElementValue(i, "<a href=\"mailto:" + mail + "\">" + mail + "</a>", true);
	}
}

QRegExp *HtmlDocument::gg_regexp = 0;

const QRegExp &HtmlDocument::ggRegExp()
{
	if (!gg_regexp)
		gg_regexp = new QRegExp(latin2unicode("gg:(/){0,3}[0-9]{1,8}"));
	return *gg_regexp;
}

void HtmlDocument::convertGGToHtml()
{
	QRegExp g = ggRegExp();
	for (int i = 0; i < countElements(); ++i)
	{
		if (isTagElement(i))
			continue;

		QString text = elementText(i);
		int p = g.indexIn(text);
		if (p < 0)
			continue;
		unsigned int l = g.matchedLength();
		QString gg = text.mid(p, l);

		splitElement(i, p, l);
		setElementValue(i, "<a href=\"" + gg + "\">" + gg + "</a>", true);
	}
}
