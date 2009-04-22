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
#include "chat/message/message.h"

#include "contacts/contact-list.h"

#include "protocols/services/chat-service.h"

#include "chat_messages_view.h"
#include "configuration_aware_object.h"

#include "exports.h"

class QSplitter;

class Account;
class ChatEditBox;
class ChatMessage;
class ChatWidget;
class ColorSelector;
class ContactsListWidget;
class CustomInput;
class EmoticonSelector;
class KaduAction;
class Protocol;

typedef QList<ChatWidget *> ChatList;

class KADUAPI ChatWidget : public QWidget, ConfigurationAwareObject, AccountsAwareObject
{
	Q_OBJECT

private:
	friend class ChatManagerOld;

	Chat *CurrentChat;

	QString Caption; /*!< tytu� okna */
	QString EscapedCaption;



	int index;	/*!< nr okna (z chat menad�era) */
	QColor actcolor; /*!< zmienna przechowuj�ca aktualny kolor */
	QIcon pix;
//	Q3MimeSourceFactory *bodyformat; /*!< zmienna ustawiaj�ca format */
	EmoticonSelector *emoticon_selector; /*!< okienko do wyboru emotikonek */
	ColorSelector *color_selector; /*!< okienko do wyboru koloru */
	bool AutoSend; /*!< okre�la czy Return powoduje wys�anie wiadomo�ci */
	bool WaitingForACK;
	ContactsListWidget *ContactsWidget; /*!< lista kontakt�w przydatna gdy jeste�my w konferencji */
	Message myLastMessage; /*!< zmienna przechowuj�ca nasz� ostatni� wiadomo�� */
	QSplitter *vertSplit, *horizSplit; /*!< obiekty oddzielaj�ce kontrolki od siebie */

	QDateTime lastMsgTime; /*!< czas ostatniej wiadomo�ci */

	ChatEditBox *Edit; /*!< okno do wpisywania wiadomo�ci */
	ChatMessagesView *body; /*!< historia rozmowy, prosz� NIE dawa� dost�pu na zewn�trz do tej zmiennej */

	// TODO: remove
	int activationCount;

	unsigned int NewMessagesCount; /*!< liczba nowych wiadomo�ci w oknie rozmowy */

	bool decodeLocalFiles(QDropEvent *event, QStringList &files);

	void changeSendToCancelSend();
	void changeCancelSendToSend();

private slots:
	/**
		\fn void changeColor()
		Slot zmieniaj�cy kolor czcionki
	**/
	void changeColor(const QWidget *activating_widget);

	/**
		\fn void connectAcknowledgeSlots()
		Slot pod��czaj�cy potwierdzenie dostarczenia wiadomo�ci
	**/
	void connectAcknowledgeSlots();

	/**
		\fn void disconnectAcknowledgeSlots()
		Slot od��czaj�cy potwierdzenie dostarczenia wiadomo�ci
	**/
	void disconnectAcknowledgeSlots();

	/**
		\fn void colorSelectorActionCreated(KaduAction *action)
		Slot jest wywo�ywany, po dodaniu do paska narz�dzi akcji zmieniaj�cej
		kolor wiadomo�ci. U�ywa setActColor() wymuszaj�c od�wie�enie koloru na przycisku.
		Metoda jest wywoływana jedynia dla dodanych akcji po otwarciu chata.
		Przy tworzeniu chata metoda setActColor() wywoływana jest z poziomu configurationUpdated()
	**/
	void colorSelectorActionCreated(KaduAction *action);

	/**
		\fn void setActColor()
		\param force wymuszenie zmiany
		Ustawia poprawny kolor na przycisku akcji obs�uguj�cej kolor wiadomo�ci
	**/
	void setActColor(bool force);

	void keyPressedSlot(QKeyEvent *e, CustomInput *sender, bool &handled);

protected:
	virtual void keyPressEvent(QKeyEvent* e);
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
	void newMessage(Account* account, Contact sender, ContactSet receipients, const QString& message, time_t time);

	/**
		\fn void repaintMessages()
		Od�wie�a zawarto�� okna uwzgl�dniaj�c ewentualne
		zmiany dokonane w kt�rej� wiadomo�ci z listy
		uzyskanej za pomoc� metody chatMessages(),
		dodanie nowych wiadomo�ci lub usuni�cie istniej�cych.
	**/
	void repaintMessages();

