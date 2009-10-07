/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QCheckBox>
#include <QtGui/QMessageBox>
#include <QtCrypto>

#include "libiris/include/filetransfer.h"
#include <xmpp.h>
#include <xmpp_tasks.h>

#include "accounts/account-manager.h"
#include "core/core.h"
#include "contacts/contact-manager.h"
#include "contacts/group.h"
#include "gui/widgets/chat-widget-manager.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/message-box.h"
#include "gui/windows/main-configuration-window.h"

#include "configuration/configuration-file.h"
#include "debug.h"
#include "exports.h"
#include "icons-manager.h"
#include "kadu-config.h"
#include "misc/misc.h"
#include "protocols/protocol-menu-manager.h"
#include "status/status.h"

#include "file-transfer/jabber-file-transfer.h"
#include "jabber-account.h"
#include "jabber-protocol.h"
#include "jabber-protocol-factory.h"
#include "os/generic/system-info.h"

extern "C" KADU_EXPORT int jabber_protocol_init()
{
	return JabberProtocol::initModule();
}

extern "C" KADU_EXPORT void jabber_protocol_close()
{
	JabberProtocol::closeModule();
}

int JabberProtocol::initModule()
{
	kdebugf();

	if (ProtocolsManager::instance()->hasProtocolFactory("jabber"))
		return 0;

	ProtocolsManager::instance()->registerProtocolFactory(JabberProtocolFactory::instance());

// TODO : to idzie do zmiany na nowe api
/*	if (!xml_config_file->hasNode("Accounts"))
	{
		JabberAccountData *jabberAccountData = new JabberAccountData(
				config_file.readEntry("Jabber", "UID"),
				pwHash(config_file.readEntry("Jabber", "Password"))
		);

		Account *defaultJabber = AccountManager::instance()->createAccount(
				"DefaultJabber", "jabber", jabberAccountData);
		AccountManager::instance()->registerAccount(defaultJabber);
	}*/

// 	defaultdescriptions = QStringList::split("<-->", config_file.readEntry("General","DefaultDescription", tr("I am busy.")), true);

	kdebugf2();
	return 0;
}

void JabberProtocol::closeModule()
{
	kdebugf();
	ProtocolsManager::instance()->unregisterProtocolFactory(JabberProtocolFactory::instance());
	kdebugf2();
}

JabberProtocol::JabberProtocol(Account *account, ProtocolFactory *factory): Protocol(account,factory), JabberClient(NULL), ResourcePool(0)
{
	kdebugf();

	initializeJabberClient();

	CurrentChatService = new JabberChatService(this);
	CurrentFileTransferService = new JabberFileTransferService(this);
	CurrentAvatarService = new JabberAvatarService(this);

	connect(ContactManager::instance(), SIGNAL(contactAdded(Contact &)),
			this, SLOT(contactAdded(Contact &)));
	connect(ContactManager::instance(), SIGNAL(contactRemoved(Contact &)),
			this, SLOT(contactRemoved(Contact &)));
	connect(ContactManager::instance(), SIGNAL(contactAccountDataAdded(Contact &, Account *)),
			this, SLOT(contactAccountDataAboutToBeRemoved(Contact & , Account *)));
	connect(ContactManager::instance(), SIGNAL(contactAccountDataAboutToBeRemoved(Contact &, Account *)),
			this, SLOT(contactAccountDataAboutToBeRemoved(Contact &, Account *)));
	connect(ContactManager::instance(), SIGNAL(contactUpdated(Contact &)),
			this, SLOT(contactUpdated(Contact &)));
	connect(ContactManager::instance(), SIGNAL(contactAccountIdChanged(Contact &, Account *, const QString &)),
			this, SLOT(contactAccountIdChanged(Contact &, Account *, const QString &)));

	kdebugf2();
}

