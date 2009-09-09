/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtGui/QIcon>
#include <QtGui/QWidget>

#include "accounts/accounts-aware-object.h"
#include "chat/chat.h"
#include "chat/message/formatted-message.h"
#include "configuration/configuration-aware-object.h"
#include "contacts/contact-list.h"
#include "protocols/services/chat-service.h"
#include "chat-messages-view.h"
#include "exports.h"

class QSplitter;

class Account;
class ChatEditBox;
class ChatWidget;
class ContactsListWidget;
class CustomInput;
class MessageRenderInfo;
class Protocol;

class KADUAPI ChatWidget : public QWidget, ConfigurationAwareObject, AccountsAwareObject
{
	Q_OBJECT

	friend class ChatWidgetManager;

	Chat *CurrentChat;

	ChatMessagesView *MessagesView;
	ContactsListWidget *ContactsWidget;
	ChatEditBox *InputBox;

	QIcon pix;
	QSplitter *vertSplit, *horizSplit; /*!< obiekty oddzielaj�ce kontrolki od siebie */

	QDateTime LastMessageTime; /*!< czas ostatniej wiadomo�ci */

	// TODO: remove
	int activationCount;

	unsigned int NewMessagesCount; /*!< liczba nowych wiadomo�ci w oknie rozmowy */
	bool SelectionFromMessagesView;

	void createGui();
	void createContactsList();

	void resetEditBox();

	bool decodeLocalFiles(QDropEvent *event, QStringList &files);

private slots:
	void connectAcknowledgeSlots();
	void disconnectAcknowledgeSlots();

	void editBoxKeyPressed(QKeyEvent *e, CustomInput *sender, bool &handled);

	void messagesViewSelectionChanged();
	void editBoxSelectionChanged();

protected:
	virtual void keyPressEvent(QKeyEvent *e);
	bool keyPressEventHandled(QKeyEvent *);

	virtual void configurationUpdated();

	virtual void accountRegistered(Account *account);
	virtual void accountUnregistered(Account *account);

public:
	explicit ChatWidget(Chat *chat, QWidget *parent = 0);
	virtual ~ChatWidget();

	Chat * chat() { return CurrentChat; };

	/**
		Dodaje now� wiadomos� systemow� do okna.

		@param rawContent tre�� wiadomo�ci w postaci HTML
		@param backgroundColor kolor t�a wiadomo�ci (format HTML)
		@param fontColor kolor wiadomo�ci (format HTML)
	 **/
	void appendSystemMessage(const QString &rawContent, const QString &backgroundColor, const QString &fontColor);

	/**
		\fn void newMessage(Account *account, ContactList senders, const QString &message, time_t time)
		Add new message to window

		\param account account on which the message was received
		\param senders list of sender
		\param message message content
		\param time czas
		**/
	void newMessage(MessageRenderInfo *message);

	/**
		\fn void repaintMessages()
		Od�wie�a zawarto�� okna uwzgl�dniaj�c ewentualne
		zmiany dokonane w kt�rej� wiadomo�ci z listy
		uzyskanej za pomoc� metody chatMessages(),
		dodanie nowych wiadomo�ci lub usuni�cie istniej�cych.
	**/
	void repaintMessages();

	CustomInput * edit();
	ContactsListWidget * contactsListWidget() { return ContactsWidget; }
	ChatEditBox * getChatEditBox() { return InputBox; }

	virtual void dragEnterEvent(QDragEnterEvent *e);
	virtual void dropEvent(QDropEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *e);

	virtual bool eventFilter(QObject *watched, QEvent *e);

	Protocol *currentProtocol();

	unsigned int newMessagesCount() const;

	QDateTime lastMessageTime();
	QIcon icon();

	void kaduStoreGeometry();
	void kaduRestoreGeometry();

	unsigned int countMessages() { return MessagesView->countMessages(); }

public slots:
// 	void messageStatusChanged(int messageId, ChatService::MessageStatus status);

	void onStatusChanged(Account *account, Contact contact, Status oldStatus);

	/**
		\fn void appendMessages(const QValueList<MessageRenderInfo *> &)
		Slot dodaj wiadomo�ci do okna
		\param messages lista wiadomo�ci
	**/
	void appendMessages(const QList<MessageRenderInfo *> &, bool pending = false);

	/**
	\fn void appendMessage(MessageRenderInfo *)
		Slot dodaj wiadomo�� do okna
		\param messages lista wiadomo�ci
	**/
	void appendMessage(MessageRenderInfo *, bool pending = false);

	/**
		\fn void sendMessage()
		Slot wywo�ywany po naci�ni�ciu przycisku
		do wysy�ania wiadomo�ci
	**/
	void sendMessage();

	/**
		\fn void colorSelectorAboutToClose()
		Slot zostaje wywo�any przy zamykaniu okna wyboru ikonek
	**/
	void colorSelectorAboutToClose();

	/**
		\fn void clearChatWindow()
		Slot czyszcz�cy okno rozmowy
	**/
	void clearChatWindow();

	void makeActive();
	void markAllMessagesRead();

	void leaveConference();

signals:
	/**
		\fn void messageSendRequested(Chat* chat)
		Sygnal jest emitowany gdy uzytkownik wyda polecenie
		wyslania wiadomosci, np klikajac na guzik "wyslij".
		\param chat wska�nik do okna kt�re emituje sygna�
	**/
	void messageSendRequested(ChatWidget *chat);

	/**
		\fn void messageSent(Chat* chat)
		Sygnal jest emitowany gdy zakonczy sie proces
		wysylania wiadomosci i zwiazanych z tym czynnosci.
		Oczywiscie nie wiemy czy wiadomosc dotarla.
		\param chat wska�nik do okna rozmowy,
		 kt�re emitowa�o sygna�
	**/
	void messageSent(ChatWidget *chat);

	/**
		\fn void messageSentAndConfirmed(ContactList receivers, const QString& message)
		This signal is emited when message was sent
		and it was confirmed.
		When confirmations are turned off signal is
		emited immediately after message was send
		just like messageSent() signal.
		\param receivers list of receivers
		\param message the message
	**/
	void messageSentAndConfirmed(Chat *chat, const QString &message);
	void messageReceived(Chat *chat);

	/**
		\fn void fileDropped(const UserGroup *users, const QString& fileName)
		Sygna� jest emitowany, gdy w oknie Chat
		upuszczono plik.
	\param users lista u�ytkownik�w
		\param fileName nazwa pliku
	**/
	void fileDropped(Chat *contacts, const QString &fileName);

	void captionUpdated();
	void closed();

	void keyPressed(QKeyEvent *e, ChatWidget *sender, bool &handled);

};

/**
	@class ChatContainer
	@brief Klasa abstrakcyjna opisuj�ca rodzica klasy ChatWidget.

	Klasa abstrakcyjna z kt�rej powinny dziedziczy� klasy b�d�ce rodzicami obiekt�w
	klasy ChatWidget.

	Informuje kt�ry chat powinien zosta� zamkni�ty w przypadku np. ignorowania kontaktu
	z kt�rym prowadzona jest rozmowa
**/

class ChatContainer
{
	public:
		ChatContainer() {};
		virtual ~ChatContainer() {};

		/**
			Metoda informuj�ca, kt�ry chat powinien zosta� zamkni�ty
		 **/
		virtual void closeChatWidget(ChatWidget *chat) = 0;
};

#endif // CHAT_WIDGET_H
