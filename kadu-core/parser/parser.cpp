/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtNetwork/QHostAddress>

#include "accounts/account-manager.h"
#include "configuration/configuration-file.h"
#include "contacts/contact-account-data.h"
#include "parser/parser-token.h"
#include "misc/misc.h"
#include "status/status-type.h"
#include "status/status-type-manager.h"

#include "debug.h"
#include "html_document.h"
#include "icons-manager.h"

#include "parser.h"

QMap<QString, QString> Parser::globalVariables;
QMap<QString, QString (*)(const Contact &)> Parser::registeredTags;
QMap<QString, QString (*)(const QObject * const)> Parser::registeredObjectTags;

bool Parser::registerTag(const QString &name, QString (*func)(const Contact &))
{
	kdebugf();
	if (registeredTags.contains(name))
	{
		kdebugmf(KDEBUG_ERROR | KDEBUG_FUNCTION_END, "tag %s already registered!\n", qPrintable(name));
		return false;
	}
	else
	{
		registeredTags.insert(name, func);
		kdebugf2();
		return true;
	}
}

bool Parser::unregisterTag(const QString &name, QString (* /*func*/)(const Contact &))
{
	kdebugf();
	if (!registeredTags.contains(name))
	{
		kdebugmf(KDEBUG_ERROR | KDEBUG_FUNCTION_END, "tag %s not registered!\n", qPrintable(name));
		return false;
	}
	else
	{
		registeredTags.remove(name);
		kdebugf2();
		return true;
	}
}

bool Parser::registerObjectTag(const QString &name, ObjectTagCallback func)
{
	kdebugf();
	if (registeredObjectTags.contains(name))
	{
		kdebugmf(KDEBUG_ERROR | KDEBUG_FUNCTION_END, "tag %s already registered!\n", qPrintable(name));
		return false;
	}
	else
	{
		registeredObjectTags.insert(name, func);
		kdebugf2();
		return true;
	}
}

bool Parser::unregisterObjectTag(const QString &name, ObjectTagCallback)
{
	kdebugf();
	if (!registeredObjectTags.contains(name))
	{
		kdebugmf(KDEBUG_ERROR | KDEBUG_FUNCTION_END, "tag %s not registered!\n", qPrintable(name));
		return false;
	}
	else
	{
		registeredObjectTags.remove(name);
		kdebugf2();
		return true;
	}
}

QString Parser::executeCmd(const QString &cmd)
{
	kdebugf();

	QString s(cmd);
	s.remove(QRegExp("`|>|<"));
	s.append(" > " + ggPath("execoutput"));

	int ret = system(qPrintable(s));

	s = QString::null;

	if (ret != -1)
	{
		QFile *f = new QFile(ggPath("execoutput"));
		if (f->open(QIODevice::ReadOnly))
		{
			s = QString(f->readAll());
			f->close();
			QFile::remove(ggPath("execoutput"));
		}
		delete f;
	}

	kdebugf2();
	return s;
}

QString Parser::parse(const QString &s, const QObject * const object, bool escape)
{
    	return parse(s, 0, Contact(), object, escape);
}

QString Parser::parse(const QString &s, Account *account, const Contact &contact, bool escape)
{
	return parse(s, account, contact, 0, escape);
}