JabberProtocol::~JabberProtocol()
{
	QObject::disconnect(ContactManager::instance(), SIGNAL(contactAdded(Contact &)),
			this, SLOT(contactAdded(Contact &)));
	QObject::disconnect(ContactManager::instance(), SIGNAL(contactRemoved(Contact &)),
			this, SLOT(contactRemoved(Contact &)));
	QObject::disconnect(ContactManager::instance(), SIGNAL(contactAccountDataAdded(Contact &, Account *)),
			this, SLOT(contactAccountDataAboutToBeRemoved(Contact & , Account *)));
	QObject::disconnect(ContactManager::instance(), SIGNAL(contactAccountDataAboutToBeRemoved(Contact &, Account *)),
			this, SLOT(contactAccountDataAboutToBeRemoved(Contact &, Account *)));
}

void JabberProtocol::initializeJabberClient()
{
	JabberClient = new XMPP::JabberClient(this);
	connect(JabberClient, SIGNAL(csDisconnected()), this, SLOT(disconnectedFromServer()));
	connect(JabberClient, SIGNAL(connected()), this, SLOT(connectedToServer()));

	connect(JabberClient, SIGNAL(subscription(const XMPP::Jid &, const QString &)),
		   this, SLOT(slotSubscription(const XMPP::Jid &, const QString &)));
	connect(JabberClient, SIGNAL(newContact(const XMPP::RosterItem &)),
		   this, SLOT(slotContactUpdated(const XMPP::RosterItem &)));
	connect(JabberClient, SIGNAL(contactUpdated(const XMPP::RosterItem &)),
		   this, SLOT(slotContactUpdated(const XMPP::RosterItem &)));
	connect(JabberClient, SIGNAL(contactDeleted(const XMPP::RosterItem &)),
		   this, SLOT(slotContactDeleted(const XMPP::RosterItem &)));
	connect(JabberClient, SIGNAL(rosterRequestFinished(bool)),
		   this, SLOT(rosterRequestFinished(bool)));

	connect(JabberClient, SIGNAL(resourceAvailable(const XMPP::Jid &, const XMPP::Resource &)),
		   this, SLOT(clientResourceAvailable(const XMPP::Jid &, const XMPP::Resource &)));
	connect(JabberClient, SIGNAL(resourceUnavailable(const XMPP::Jid &, const XMPP::Resource &)),
		   this, SLOT(clientResourceUnavailable(const XMPP::Jid &, const XMPP::Resource &)));

	connect(JabberClient, SIGNAL(incomingFileTransfer()), this, SLOT(slotIncomingFileTransfer()));

		/*//TODO: implement in the future
		connect( JabberClient, SIGNAL ( groupChatJoined ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatJoined ( const XMPP::Jid & ) ) );
		connect( JabberClient, SIGNAL ( groupChatLeft ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatLeft ( const XMPP::Jid & ) ) );
		connect( JabberClient, SIGNAL ( groupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ),
				   this, SLOT ( slotGroupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ) );
		connect( JabberClient, SIGNAL ( groupChatError ( const XMPP::Jid &, int, const QString & ) ),
				   this, SLOT ( slotGroupChatError ( const XMPP::Jid &, int, const QString & ) ) );
		*/
	connect(JabberClient, SIGNAL( debugMessage(const QString &)),
		   this, SLOT(slotClientDebugMessage(const QString &)));
}

