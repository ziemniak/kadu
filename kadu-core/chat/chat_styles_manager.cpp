/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>

#include "accounts/account_manager.h"

#include "chat/chat_message.h"
#include "chat/style-engines/chat_engine_adium.h"
#include "chat/style-engines/chat_engine_kadu.h"

#include "gui/widgets/chat_messages_view.h"
#include "gui/widgets/preview.h"
#include "gui/widgets/configuration/configuration-widget.h"
#include "gui/widgets/configuration/config-group-box.h"

#include "config_file.h"
#include "kadu.h"
#include "message_box.h"
#include "misc.h"

#include "chat_styles_manager.h"

ChatStylesManager * ChatStylesManager::Instance = 0;

ChatStylesManager * ChatStylesManager::instance()
{
	if (0 == Instance)
		Instance = new ChatStylesManager();

	return Instance;
}

ChatStylesManager::ChatStylesManager() : CurrentEngine(0), kaduEngine(0)
{
	//FIXME:
	kaduEngine = new KaduChatStyleEngine();
	registerChatStyleEngine("Kadu", kaduEngine);

	adiumEngine = new AdiumChatStyleEngine();
	registerChatStyleEngine("Adium", adiumEngine);

	loadThemes();
	configurationUpdated();
}

ChatStylesManager::~ChatStylesManager()
{
	unregisterChatStyleEngine("Kadu");
	unregisterChatStyleEngine("Adium");
}

void ChatStylesManager::registerChatStyleEngine(const QString &name, ChatStyleEngine *engine)
{
	if (0 != engine && !registeredEngines.contains(name))
		registeredEngines[name] = engine;
}

void ChatStylesManager::unregisterChatStyleEngine(const QString &name)
{
	if (registeredEngines.contains(name))
	{
		delete registeredEngines[name];
		registeredEngines.remove(name);
	}
}

void ChatStylesManager::chatViewCreated(ChatMessagesView *view)
{
	if (0 != view)
	{
		chatViews.append(view);
		CurrentEngine->refreshView(view);
	}
}

void ChatStylesManager::chatViewDestroyed(ChatMessagesView *view)
{
	if (chatViews.contains(view))
		chatViews.removeAll(view);
}

void ChatStylesManager::configurationUpdated()
{
	if (config_file.readBoolEntry("Chat", "ChatPrune"))
		Prune = config_file.readUnsignedNumEntry("Chat", "ChatPruneLen");
	else
		Prune = 0;

	ParagraphSeparator = config_file.readUnsignedNumEntry("Look", "ParagraphSeparator");

	QFont font = config_file.readFontEntry("Look","ChatFont");

	QString fontFamily = font.family();
	QString fontSize;
	if (font.pointSize() > 0)
#ifdef Q_OS_MAC
		/*  Dorr: On MacOSX this font is being displayed 3pts larger than
		 *  it really is, so reduce it's size to be coherent in entire
		 *  application.
		 */
		fontSize = QString::number(font.pointSize()-3) + "pt";
#else
		fontSize = QString::number(font.pointSize()) + "pt";
#endif
	else
		fontSize = QString::number(font.pixelSize()) + "px";
	QString fontStyle = font.italic() ? "italic" : "normal";
	QString fontWeight = font.bold() ? "bold" : "normal";
	QString textDecoration = font.underline() ? "underline" : "none";
	QString backgroundColor = config_file.readColorEntry("Look", "ChatBgColor").name();

	MainStyle = QString(
		"html {"
		"	font: %1 %2 %3 %4;"
		"	text-decoration: %5;"
		"}"
		"a {"
		"	text-decoration: underline;"
		"}"
		"body {"
		"	margin: %6;"
		"	padding: 0;"
		"	background-color: %7;"
		"}"
		"p {"
		"	margin: 0;"
		"	padding: 3px;"
		"}").arg(fontStyle, fontWeight, fontSize, fontFamily, textDecoration, QString::number(ParagraphSeparator), backgroundColor);

	CfgNoHeaderRepeat = config_file.readBoolEntry("Look", "NoHeaderRepeat");

	// headers removal stuff
	if (CfgNoHeaderRepeat)
	{
		CfgHeaderSeparatorHeight = config_file.readUnsignedNumEntry("Look", "HeaderSeparatorHeight");
		CfgNoHeaderInterval = config_file.readUnsignedNumEntry("Look", "NoHeaderInterval");
	}
	else
	{
		CfgHeaderSeparatorHeight = 0;
		CfgNoHeaderInterval = 0;
	}

	NoServerTime = config_file.readBoolEntry("Look", "NoServerTime");
	NoServerTimeDiff = config_file.readUnsignedNumEntry("Look", "NoServerTimeDiff");

	QString newStyleName = config_file.readEntry("Look", "Style");
	QString newVariantName = config_file.readEntry("Look", "ChatStyleVariant");
	// if theme was changed, load new theme
//TODO: addVariantsSupport
	if (!CurrentEngine || CurrentEngine->currentStyleName() != newStyleName || CurrentEngine->currentStyleVariant() != newVariantName)
	{
		if (availableStyles[newStyleName].engine != CurrentEngine)
			CurrentEngine = availableStyles[newStyleName].engine;
		CurrentEngine->loadTheme(newStyleName, newVariantName);
	}
	else
	{//FIXME
		CurrentEngine->configurationUpdated();
	}

	foreach (ChatMessagesView *view, chatViews)
	{
		view->updateBackgroundsAndColors();
		CurrentEngine->refreshView(view);
	}
}

