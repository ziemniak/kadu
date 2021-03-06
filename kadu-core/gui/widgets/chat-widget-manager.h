/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHAT_WIDGET_MANAGER
#define CHAT_WIDGET_MANAGER

#include <QtCore/QTimer>

#include "configuration/configuration-aware-object.h"
#include "configuration/storable-string-list.h"
#include "contacts/contact-list.h"
#include "gui/widgets/chat-widget.h"

#include "exports.h"

class ActionDescription;
class ChatWidgetActions;
class Protocol;

/**
	Klasa pozwalaj�ca zarz�dza� otwartymi oknami rozm�w: otwiera�,
	zamykac, szuka� okna ze wgl�du na list� u�ytkownik�w itp.
	\class ChatManagerOld
	\brief Klasa zarz�dzaj�ca oknami ChatWidget
**/

class KADUAPI ChatWidgetManager : public QObject, ConfigurationAwareObject, StorableStringList
{
	Q_OBJECT
	Q_DISABLE_COPY(ChatWidgetManager)

	static ChatWidgetManager *Instance;

	ChatWidgetActions *Actions;

	QHash<Chat *, ChatWidget *> Chats;
	QList<Chat *> ClosedChats; /*!< u�ytkownicy, kt�rych okna zosta�y zamkni�te*/
	QList<QDateTime> ClosedChatsDates;

	ChatWidgetManager();
	virtual ~ChatWidgetManager();

	void autoSendActionCheck();
	void insertEmoticonActionEnabled();

private slots:
	void openChatWith();
	void clearClosedChats();

	void messageReceived(const Message &message);
	void messageSent(const Message &message);

protected:
	virtual void configurationUpdated();

public:
	static ChatWidgetManager * instance();

	ChatWidgetActions * actions() { return Actions; }

	const QHash<Chat *, ChatWidget *> & chats() const;
	const QList<Chat *> closedChats() const;

	ChatWidget * byChat(Chat *chat, bool create = false) const;

	virtual void load();
	virtual void store();

	void activateChatWidget(ChatWidget *chatWidget, bool forceActivate);

public slots:
	ChatWidget * openChatWidget(Chat *chat, bool forceActivate = false);

	/**
		\fn void openPendingMsgs(ContactList users)
		Funkcja wpisuje zakolejkowane wiadomo�ci do okna
		z u�ytkownikami "users"
		\param users lista u�ytkownik�w identyfikuj�cych okno
	**/
	void openPendingMsgs(Chat *chat, bool forceActivate = false);

	/**
		\fn void openPendingMsgs()
		Funkcja wpisuje wszystkie zakolejkowane wiadomo�ci
		do odpowiednich okien
	**/
	void openPendingMsgs(bool forceActivate = false);

	/**
		\fn void deletePendingMsgs(ContactList users)
		Funkcja usuwa zakolejkowane wiadomo�ci
		z u�ytkownikami "users"
		\param users lista u�ytkownik�w identyfikuj�cych okno
	**/
	void deletePendingMsgs(Chat *chat);

	void sendMessage(Chat *chat);

	/**
		\fn void closeAllWindows()
		Funkcja zamyka wszystkie okna chat
	**/
	void closeAllWindows();

	/**
		\fn int registerChatWidget(ChatWidget* chat)
		Dodaje okno do menad�era
		\param chat wska�nik do okna ktore chcemy doda�
		\return zwraca numer naszego okna po zarejestrowaniu
	**/
	void registerChatWidget(ChatWidget *chat);

	/**
		\fn void unregisterChat(Chat* chat)
		Funkcja wyrejestrowuje okno z managera \n
		Zapisuje w�asno�ci okna \n
		wysy�a sygna� chatDestroying i chatDestroyed
		\param chat okno kt�re b�dzie wyrejestrowane
	**/
	void unregisterChatWidget(ChatWidget *chat);

signals:
	/**
		\fn void handleNewChatWidget(ChatWidget *chat, bool &handled)
	 	Sygna� ten jest wysy�any po utworzeniu nowego okna chat.
		Je�li zmienna handled zostanie ustawiona na true, to
		niezostanie utworzony nowy obiekt ChatWindiw
		\param chat nowe okno chat
	**/
	void handleNewChatWidget(ChatWidget *chat, bool &handled);
	/**
		\fn void chatWidgetCreated(ChatWidget *chat)
	 	Sygna� ten jest wysy�any po utworzeniu nowego okna chat
		\param chat nowe okno chat
	**/
	void chatWidgetCreated(ChatWidget *chat);

	void chatWidgetActivated(ChatWidget *chat);

	/**
		\fn void chatCreated(const UserGroup *group)
	 	Sygna� ten jest wysy�any po utworzeniu nowego okna chat
		\param chat nowe okno chat
		\param time time of pending message that created a chat or 0 if not applicable
	**/
	void chatWidgetCreated(ChatWidget *chat, time_t time);

	/**
		\fn void chatDestroying(const UserGroup *group)
	 	Sygna� ten jest wysy�any przed zamnkni�ciem okna chat
		\param chat zamykane okno
	**/
	void chatWidgetDestroying(ChatWidget *chat);

	/**
		\fn void chatOpen(ContactList users)
		Sygna� ten jest wysy�aniy podczas ka�dej pr�by
		otwarcia nowego okna chat nawet je�li ju� taki istnieje
		\param chat otwarte okno
	**/
	void chatWidgetOpen(ChatWidget *chat);

	void chatWidgetTitlesUpdated();

	/**
		\fn void messageSentAndConfirmed(Chat *chat, const QString& message)
		This signal is emited when message was sent
		and it was confirmed.
		When confirmations are turned off signal is
		emited immediately after message was send.
		\param receivers list of receivers
		\param message the message
	**/
	void messageSentAndConfirmed(Chat *chat, const QString& message);

};

#endif // CHAT_WIDGET_MANAGER