void JabberProtocol::connectToServer()
{
	kdebugf();

	JabberAccount *jabberAccount = dynamic_cast<JabberAccount *>(account());
	if (jabberAccount->id().isNull() || jabberAccount->password().isNull())
	{
		setStatus(Status());

		MessageBox::msg(jabberAccount->name() + ": " + tr("Jabber ID or password not set!"), false, "Warning");
		kdebugmf(KDEBUG_FUNCTION_END, "end: Jabber ID or password not set\n");
		return;
	}
	
	JabberClient->disconnect();

	JabberClient->setOSName(SystemInfo::instance()->osFullName());
	JabberClient->setTimeZone(SystemInfo::instance()->timezone(), SystemInfo::instance()->timezoneOffset());
	JabberClient->setClientName("Kadu");
	JabberClient->setClientVersion(VERSION);

	// Set caps node information
	JabberClient->setCapsNode("http://psi-im.org/caps");
	JabberClient->setCapsVersion("0.12");

	JabberClient->setForceTLS(jabberAccount->encryptionMode() != JabberAccount::Encryption_No);

	// override server and port (this should be dropped when using the new protocol and no direct SSL)
	JabberClient->setUseSSL(jabberAccount->encryptionMode() == JabberAccount::Encryption_Legacy);
	JabberClient->setOverrideHost(jabberAccount->useCustomHostPort(), jabberAccount->customHost(), jabberAccount->customPort());

	// allow plaintext password authentication or not?
	///gtalk
	///JabberClient->setAllowPlainTextPassword(XMPP::ClientStream::AllowPlainOverTLS);

	JabberClient->setFileTransfersEnabled(true); // i haz it
	rosterRequestDone = false;
	jabberID = jabberAccount->id();

/*
//TODO: do nowej klasy dostosowa?
	XMPP::AdvancedConnector::Proxy p;
	if(config_file.readBoolEntry("Network", "UseProxy"))
	{
		p.setHttpConnect(config_file.readEntry("Network", "ProxyHost"), config_file.readNumEntry("Network", "ProxyPort"));
		if (!config_file.readEntry("Network", "ProxyUser").isEmpty())
			p.setUserPass(config_file.readEntry("Network", "ProxyUser"), config_file.readEntry("Network", "ProxyPassword"));
	}
	connector = new XMPP::AdvancedConnector;
	if(confUseSSL && QCA::isSupported("tls"))
	{
		tls = new QCA::TLS;
		tls->setTrustedCertificates(CertUtil::allCertificates());
		tlsHandler = new XMPP::QCATLSHandler(tls);
		tlsHandler->setXMPPCertCheck(true);
		connect(tlsHandler, SIGNAL(tlsHandshaken()), SLOT(tlsHandshaken()));
	}
	connector->setProxy(p);
	connector->setOptHostPort(host, port);
	connector->setOptSSL(confUseSSL);

	stream = new XMPP::ClientStream(connector, tlsHandler);
	stream->setAllowPlain(XMPP::ClientStream::AllowPlainOverTLS);

*/
	whileConnecting = true;
	networkStateChanged(NetworkConnecting);
	jabberID = jabberID.withResource(jabberAccount->resource());
	networkStateChanged(NetworkConnecting);
	JabberClient->connect(jabberID, jabberAccount->password(), true);
	kdebugf2();
}

void JabberProtocol::connectedToServer()
{
	kdebugf();

	whileConnecting = false;
	//crash by�... gdzie indziej status ustawia�
	///setStatus(Status::Online);
	networkStateChanged(NetworkConnected);
	//po zalogowaniu pobierz roster
	JabberClient->requestRoster();
	kdebugf2();
}

// disconnect or stop reconnecting
void JabberProtocol::logout(const XMPP::Status &s)
{
	kdebugf();

	if (!status().isDisconnected())
		setStatus(Status());

	setAllOffline();
	disconnect();

	kdebugf2();
}

void JabberProtocol::disconnect(const XMPP::Status &s)
{
	kdebugf();
	kdebug("disconnect() called\n");

	if (isConnected())
	{
		kdebug("Still connected, closing connection...\n");
		/* Tell backend class to disconnect. */
		JabberClient->disconnect();
	}

	// make sure that the connection animation gets stopped if we're still
	// in the process of connecting
	setPresence(s);

	/* FIXME:
	 * We should delete the JabberClient instance here,
	 * but active timers in Iris prevent us from doing so.
	 * (in a failed connection attempt, these timers will
	 * try to access an already deleted object).
	 * Instead, the instance will lurk until the next
	 * connection attempt.
	 */
	kdebug("Disconnected.\n");
	networkStateChanged(NetworkDisconnected);
	kdebugf2();
}


void JabberProtocol::slotClientDebugMessage(const QString &msg)
{
	kdebugm(KDEBUG_WARNING, "Jabber Client debug:  %s\n", qPrintable(msg));
}