//any better ideas?
void ChatStylesManager::loadThemes()
{
	QDir dir;
	QString path;
	QFileInfo fi;
	QStringList files;

	path = ggPath() + "syntax/chat/";
	dir.setPath(path);

	files = dir.entryList();

	foreach (const QString &file, files)
	{
		fi.setFile(path + file);
		if (fi.isReadable() && !availableStyles.contains(file))
		{
			foreach (ChatStyleEngine *engine, registeredEngines.values())
			{
				if (engine->isThemeValid(path + file))
				{
					availableStyles[fi.baseName()].engine = engine;
					availableStyles[fi.baseName()].global = false;
					break;
				}
			}
		}
	}

	path = dataPath("kadu") + "/syntax/chat/";
	dir.setPath(path);

	files = dir.entryList();

	foreach (const QString &file, files)
	{
		fi.setFile(path + file);
		if (fi.isReadable() && !availableStyles.contains(file))
		{
			foreach (ChatStyleEngine *engine, registeredEngines.values())
			{
				if (engine->isThemeValid(path + file))
				{
					availableStyles[fi.baseName()].engine = engine;
					availableStyles[fi.baseName()].global = true;
					break;
				}
			}
		}
	}
}

void ChatStylesManager::mainConfigurationWindowCreated(MainConfigurationWindow *window)
{
	connect(window, SIGNAL(destroyed()), this, SLOT(configurationWindowDestroyed()));
	connect(window, SIGNAL(configurationWindowApplied()), this, SLOT(configurationApplied()));

	ConfigGroupBox *groupBox = window->widget()->configGroupBox("Look", "Chat window", "Style");
//editor
	QLabel *editorLabel = new QLabel(qApp->translate("@default", "Style") + ":");
	editorLabel->setToolTip(qApp->translate("@default", "Choose style of chat window"));

	QWidget  *editor = new QWidget(groupBox->widget());
	editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	editor->setToolTip(qApp->translate("@default", "Choose style of chat window"));
	QHBoxLayout *editorLayout = new QHBoxLayout(editor);

	syntaxListCombo = new QComboBox(editor);
	syntaxListCombo->addItems(availableStyles.keys());
	syntaxListCombo->setCurrentIndex(syntaxListCombo->findText(CurrentEngine->currentStyleName()));
	connect(syntaxListCombo, SIGNAL(activated(const QString &)), this, SLOT(styleChangedSlot(const QString &)));

	editButton = new QPushButton(tr("Edit"), editor);
	editButton->setEnabled(CurrentEngine->supportEditing());

	deleteButton = new QPushButton(tr("Delete"), editor);
	deleteButton->setEnabled(!availableStyles[CurrentEngine->currentStyleName()].global);
	connect(editButton, SIGNAL(clicked()), this, SLOT(editStyleClicked()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteStyleClicked()));

	editorLayout->addWidget(syntaxListCombo, 100);
	editorLayout->addWidget(editButton);
	editorLayout->addWidget(deleteButton);
//variants
	variantListCombo = new QComboBox();
	variantListCombo->addItem("Default");
	variantListCombo->addItems(CurrentEngine->styleVariants(CurrentEngine->currentStyleName()));
	variantListCombo->setCurrentIndex(variantListCombo->findText(CurrentEngine->currentStyleVariant().isNull() ? "Default" : CurrentEngine->currentStyleVariant()));
	variantListCombo->setEnabled(CurrentEngine->supportVariants());
	connect(variantListCombo, SIGNAL(activated(const QString &)), this, SLOT(variantChangedSlot(const QString &)));
//preview
	preview = new Preview();

	preparaPreview(preview);

	CurrentEngine->prepareStylePreview(preview, CurrentEngine->currentStyleName(), CurrentEngine->currentStyleVariant());
//
	groupBox->addWidgets(editorLabel, editor);
	groupBox->addWidgets(new QLabel(qApp->translate("@default", "Style variant") + ":"), variantListCombo);
	groupBox->addWidgets(new QLabel(qApp->translate("@default", "Preview") + ":"), preview);
}