QString Parser::parse(const QString &s, Account *account, const Contact &contact, const QObject * const object, bool escape)
{
	kdebugmf(KDEBUG_DUMP, "%s escape=%i\n", qPrintable(s), escape);
	int index = 0, i, len = s.length();
	QList<ParserToken> parseStack;

	static QHash<QChar, bool> searchChars;

	if (!searchChars.value('%', false))
	{
		searchChars['%'] = true;
		searchChars['['] = true;
		searchChars['{'] = true;
		searchChars['\\'] = true;
		searchChars['$'] = true;
		searchChars['@'] = true;
		searchChars['#'] = true;
		searchChars['}'] = true;
		searchChars[']'] = true;
	}

	bool allowExec = config_file.readBoolEntry("General", "AllowExecutingFromParser", false);
	searchChars['`'] = allowExec;
	searchChars['\''] = allowExec;

	while (index < len)
	{
		ParserToken pe1, pe;

		for(i = index; i < len; ++i)
			if (searchChars.value(s[i], false))
				break;

//		this is the same, but code above is muuuuch faster
//		i=s.find(QRegExp("%|`|\\{|\\[|'|\\}|\\]"), index);

		if (i != index)
		{
			pe1.type = ParserToken::PT_STRING;
			pe1.content = s.mid(index, i - index);
			parseStack.push_back(pe1);

			if (i == len)
				break;
		}

		char c = s[i].toAscii();
		if (c == '%')
		{
			++i;
			if (i == len)
				break;
			pe.type = ParserToken::PT_STRING;

			ContactAccountData *data = contact.accountData(account);

			switch (s[i].toAscii())
			{
				case 's':
					++i;
					if (data)
					{
						StatusType *type = StatusTypeManager::instance()->statusType(data->status().type());
						if (type)
							pe.content = type->displayName();
					}
					break; // TODO: 't' removed
				case 'q':
					++i;
					if (data)
						pe.content = "" ; // ule.status("Gadu").pixmapName(); TODO: 0.6.6
					break;
				case 'd':
					++i;
					if (data)
						pe.content = data->status().description();

				 	if (escape)
			 			HtmlDocument::escapeText(pe.content);
					if (config_file.readBoolEntry("Look", "ShowMultilineDesc"))
					{
						pe.content.replace("\n", "<br/>");
						pe.content.replace(QRegExp("\\s\\s"), QString(" &nbsp;"));
					}
					break;
				case 'i':
					++i;
					if (data)
						pe.content = data->ip().toString();
					break;
				case 'v':
					++i;
					if (data)
						pe.content = data->dnsName();
					break;
				case 'o':
					++i;
					if (data && data->port() == 2)
						pe.content = " ";
					break;
				case 'p':
					++i;
					if (data && data->port())
						pe.content = QString::number(data->port());
					break;
				case 'u':
					++i;
					if (data)
						pe.content = data->id();
					break;
				case 'h':
					++i;
					if (data)
						if (data && !data->status().isDisconnected())
							pe.content = data->protocolVersion();
					break;
				case 'n':
					++i;
					pe.content = contact.nickName();
					if (escape)
						HtmlDocument::escapeText(pe.content);
					break;
				case 'a':
					++i;
					pe.content = contact.display();
					if (escape)
						HtmlDocument::escapeText(pe.content);
					break;
				case 'f':
					++i;
					pe.content = contact.firstName();
					if (escape)
						HtmlDocument::escapeText(pe.content);
					break;
				case 'r':
					++i;
					pe.content = contact.lastName();
					if (escape)
						HtmlDocument::escapeText(pe.content);
					break;
				case 'm':
					++i;
					pe.content = contact.mobile();
					break;
				case 'g':
					++i;
					pe.content = ""; // TODO 0.6.6 contact.data("Groups").toStringList().join(",");
					break;
				case 'e':
					++i;
					pe.content = contact.email();
					break;
				case 'x':
					++i;
					//if (ule.usesProtocol("Gadu"))
						pe.content = ""; // TODO 0.6.6 QString::number(ule.protocolData("Gadu", "MaxImageSize").toUInt());
					break;
				case '%':
					++i;
				default:
					pe.content = "%";
			}
			parseStack.push_back(pe);
		}
		else if (c == '[')
		{
			++i;
			if (i == len)
				break;
			if (s[i] == '!')
			{
				pe.type = ParserToken::PT_CHECK_ANY_NULL;
				++i;
			}
			else
				pe.type = ParserToken::PT_CHECK_ALL_NOT_NULL;
			parseStack.push_back(pe);
		}
		else if (c == ']')
		{
			++i;
			bool anyNull = false;
			bool found = false;
			if (!parseStack.isEmpty())
			{
				QList<ParserToken>::const_iterator begin = parseStack.constBegin();
				QList<ParserToken>::const_iterator it = parseStack.constEnd();
				while (!found && it != begin)
				{
					--it;
					ParserToken::ParserTokenType t = (*it).type;
					if (t == ParserToken::PT_STRING)
						continue;
					else if (t == ParserToken::PT_CHECK_ALL_NOT_NULL || t == ParserToken::PT_CHECK_ANY_NULL)
						found = true;
					else
						break;
				}
			}
			if (!found)
			{
				pe.content = ']';
				pe.type = ParserToken::PT_STRING;
				parseStack.push_back(pe);
			}
			else
				while (!parseStack.empty())
				{
					const ParserToken &pe2 = parseStack.last();
					if (pe2.type == ParserToken::PT_STRING)
					{
						anyNull = anyNull || pe2.content.isEmpty();
						pe.content.prepend(pe2.content);
						parseStack.pop_back();
					}
					else if (pe2.type == ParserToken::PT_CHECK_ALL_NOT_NULL)
					{
						parseStack.pop_back();
						if (!anyNull)
						{
							pe.type = ParserToken::PT_STRING;
							parseStack.push_back(pe);
						}
						break;
					}
					else if (pe2.type == ParserToken::PT_CHECK_ANY_NULL)
					{
						parseStack.pop_back();
						if (anyNull)
						{
							pe.type = ParserToken::PT_STRING;
							parseStack.push_back(pe);
						}
						break;
					}
				}
		}
		else if (c == '{')
		{
			++i;
			if (i == len)
				break;
			if (s[i] == '!' || s[i] == '~')
			{
				pe.type = ParserToken::PT_CHECK_FILE_NOT_EXISTS;
				++i;
			}
			else
				pe.type = ParserToken::PT_CHECK_FILE_EXISTS;
			parseStack.push_back(pe);
		}
		else if (c == '}')
		{
			++i;
			bool found = false;

			if (!parseStack.isEmpty())
			{
				QList<ParserToken>::const_iterator begin = parseStack.constBegin();
				QList<ParserToken>::const_iterator it = parseStack.constEnd();
				while (!found && it != begin)
				{
					--it;
					ParserToken::ParserTokenType t = (*it).type;
					if (t == ParserToken::PT_STRING)
						continue;
					else if (t == ParserToken::PT_EXECUTE ||
								t == ParserToken::PT_CHECK_FILE_EXISTS ||
								t == ParserToken::PT_CHECK_FILE_NOT_EXISTS ||
								t == ParserToken::PT_VARIABLE ||
								t == ParserToken::PT_ICONPATH ||
								t == ParserToken::PT_EXTERNAL_VARIABLE ||
								t == ParserToken::PT_EXECUTE2)
						found = true;
					else
						break;
				}
			}

			if (!found)
			{
				pe.content = '}';
				pe.type = ParserToken::PT_STRING;
				parseStack.push_back(pe);
			}
			else
				while (!parseStack.empty())
				{
					const ParserToken &pe2 = parseStack.last();
					if (pe2.type == ParserToken::PT_STRING)
					{
						pe.content.prepend(pe2.content);
						parseStack.pop_back();
					}
					else if (pe2.type == ParserToken::PT_CHECK_FILE_EXISTS || pe2.type == ParserToken::PT_CHECK_FILE_NOT_EXISTS)
					{
						// zmienna potrzebna, bo pop_back() zniszczy nam zmienn� pe2, kt�r� pobrali�my przez referencj�
						bool check_file_exists = pe2.type == ParserToken::PT_CHECK_FILE_EXISTS;

						int spacePos = pe.content.indexOf(' ', 0);
						parseStack.pop_back();
						QString file;
//						kdebugm(KDEBUG_INFO, "spacePos: %d\n", spacePos);
						if (spacePos == -1)
							file = pe.content;
						else
							file = pe.content.left(spacePos);
//						kdebugm(KDEBUG_INFO, "file: %s\n", qPrintable(file));
						if (QFile::exists(file) == check_file_exists)
						{
							pe.content = pe.content.mid(spacePos + 1);
							pe.type = ParserToken::PT_STRING;
							parseStack.push_back(pe);
						}
						break;
					}
					else if (pe2.type == ParserToken::PT_VARIABLE)
					{
						parseStack.pop_back();
						pe.type = ParserToken::PT_STRING;
						if (Parser::globalVariables.contains(pe.content))
						{
							kdebugm(KDEBUG_INFO, "name: %s, value: %s\n", qPrintable(pe.content), qPrintable(Parser::globalVariables[pe.content]));
							pe.content = Parser::globalVariables[pe.content];
						}
						else
						{
							kdebugm(KDEBUG_WARNING, "variable %s undefined\n", qPrintable(pe.content));
							pe.content = QString::null;
						}
						parseStack.push_back(pe);
						break;
					}
					else if (pe2.type == ParserToken::PT_ICONPATH)
					{
						parseStack.pop_back();
						pe.type = ParserToken::PT_STRING;
						pe.content = IconsManager::instance()->iconPath(pe.content);
						parseStack.push_back(pe);
						break;
					}
					else if (pe2.type == ParserToken::PT_EXTERNAL_VARIABLE)
					{
						parseStack.pop_back();
						pe.type = ParserToken::PT_STRING;
						if (registeredTags.contains(pe.content))
							pe.content = registeredTags[pe.content](contact);
						else if (object && registeredObjectTags.contains(pe.content))
							pe.content = registeredObjectTags[pe.content](object);
						else
						{
							kdebugm(KDEBUG_WARNING, "tag %s not registered\n", qPrintable(pe.content));
							pe.content = QString::null;
						}
						parseStack.push_back(pe);
						break;
					}
					else if (pe2.type == ParserToken::PT_EXECUTE2)
					{
						parseStack.pop_back();
						pe.type = ParserToken::PT_STRING;
						pe.content = executeCmd(pe.content);
						parseStack.push_back(pe);
						break;
					}
				}
		}
		else if (c == '`')
		{
			++i;
			if (i == len || s[i] != '{')
			{
				pe.type = ParserToken::PT_EXECUTE;
				parseStack.push_back(pe);
			}
			else
			{
				++i;
				pe.type = ParserToken::PT_EXECUTE2;
				parseStack.push_back(pe);
			}
		}
		else if (c == '\'')
		{
			++i;
			pe.content = QString::null;
			bool found = false;
			if (!parseStack.isEmpty())
			{
				QList<ParserToken>::const_iterator begin = parseStack.constBegin();
				QList<ParserToken>::const_iterator it = parseStack.constEnd();
				while (!found && it != begin)
				{
					--it;
					ParserToken::ParserTokenType t = (*it).type;
					if (t == ParserToken::PT_STRING)
						continue;
					else if (t == ParserToken::PT_EXECUTE)
						found = true;
					else
						break;
				}
			}
			if (!found)
			{
				pe.content = '\'';
				pe.type = ParserToken::PT_STRING;
				parseStack.push_back(pe);
			}
			else
				while (!parseStack.empty())
				{
					const ParserToken &pe2 = parseStack.last();
					if (pe2.type == ParserToken::PT_STRING)
					{
						pe.content.prepend(pe2.content);
						parseStack.pop_back();
					}
					else if (pe2.type == ParserToken::PT_EXECUTE)
					{
						parseStack.pop_back();
						pe.type = ParserToken::PT_STRING;
						pe.content = executeCmd(pe.content);
						parseStack.push_back(pe);
						break;
					}
				}
		}
		else if (c == '\\')
		{
			++i;
			if (i == len)
				break;
			pe.type = ParserToken::PT_STRING;
			pe.content = s[i++];
			parseStack.push_back(pe);
		}
		else if (c == '$')
		{
			++i;
			if (i == len || s[i] != '{')
			{
				pe.type = ParserToken::PT_STRING;
				pe.content = '$';
				parseStack.push_back(pe);
			}
			else
			{
				++i;
				pe.type = ParserToken::PT_VARIABLE;
				parseStack.push_back(pe);
			}
		}
		else if (c == '@')
		{
			++i;
			if (i == len || s[i] != '{')
			{
				pe.type = ParserToken::PT_STRING;
				pe.content = '@';
				parseStack.push_back(pe);
			}
			else
			{
				++i;
				pe.type = ParserToken::PT_ICONPATH;
				parseStack.push_back(pe);
			}
		}
		else if (c == '#')
		{
			++i;
			if (i == len || s[i] != '{')
			{
				pe.type = ParserToken::PT_STRING;
				pe.content = '#';
				parseStack.push_back(pe);
			}
			else
			{
				++i;
				pe.type = ParserToken::PT_EXTERNAL_VARIABLE;
				parseStack.push_back(pe);
			}
		}
		else
			kdebugm(KDEBUG_ERROR, "shit happens? %d %c %d\n", i, (char)c, (char)c);
		index = i;
	}
	QString ret;
	QString p;
	foreach(const ParserToken &elem, parseStack)
	{
		if (elem.type != ParserToken::PT_STRING)
			kdebugm(KDEBUG_WARNING, "Incorrect parse string! %d\n", elem.type);

		switch (elem.type)
		{
			case ParserToken::PT_STRING:					p = elem.content;	break;
			case ParserToken::PT_EXTERNAL_VARIABLE:		p = "#{";		break;
			case ParserToken::PT_ICONPATH:				p = "@{";		break;
			case ParserToken::PT_VARIABLE:				p = "${";		break;
			case ParserToken::PT_CHECK_FILE_EXISTS:		p = '{';		break;
			case ParserToken::PT_CHECK_FILE_NOT_EXISTS:	p = "{!";		break;
			case ParserToken::PT_CHECK_ALL_NOT_NULL:		p = '[';		break;
			case ParserToken::PT_CHECK_ANY_NULL:			p = "[!";		break;
			case ParserToken::PT_EXECUTE:					p = '`';		break;
			case ParserToken::PT_EXECUTE2:				p = "`{";		break;
		}
		ret += p;
	}
	kdebugm(KDEBUG_DUMP, "%s\n", qPrintable(ret));
	return ret;
}