void JabberProtocol::setPresence(const XMPP::Status &status)
{
	kdebug("Status: %s, Reason: %s\n", status.show().toLocal8Bit().data(), status.status().toLocal8Bit().data());

	// fetch input status
	XMPP::Status newStatus = status;

	// TODO: Check if Caps is enabled
	// Send entity capabilities
	if (JabberClient)
	{
		newStatus.setCapsNode(JabberClient->capsNode());
		newStatus.setCapsVersion(JabberClient->capsVersion());
		newStatus.setCapsExt(JabberClient->capsExt());
	}

	JabberAccount *jabberAccount = dynamic_cast<JabberAccount *>(account());
	newStatus.setPriority(jabberAccount->priority());
	//TODO whatever
	XMPP::Jid jid(jabberID);
	XMPP::Resource newResource(jabberAccount->resource(), newStatus);

	// update our resource in the resource pool
	resourcePool()->addResource(jid, newResource);

	// make sure that we only consider our own resource locally
	resourcePool()->lockToResource(jid, newResource);

	/*
	 * Unless we are in the connecting status, send a presence packet to the server
	 */
	if (status.show() != QString("connecting"))
	{
		/*
		 * Make sure we are actually connected before sending out a packet.
		 */
		if (isConnected())
		{
			kdebug("Sending new presence to the server.");

			XMPP::JT_Presence * task = new XMPP::JT_Presence(JabberClient->rootTask());
			task->pres(newStatus);
			task->go(true);
		}
		else
			kdebug("We were not connected, presence update aborted.");
	}

}


void JabberProtocol::rosterRequestFinished(bool success)
{
	kdebugf();
	if (success)
	{
		// the roster was imported successfully, clear
		// all "dirty" items from the contact list
		///contactPool()->cleanUp ();
	}
	rosterRequestDone = true;

	/* Since we are online now, set initial presence. Don't do this
	* before the roster request or we will receive presence
	* information before we have updated our roster with actual
	* contacts from the server! (Iris won't forward presence
	* information in that case either). */
	kdebug("Setting initial presence...\n");

	changeStatus(nextStatus());

	kdebugf2();
}

void JabberProtocol::disconnectedFromServer()
{
	kdebugf();

	if (!status().isDisconnected())
		setStatus(Status());

	//if (!Kadu::closing())
		setAllOffline();

	networkStateChanged(NetworkDisconnected);
	kdebugf2();
}

void JabberProtocol::login()
{
	if (isConnected())
		return;
	connectToServer();
}

void JabberProtocol::changeStatus(Status status)
{
	XMPP::Status s = XMPP::Status();
	const QString &type = status.type();

	if ("Online" == type)
		s.setType(XMPP::Status::Online);
	else if ("FreeForChat" == type)
		s.setType(XMPP::Status::FFC);
	else if ("DoNotDisturb" == type)
		s.setType(XMPP::Status::DND);
	else if ("NotAvailable" == type)
		s.setType(XMPP::Status::XA);
	else if ("Away" == type)
		s.setType(XMPP::Status::Away);
	else if ("Invisible" == type)
		s.setType(XMPP::Status::Invisible);
	else
		s.setType(XMPP::Status::Offline);
///WTF! kutfa, wywo�anie tego z protocol wywala kadu...
	s.setStatus(status.description());
	setPresence(s);

	if (status.isDisconnected())
	{
		networkStateChanged(NetworkDisconnected);

		setAllOffline();

		if (!nextStatus().isDisconnected())
			setStatus(Status());
	}

	statusChanged(status);
}

void JabberProtocol::slotIncomingFileTransfer()
{
	JabberFileTransfer *jft = new JabberFileTransfer(account(),
			FileTransfer::TypeReceive, client()->fileTransferManager()->takeIncoming());

	CurrentFileTransferService->incomingFile(jft);
}