void ChatStylesManager::configurationApplied()
{
	config_file.writeEntry("Look", "Style", syntaxListCombo->currentText());
	config_file.writeEntry("Look", "ChatStyleVariant", variantListCombo->currentText());
}

void ChatStylesManager::preparaPreview(Preview *preview)
{
	Account * account = AccountManager::instance()->defaultAccount();

	Contact example;
	example.setFirstName(qApp->translate("@default", "Mark"));
	example.setLastName(qApp->translate("@default", "Smith"));
	example.setNickName(qApp->translate("@default", "Jimbo"));
	example.setDisplay(qApp->translate("@default", "Jimbo"));
	example.setMobile("+48123456789");
	example.setEmail("jimbo@mail.server.net");
	example.setHomePhone("+481234567890");

	ContactList receivers;
	receivers.append(example);
	ChatMessage *chatMessage = new ChatMessage(account, kadu->myself(), receivers, tr("Your message"), TypeSent,
		QDateTime::currentDateTime(), QDateTime::currentDateTime());
	chatMessage->setSeparatorSize(CfgHeaderSeparatorHeight);
	preview->addObjectToParse(kadu->myself() , chatMessage);

	chatMessage = new ChatMessage(account, example, kadu->myself(),
			tr("Message from Your friend"), TypeReceived,
			QDateTime::currentDateTime(), QDateTime::currentDateTime());
	chatMessage->setSeparatorSize(CfgHeaderSeparatorHeight);
	preview->addObjectToParse(example, chatMessage);
}

void ChatStylesManager::styleChangedSlot(const QString &styleName)
{
	ChatStyleEngine *engine = availableStyles[styleName].engine;
	editButton->setEnabled(engine->supportEditing());
	deleteButton->setEnabled(!availableStyles[styleName].global);
	variantListCombo->clear();
	variantListCombo->addItem("Default");
	variantListCombo->addItems(engine->styleVariants(styleName));
	variantListCombo->setEnabled(engine->supportVariants());
	engine->prepareStylePreview(preview, styleName, variantListCombo->currentText());
}

void ChatStylesManager::variantChangedSlot(const QString &variantName)
{
	availableStyles[syntaxListCombo->currentText()].engine->prepareStylePreview(preview, syntaxListCombo->currentText(), variantName);
}

void ChatStylesManager::editStyleClicked()
{
	availableStyles[syntaxListCombo->currentText()].engine->styleEditionRequested(syntaxListCombo->currentText());
}

void ChatStylesManager::deleteStyleClicked()
{
	QString styleName = syntaxListCombo->currentText();
	if (availableStyles[styleName].engine->removeStyle(styleName))
	{
		availableStyles.remove(styleName);
		syntaxListCombo->removeItem(syntaxListCombo->currentIndex());
		styleChangedSlot(*(availableStyles.keys().begin()));
	}
	else
		MessageBox::msg(tr("Unable to remove style: %1").arg(styleName), true, "Warning");
}

void ChatStylesManager::syntaxUpdated(const QString &syntaxName)
{
	if (!availableStyles.contains(syntaxName))
		return;

	if (syntaxListCombo && syntaxListCombo->currentText() == syntaxName)
		styleChangedSlot(syntaxName);

	if (CurrentEngine->currentStyleName() == syntaxName)
		CurrentEngine->loadTheme(syntaxName, variantListCombo->currentText());
}

void ChatStylesManager::addStyle(const QString &syntaxName, ChatStyleEngine *engine)
{
	if (availableStyles.contains(syntaxName))
		return;

	availableStyles[syntaxName].engine = engine;
	availableStyles[syntaxName].global = false;

	if (syntaxListCombo)
		syntaxListCombo->addItem(syntaxName);
}

void ChatStylesManager::configurationWindowDestroyed()
{
	syntaxListCombo = 0;
}