	/**
		\fn const QString& title() const
		Zwraca aktualny tytu� okna
	**/
	const QString & caption() const;
	const QString & escapedCaption() const;

	/**
		\fn CustonInput* edit()
		Zwraca wska�nik do okna wpisywania wiadomo�ci
	**/
	CustomInput * edit();

	/**
		\fn UserBox* userbox()
		Zwraca wska�nik do userboxa konferencji, je�li on istnieje
	**/
	ContactsListWidget * contactsListWidget() { return ContactsWidget; }

	ChatEditBox * getChatEditBox() { return Edit; }

	/**
		\fn bool autoSend() const
		Zwraca warto�� okre�laj�c�, czy Return powoduje
		wys�anie wiadomo�ci.
	**/
	bool autoSend() const;

	bool waitingForACK() const;

	virtual void dragEnterEvent(QDragEnterEvent *e);
	virtual void dropEvent(QDropEvent *e);
	virtual void dragMoveEvent(QDragMoveEvent *e);

	virtual bool eventFilter(QObject *watched, QEvent *e);

	Protocol *currentProtocol();

	unsigned int newMessagesCount() const;

	QDateTime getLastMsgTime();
	QIcon icon();

	void kaduStoreGeometry();
	void kaduRestoreGeometry();

	unsigned int countMessages() { return body->countMessages(); }

public slots:
	/**
		\fn void refreshTitle()
		Slot ustawiaj�cy tytu� okna zgodnie z konfiguracj�
	**/
	void refreshTitle();

	/**
		\fn void addEmoticon(QString)
		Slot dodaje ikonke do pola wpisywania wiadomo�ci
		\param emot ikonka np. <lol>
	**/
	void addEmoticon(QString);

	void messageStatusChanged(int messageId, ChatService::MessageStatus status);

	void onStatusChanged(Account *account, Contact contact, Status oldStatus);

	/**
		\fn void appendMessages(const QValueList<ChatMessage *> &)
		Slot dodaj wiadomo�ci do okna
		\param messages lista wiadomo�ci
	**/
	void appendMessages(const QList<ChatMessage *> &, bool pending = false);

	/**
		\fn void appendMessage(ChatMessage *)
		Slot dodaj wiadomo�� do okna
		\param messages lista wiadomo�ci
	**/
	void appendMessage(ChatMessage *, bool pending = false);

	/**
		\fn void sendMessage()
		Slot wywo�ywany po naci�ni�ciu przycisku
		do wysy�ania wiadomo�ci
	**/
	void sendMessage();

	/**
		\fn void cancelMessage()
		Slot wywo�ywany po naci�nieciu przycisku anulowania
		wysy�aniu wiadomo�ci
	**/
	void cancelMessage();

	/**
		\fn void writeMyMessage()
		Slot wpisuj�cy wiadomo�� do okna
		\see sendMessage
	**/
	void writeMyMessage();

	/**
		\fn void curPosChanged(int para, int pos)
		Slot wywo�ywany kiedy pozycja kursosa zmieni�a si�
	**/
	void curPosChanged();

	/**
		\fn void specialKeyPressed(int key)
		Slot zostaje wywo�any gdy naci�ni�to specjalny klawisz (skr�t)
		odpowiadaj�cy np KEY_BOLD
		\param key warto�� naci�ni�tego specjalnego klawisza
	**/
	void specialKeyPressed(int key);

	/**
		\fn void colorChanged(const QColor& color)
		Slot zostaje wywo�wany gdy u�ytkownik zmieni� kolor czcionki
		kt�r� b�dzie pisa�
		\param color warto�� zmienionego koloru
	**/
	void colorChanged(const QColor& color);

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

	/**
		\fn void setAutoSend(bool auto_send)
		Ustaw autowysylanie
	**/
	void setAutoSend(bool auto_send);

	/**
		\fn void emoticonSelectorClicked()
		Slot wy�wietlaj�cy okno z wyborem emotikonek
	**/
	void openEmoticonSelector(const QWidget *activating_widget);

	/**
		\fn void insertImage()
		Slot wklejaj�cy obrazek do okna rozmowy (wyb�r obrazka)
	**/
	void insertImage();

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