void JabberProtocol::clientResourceAvailable(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebugf();
	kdebug("New resource available for %s\n", jid.full().toLocal8Bit().data());
	resourcePool()->addResource(jid, resource);
	//TODO: na razie brak lepszego miejsca na to
	Status status;
	if (resource.status().isAvailable())
		status.setType("Online");
	else if (resource.status().isInvisible())
		status.setType("Invisible");
	else
		status.setType("Offline");

	if (resource.status().show() == "away")
		status.setType("Away");
	else if (resource.status().show() == "xa")
		status.setType("NotAvailable");
	else if (resource.status().show() == "dnd")
		status.setType("DoNotDisturb");
	else if (resource.status().show() == "chat")
		status.setType("FreeForChat");

	QString description = resource.status().status();
	description.replace("\r\n", "\n");
	description.replace("\r", "\n");
	status.setDescription(description);

	Contact contact = account()->getContactById(jid.bare());
	/* is this contact realy anonymous? - need deep check
 	if (contact.isAnonymous())
	{
		// TODO - ignore! - przynajmniej na razie
		// emit userStatusChangeIgnored(contact);
		// userlist->addUser(contact);
		return;
	}
	*/
	
	if (contact.display().isEmpty())
		contact.setDisplay(jid.bare());

	JabberContactAccountData *data = jabberContactAccountData(contact);

	if (!data)
		return;

	Status oldStatus = data->status();
	data->setStatus(status);

	emit contactStatusChanged(account(), contact, oldStatus);
	kdebugf2();
}

void JabberProtocol::clientResourceUnavailable(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebugf();
	kdebug("Resource now unavailable for %s\n", jid.full().toLocal8Bit().data());
	resourcePool()->removeResource(jid, resource);
	//TODO: na razie brak lepszego miejsca na to
	Status status;
	if (resource.status().isAvailable())
		status.setType("Online");
	else if (resource.status().isInvisible())
		status.setType("Invisible");
	else
		status.setType("Offline");

	if (resource.status().show() == "away")
		status.setType("Away");
	else if (resource.status().show() == "xa")
		status.setType("NotAvailable");
	else if (resource.status().show() == "dnd")
		status.setType("DoNotDisturb");
	else if (resource.status().show() == "chat")
		status.setType("FreeForChat");

	QString description = resource.status().status();
	description.replace("\r\n", "\n");
	description.replace("\r", "\n");
	status.setDescription(description);

	Contact contact = account()->getContactById(jid.bare());
 	if (contact.isAnonymous())
	{
		// TODO - ignore! - przynajmniej na razie
		// emit userStatusChangeIgnored(contact);
		// userlist->addUser(contact);
		return;
	}
	if (contact.display().isEmpty())
		contact.setDisplay(jid.bare());

	JabberContactAccountData *data = jabberContactAccountData(contact);

	if (!data)
		return;

	Status oldStatus = data->status();
	data->setStatus(status);

	emit contactStatusChanged(account(), contact, oldStatus);
	kdebugf2();
}

void JabberProtocol::contactAdded(Contact &contact)
{
	JabberContactAccountData *jcad = jabberContactAccountData(contact);
	if (!jcad)
		return;
	QStringList groupsList;
	foreach (Group *group, contact.groups())
		groupsList.append(group->name());
	//TODO opcja żądania autoryzacji, na razie na sztywno true
	JabberClient->addContact(jcad->id(), contact.display(), groupsList, true);
}

void JabberProtocol::contactRemoved(Contact &contact)
{
	JabberContactAccountData *jcad = jabberContactAccountData(contact);
	if (!jcad || !isConnected())
		return;
	JabberClient->removeContact(jcad->id());
	if (contact.accountDatas().count() == 1)
	{
		contact.setType(ContactData::TypeAnonymous);
	}
}

void JabberProtocol::contactUpdated(Contact &contact)
{
	JabberContactAccountData *jcad = jabberContactAccountData(contact);
	if (!jcad)
		return;
	QStringList groupsList;
	foreach (Group *group, contact.groups())
		groupsList.append(group->name());
	JabberClient->updateContact(jcad->id(), contact.display(), groupsList);

}

void JabberProtocol::contactAccountDataAdded(Contact &contact, Account *contactAccount)
{
	if (contactAccount != account())
		return;
	contactAdded(contact);
}

void JabberProtocol::contactAccountDataAboutToBeRemoved(Contact &contact, Account *contactAccount)
{
	if (contactAccount != account())
		return;
	contactRemoved(contact);
}


void JabberProtocol::contactAccountIdChanged(Contact &contact, Account *account, const QString &oldId)
{
	contactUpdated(contact);
}

void JabberProtocol::slotContactUpdated(const XMPP::RosterItem &item)
{
	kdebugf();
	/**
	 * Subscription types are: Both, From, To, Remove, None.
	 * Both:   Both sides have authed each other, each side
	 *         can see each other's presence
	 * From:   The other side can see us.
	 * To:     We can see the other side. (implies we are
	 *         authed)
	 * Remove: Other side revoked our subscription request.
	 *         Not to be handled here.
	 * None:   No subscription.
	 *
	 * Regardless of the subscription type, we have to add
	 * a roster item here.
	 */

	kdebug("New roster item: %s (Subscription: %s )\n", item.jid().full().toLocal8Bit().data(), item.subscription().toString().toLocal8Bit().data());

	/*
	 * See if the contact need to be added, according to the criterias of
	 *  JEP-0162: Best Practices for Roster and Subscription Management
	 * http://www.jabber.org/jeps/jep-0162.html#contacts
	 */
	bool need_to_add=false;
	if (item.subscription().type() == XMPP::Subscription::Both || item.subscription().type() == XMPP::Subscription::To)
		need_to_add = true;
	else if (!item.ask().isEmpty())
		need_to_add = true;
	else if (!item.name().isEmpty() || !item.groups().isEmpty())
		need_to_add = true;

	if (item.jid().bare() == jabberID.bare())
	{
		// don't let remove the gateway contact, eh!
		need_to_add = true;
	}

	if (need_to_add)
	{
		/*
		 * See if the contact is already on our contact list
		 * if not add contact to our list
		 */
		 Contact c = ContactManager::instance()->byId(account(), item.jid().bare());
		 if (c.isAnonymous())
		 {
			c.setType(ContactData::TypeNormal);

			if (!item.name().isNull())
				c.setDisplay(item.name());
			else
				c.setDisplay(item.jid().bare());
		}

		if (c.isAnonymous())
		{
			// add this contact to all groups the contact is a member of
			///foreach (QString group, item.groups())
				///TODO: doda� go do grupy - na razie nie wiem jak to si� robi
		}
		else
		{
			//TODO: synchronize groups
		}

		/*
		* Add / update the contact in contact list. In case the contact is already there,
		* it will be updated. In case the contact is not there yet, it
		* will be added to it.
		*/
		///JabberContact *contact = contactPool()->addContact ( item, metaContact, false );

		/*
		* Set authorization property
		*/
		/**if ( !item.ask().isEmpty () )
// 		{
// 			contact->setProperty ( protocol()->propAuthorizationStatus, i18n ( "Waiting for authorization" ) );
// 		}
// 		else
// 		{
// 			contact->removeProperty ( protocol()->propAuthorizationStatus );
// 		}*/
	}
//	else if (!c.isAnonymous())  //we don't need to add it, and it is in the contact list
//	{
// 		Kopete::MetaContact *metaContact=c->metaContact();
// 		if(metaContact->isTemporary())
// 			return;
// 		kDebug (JABBER_DEBUG_GLOBAL) << c->contactId() <<
// 				" is on the contact list while it should not.  we are removing it.  - " << c << endl;
// 		delete c;
// 		if(metaContact->contacts().isEmpty())
// 			Kopete::ContactList::self()->removeMetaContact( metaContact );
//	}

	kdebugf2();
}

void JabberProtocol::slotContactDeleted(const XMPP::RosterItem &item)
{
	kdebug("Deleting contact %s", item.jid().full().toLocal8Bit().data());
	//TODO: usun�� z listy - tego chyba jeszcze nie ma...
}

void JabberProtocol::slotSubscription(const XMPP::Jid & jid, const QString &type)
{
	if (type == "unsubscribed")
	{
		/*
		 * Someone else removed our authorization to see them.
		 */
		kdebug("%s revoked our presence authorization", jid.full().toLocal8Bit().data());

		XMPP::JT_Roster *task;
		if (MessageBox::ask(tr("The user %1 removed subscription to you. "
								   "You will no longer be able to view his/her online/offline status. "
								   "Do you want to delete the contact?").arg(jid.full())))
		{
			/*
			 * Delete this contact from our roster.
			 */
			task = new XMPP::JT_Roster(JabberClient->rootTask());
			task->remove(jid);
			task->go(true);
			//TODO: usuwa� kontakt z listy
		}
		else
			/*
				 * We want to leave the contact in our contact list.
				 * In this case, we need to delete all the resources
				 * we have for it, as the Jabber server won't signal us
				 * that the contact is offline now.
			*/
		resourcePool()->removeAllResources(jid);
	}

	if (type == "subscribe")
	{
		/*
		* Authorize user.
		*/
		if (MessageBox::ask(tr("The user %1 wants to add you to his contact list.\n Do you agree?").arg(jid.full())))
		{
			ContactManager::instance()->byId(account(), jid.bare());
			XMPP::JT_Presence *task = new XMPP::JT_Presence(JabberClient->rootTask());
			task->sub(jid, "subscribed");
			task->go(true);
		}
	}
	else if (type == "subscribed")
		MessageBox::msg(QString("You are authorized by %1").arg(jid.bare()), false, "Warning");
	else if (type == "unsubscribe")
		MessageBox::msg(QString("Contact %1 has removed authorization for you.").arg(jid.bare()), false, "Warning");
		//TODO: usuwa� kontakt z listy... ta, chyba tak

}

bool JabberProtocol::validateUserID(QString& uid)
{
	//TODO: this does not work
	XMPP::Jid j = uid;
	if (j.isValid())
		return true;
	else
		return false;
}

JabberResourcePool *JabberProtocol::resourcePool()
{
	if (!ResourcePool)
		ResourcePool = new JabberResourcePool(this);
	return ResourcePool;
}

void JabberProtocol::changeStatus()
{
	Status newStatus = nextStatus();

	if (newStatus.isDisconnected() && status().isDisconnected())
	{
		networkStateChanged(NetworkDisconnected);

		setAllOffline();

		if (!nextStatus().isDisconnected())
			setStatus(Status());
		return;
	}

	if (NetworkConnecting == state())
		return;

	if (status().isDisconnected())
	{
		login();
		return;
	}

	changeStatus(newStatus);
}

void JabberProtocol::changePrivateMode()
{
	changeStatus();
}

QPixmap JabberProtocol::statusPixmap(Status status)
{
	QString pixmapName(dataPath("kadu/modules/data/jabber_protocol/"));

	QString groupName = status.type();

	if ("Online" == groupName)
		pixmapName.append("online");

	else if ("FreeForChat" == groupName)
		pixmapName.append("ffc");

	else if ("DoNotDisturb" == groupName)
		pixmapName.append("dnd");

	else if ("Away" == groupName)
		pixmapName.append("away");

	else if ("NotAvailable" == groupName)
		pixmapName.append("xa");

	else if ("Invisible" == groupName)
		pixmapName.append("invisible");

	else	pixmapName.append("offline");

	pixmapName.append(".png");

	return IconsManager::instance()->loadPixmap(pixmapName);
}

QPixmap JabberProtocol::statusPixmap(const QString &statusType)
{
	QString pixmapName(dataPath("kadu/modules/data/jabber_protocol/"));

	if ("Online" == statusType)
		pixmapName.append("online");

	else if ("FreeForChat" == statusType)
		pixmapName.append("ffc");

	else if ("DoNotDisturb" == statusType)
		pixmapName.append("dnd");

	else if ("Away" == statusType)
		pixmapName.append("away");

	else if ("NotAvailable" == statusType)
		pixmapName.append("xa");

	else if ("Invisible" == statusType)
		pixmapName.append("invisible");

	else	pixmapName.append("offline");

	pixmapName.append(".png");

	return IconsManager::instance()->loadPixmap(pixmapName);
}

JabberContactAccountData * JabberProtocol::jabberContactAccountData(Contact contact) const
{
	return dynamic_cast<JabberContactAccountData *>(contact.accountData(account()));
}
