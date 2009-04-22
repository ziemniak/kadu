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

#include <xmpp.h>
#include <xmpp_tasks.h>

#include "action.h"
#include "accounts/account-manager.h"
#include "chat/chat_manager-old.h"
#include "config_file.h"
#include "contacts/contact-manager.h"
#include "debug.h"
#include "icons_manager.h"
#include "core/core.h"
#include "gui/windows/kadu-window.h"
#include "misc/misc.h"
#include "message_box.h"
#include "main_configuration_window.h"
#include "protocols/protocol-menu-manager.h"
#include "protocols/status.h"

#include "cert-util.h"
#include "jabber-account.h"
#include "jabber_protocol.h"
#include "jabber-protocol-factory.h"
#include "system-info.h"

extern "C" int jabber_protocol_init()
{
	return JabberProtocol::initModule();
}

extern "C" void jabber_protocol_close()
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

	loginJabberActionDescription = new ActionDescription(
		0, ActionDescription::TypeGlobal, "loginJabberAction",
		this, SLOT(loginAction(QAction *, bool)),
		"Online", "Login" //+JabberData->id()
	);
	Core::instance()->kaduWindow()->insertMenuActionDescription(loginJabberActionDescription, KaduWindow::MenuKadu,1);
	logoutJabberActionDescription = new ActionDescription(
		0, ActionDescription::TypeGlobal, "logoutJabberAction",
		this, SLOT(logoutAction(QAction *, bool)),
		"Offline", "Logout" //+JabberData->id()
	);
	Core::instance()->kaduWindow()->insertMenuActionDescription(logoutJabberActionDescription, KaduWindow::MenuKadu,1);
	InitialPresence = XMPP::Status("", "Jabber w Kadu", 5, true);

	initializeJabberClient();

	CurrentChatService = new JabberChatService(this);

	kdebugf2();
}

JabberProtocol::~JabberProtocol()
{
	Core::instance()->kaduWindow()->removeMenuActionDescription(loginJabberActionDescription);
	delete loginJabberActionDescription;
	Core::instance()->kaduWindow()->removeMenuActionDescription(logoutJabberActionDescription);
	delete logoutJabberActionDescription;
}

void JabberProtocol::initializeJabberClient()
{
	JabberClient = new XMPP::JabberClient;
	connect( JabberClient, SIGNAL ( csDisconnected () ), this, SLOT (disconnectedFromServer()) );
	connect( JabberClient, SIGNAL ( tlsWarning ( QCA::TLS::IdentityResult, QCA::Validity ) ), this, SLOT ( slotHandleTLSWarning ( QCA::TLS::IdentityResult, QCA::Validity ) ) );
	connect( JabberClient, SIGNAL ( connected () ), this, SLOT ( connectedToServer() ) );

	connect( JabberClient, SIGNAL ( subscription ( const XMPP::Jid &, const QString & ) ),
		   this, SLOT ( slotSubscription ( const XMPP::Jid &, const QString & ) ) );
	connect( JabberClient, SIGNAL ( newContact ( const XMPP::RosterItem & ) ),
		   this, SLOT ( slotContactUpdated ( const XMPP::RosterItem & ) ) );
	connect( JabberClient, SIGNAL ( contactUpdated ( const XMPP::RosterItem & ) ),
		   this, SLOT ( slotContactUpdated ( const XMPP::RosterItem & ) ) );
	connect( JabberClient, SIGNAL ( contactDeleted ( const XMPP::RosterItem & ) ),
		   this, SLOT ( slotContactDeleted ( const XMPP::RosterItem & ) ) );
	connect( JabberClient, SIGNAL ( rosterRequestFinished ( bool ) ),
		   this, SLOT ( rosterRequestFinished ( bool ) ) );

	connect( JabberClient, SIGNAL ( resourceAvailable ( const XMPP::Jid &, const XMPP::Resource & ) ),
		   this, SLOT ( clientResourceAvailable ( const XMPP::Jid &, const XMPP::Resource & ) ) );
	connect( JabberClient, SIGNAL ( resourceUnavailable ( const XMPP::Jid &, const XMPP::Resource & ) ),
		   this, SLOT ( clientResourceUnavailable ( const XMPP::Jid &, const XMPP::Resource & ) ) );

		/*//TODO: implement in the future
		connect( JabberClient, SIGNAL ( incomingFileTransfer () ),
				   this, SLOT ( slotIncomingFileTransfer () ) );
		connect( JabberClient, SIGNAL ( groupChatJoined ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatJoined ( const XMPP::Jid & ) ) );
		connect( JabberClient, SIGNAL ( groupChatLeft ( const XMPP::Jid & ) ),
				   this, SLOT ( slotGroupChatLeft ( const XMPP::Jid & ) ) );
		connect( JabberClient, SIGNAL ( groupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ),
				   this, SLOT ( slotGroupChatPresence ( const XMPP::Jid &, const XMPP::Status & ) ) );
		connect( JabberClient, SIGNAL ( groupChatError ( const XMPP::Jid &, int, const QString & ) ),
				   this, SLOT ( slotGroupChatError ( const XMPP::Jid &, int, const QString & ) ) );
		*/
	connect( JabberClient, SIGNAL ( debugMessage ( const QString & ) ),
		   this, SLOT ( slotClientDebugMessage ( const QString & ) ) );
}

void JabberProtocol::connectToServer()
{
	kdebugf();

	JabberAccount *jabberAccount = dynamic_cast<JabberAccount *>(account());
	if (jabberAccount->id().isNull() || jabberAccount->password().isNull())
	{
		setStatus(Status::Offline);

		MessageBox::msg(tr("Jabber ID or password not set!"), false, "Warning");
		kdebugmf(KDEBUG_FUNCTION_END, "end: Jabber ID or password not set\n");
		return;
	}

	
	JabberClient->disconnect();

	JabberClient->setOSName(SystemInfo::instance()->os());
	JabberClient->setTimeZone(SystemInfo::instance()->timezoneString(), SystemInfo::instance()->timezoneOffset());
	JabberClient->setClientName("Kadu");
	JabberClient->setClientVersion(VERSION);
	// we need to use the old protocol for now
	//JabberClient->setUseXMPP09 (true);

	// set SSL flag (this should be converted to forceTLS when using the new protocol)
	JabberClient->setUseSSL(false);

	// override server and port (this should be dropped when using the new protocol and no direct SSL)
	JabberClient->setOverrideHost(false);

	// allow plaintext password authentication or not?
	///gtalk
	///JabberClient->setAllowPlainTextPassword(XMPP::ClientStream::AllowPlainOverTLS);
	//JabberClient->setUseXMPP09(false);
 	//JabberClient->setForceTLS(true);

	//do tego jeszcze chyba troche daleko
	JabberClient->setFileTransfersEnabled(false);
	rosterRequestDone = false;
	usingSSL = false;
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
	//ma�e pro
	//Jid j = d->jid.withResource((d->acc.opt_automatic_resource ? localHostName() : d->acc.resource ));
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

	if (!status().isOffline())
		setStatus(Status::Offline);

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
	setPresence (s);
	InitialPresence = XMPP::Status ("", "", 5, true);

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


void JabberProtocol::slotClientDebugMessage ( const QString &msg )
{
	kdebugm(KDEBUG_WARNING, "Jabber Client debug:  %s\n", qPrintable(msg));
}

bool JabberProtocol::handleTLSWarning (XMPP::JabberClient *jabberClient, QCA::TLS::IdentityResult identityResult, QCA::Validity validityResult)
{
	QString validityString, code, idString, idCode;

	QString server    = jabberClient->jid().domain ();
	QString accountId = jabberClient->jid().bare ();

	switch ( identityResult )
	{
		case QCA::TLS::Valid:
			break;
		case QCA::TLS::HostMismatch:
			idString = tr("The host name does not match the one in the certificate.");
			idCode   = "HostMismatch";
			break;
		case QCA::TLS::InvalidCertificate:
			idString = tr("The certificate is invalid.");
			idCode   = "InvalidCert";
			break;
		case QCA::TLS::NoCertificate:
			idString = tr("No certificate was presented.");
			idCode   = "NoCert";
			break;
	}

	switch ( validityResult )
	{
		case QCA::ValidityGood:
			break;
		case QCA::ErrorRejected:
			validityString = tr("The Certificate Authority rejected the certificate.");
			code = "Rejected";
			break;
		case QCA::ErrorUntrusted:
			validityString = tr("The certificate is not trusted.");
			code = "Untrusted";
			break;
		case QCA::ErrorSignatureFailed:
			validityString = tr("The signature is invalid.");
			code = "SignatureFailed";
			break;
		case QCA::ErrorInvalidCA:
			validityString = tr("The Certificate Authority is invalid.");
			code = "InvalidCA";
			break;
		case QCA::ErrorInvalidPurpose:
			validityString = tr("Invalid certificate purpose.");
			code = "InvalidPurpose";
			break;
		case QCA::ErrorSelfSigned:
			validityString = tr("The certificate is self-signed.");
			code = "SelfSigned";
			break;
		case QCA::ErrorRevoked:
			validityString = tr("The certificate has been revoked.");
			code = "Revoked";
			break;
		case QCA::ErrorPathLengthExceeded:
			validityString = tr("Maximum certificate chain length was exceeded.");
			code = "PathLengthExceeded";
			break;
		case QCA::ErrorExpired:
			validityString = tr("The certificate has expired.");
			code = "Expired";
			break;
		case QCA::ErrorExpiredCA:
			validityString = tr("The Certificate Authority has expired.");
			code = "ExpiredCA";
			break;
		case QCA::ErrorValidityUnknown:
			validityString = tr("Validity is unknown.");
			code = "ValidityUnknown";
			break;
	}

	QString message;
	if (!idString.isEmpty())
	{
		if (!validityString.isEmpty())
		{
			message = tr(QString("<qt><p>The identity and the certificate of server %s could not be "
					"validated for account %s:</p><p>%3</p><p>%4</p><p>Do you want to continue?</p></qt>").arg(server).arg(accountId).arg(idString).arg(validityString));
		}
		else
		{
			message = tr(QString("<qt><p>The certificate of server %s could not be validated for "
					"account %s: %3</p><p>Do you want to continue?</p></qt>").arg(server).arg(accountId).arg( idString));
		}
	}
	else
	{
		message = tr(QString("<qt><p>The certificate of server %s could not be validated for "
			"account %s: %3</p><p>Do you want to continue?</p></qt>").arg(server).arg( accountId).arg( validityString));
	}

	QMessageBox* m = new QMessageBox(QMessageBox::Critical,
	(/*printAccountName*/!accountId.isEmpty() ? QString("%s: ").arg(name()) : "") + tr("Server Error"),
	tr("There was an error communicating with the server.\nDetails: %s").arg(server + idCode + code),
	QMessageBox::Ok, 0, Qt::WDestructiveClose);
	m->setModal(true);
	m->show();

	return true;
}

void JabberProtocol::slotHandleTLSWarning (
		QCA::TLS::IdentityResult identityResult,
		QCA::Validity validityResult )
{
	kdebug("Handling TLS warning...\n");

	if (handleTLSWarning( JabberClient, identityResult, validityResult ))
	{
		// resume stream
		JabberClient->continueAfterTLSWarning ();
	}
	else
	{
		// disconnect stream
		JabberClient->disconnect();
	}

}

void JabberProtocol::setPresence(const XMPP::Status &status)
{
	kdebug("Status: %s, Reason: %s\n", status.show().local8Bit().data(), status.status().local8Bit().data());

	// fetch input status
	XMPP::Status newStatus = status;

	// TODO: Check if Caps is enabled
	// Send entity capabilities
	if(JabberClient)
	{
		newStatus.setCapsNode( JabberClient->capsNode() );
		newStatus.setCapsVersion( JabberClient->capsVersion() );
		newStatus.setCapsExt( JabberClient->capsExt() );
	}

	JabberAccount *jabberAccount = dynamic_cast<JabberAccount *>(account());
	newStatus.setPriority(jabberAccount->priority());
	//TODO whatever
	XMPP::Jid jid (jabberID);
	XMPP::Resource newResource (jabberAccount->resource(), newStatus );

	// update our resource in the resource pool
	resourcePool()->addResource(jid, newResource);

	// make sure that we only consider our own resource locally
	resourcePool()->lockToResource(jid, newResource);

	/*
	 * Unless we are in the connecting status, send a presence packet to the server
	 */
	if(status.show() != QString("connecting") )
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
	setPresence(InitialPresence);
	kdebugf2();
}

void JabberProtocol::disconnectedFromServer()
{
	kdebugf();

	if (!status().isOffline())
		setStatus(Status::Offline);

	//if (!Kadu::closing())
		setAllOffline();

	networkStateChanged(NetworkDisconnected);
	kdebugf2();
}

void JabberProtocol::login()
{
	if(isConnected())
		return;
	connectToServer();
}

void JabberProtocol::changeStatus(Status status)
{
 	// TODO: add rest status options
	XMPP::Status s = XMPP::Status();

	if(status.isOnline())
		s.setType(XMPP::Status::Online);
	else if(status.isInvisible())
		s.setType(XMPP::Status::Invisible);
	else if(status.isBusy())
		s.setType(XMPP::Status::Away);
	else
		s.setType(XMPP::Status::Offline);
///WTF! kutfa, wywo�anie tego z protocol wywala kadu...
	s.setStatus(status.description());
	setPresence(s);
	statusChanged(status);
}

void JabberProtocol::clientResourceAvailable(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebugf();
	kdebug("New resource available for %s\n", jid.full().local8Bit().data());
	resourcePool()->addResource(jid, resource);
	//TODO: na razie brak lepszego miejsca na to
		Status status;
	if(resource.status().isAvailable())
		status.setType(Status::Online);
	else if(resource.status().isInvisible())
		status.setType(Status::Invisible);
	else
		status.setType(Status::Offline);

	if(resource.status().show() == "away")
		status.setType(Status::Busy);
	else if(resource.status().show() == "xa")
		status.setType(Status::Busy);
	else if(resource.status().show() == "dnd")
		status.setType(Status::Busy);
	else if(resource.status().show() == "chat")
		status.setType(Status::Online);

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

	JabberContactAccountData *data = dynamic_cast<JabberContactAccountData *>(contact.accountData(account()));

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
	kdebug("Resource now unavailable for %s\n", jid.full().local8Bit().data());
	resourcePool()->removeResource(jid, resource);
	//TODO: na razie brak lepszego miejsca na to
	Status status;
	status.setType(Status::Offline);

	if(resource.status().show() == "away")
		status.setType(Status::Busy);
	else if(resource.status().show() == "xa")
		status.setType(Status::Busy);
	else if(resource.status().show() == "dnd")
		status.setType(Status::Busy);
	else if(resource.status().show() == "chat")
		status.setType(Status::Online);

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

	JabberContactAccountData *data = dynamic_cast<JabberContactAccountData *>(contact.accountData(account()));

	if (!data)
		return;

	Status oldStatus = data->status();
	data->setStatus(status);

	emit contactStatusChanged(account(), contact, oldStatus);
	kdebugf2();
}

void JabberProtocol::slotContactUpdated(const XMPP::RosterItem & item)
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

	kdebug("New roster item: %s (Subscription: %s )\n", item.jid().full().local8Bit().data(), item.subscription().toString ().local8Bit().data());

	/*
	 * See if the contact need to be added, according to the criterias of
	 *  JEP-0162: Best Practices for Roster and Subscription Management
	 * http://www.jabber.org/jeps/jep-0162.html#contacts
	 */
	bool need_to_add=false;
	if(item.subscription().type() == XMPP::Subscription::Both || item.subscription().type() == XMPP::Subscription::To)
		need_to_add = true;
	else if( !item.ask().isEmpty() )
		need_to_add = true;
	else if( !item.name().isEmpty() || !item.groups().isEmpty() )
		need_to_add = true;

	/*
	 * See if the contact is already on our contact list
	 */
	 Contact c = ContactManager::instance()->byId(account(), item.jid().bare());
	 if(c.display().isNull())
		if(!item.name().isNull())
			c.setDisplay(item.name());
		else
			c.setDisplay(item.jid().bare());

	if(!c.isNull() && item.jid().bare() == jabberID.bare())
	{
		// don't let remove the gateway contact, eh!
		need_to_add = true;
	}

	if(need_to_add)
	{
		if (c.isAnonymous())
		{
// 			/*
// 			* No contact with this ID has been found, so Contact Manager created anonymous contact.
// 			*/
			ContactManager::instance()->addContact(c);
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
	else if(!c.isAnonymous())  //we don't need to add it, and it is in the contact list
	{
// 		Kopete::MetaContact *metaContact=c->metaContact();
// 		if(metaContact->isTemporary())
// 			return;
// 		kDebug (JABBER_DEBUG_GLOBAL) << c->contactId() <<
// 				" is on the contact list while it should not.  we are removing it.  - " << c << endl;
// 		delete c;
// 		if(metaContact->contacts().isEmpty())
// 			Kopete::ContactList::self()->removeMetaContact( metaContact );
	}

	kdebugf2();
}

void JabberProtocol::slotContactDeleted(const XMPP::RosterItem & item)
{
	kdebug("Deleting contact %s", item.jid().full().local8Bit().data());
	//TODO: usun�� z listy - tego chyba jeszcze nie ma...
}

void JabberProtocol::slotSubscription(const XMPP::Jid & jid, const QString & type)
{
	if (type == "unsubscribed")
	{
		/*
		 * Someone else removed our authorization to see them.
		 */
		kdebug("%s revoked our presence authorization", jid.full().local8Bit().data());

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

void JabberProtocol::changeSubscription(const XMPP::Jid &jid, const QString type)
{
	XMPP::JT_Presence *task = new XMPP::JT_Presence(JabberClient->rootTask());
	task->sub(jid, type);
	task->go(true);
}

void JabberProtocol::requestSubscription(const XMPP::Jid &jid)
{
	changeSubscription(jid, "subscribe");
}

void JabberProtocol::resendSubscription(const XMPP::Jid &jid)
{
	changeSubscription(jid, "subscribed");
}

void JabberProtocol::rejectSubscription(const XMPP::Jid &jid)
{
	changeSubscription(jid, "unsubscribed");
}

void JabberProtocol::loginAction(QAction *sender, bool toggled)
{
	login();
}

void JabberProtocol::logoutAction(QAction *sender, bool toggled)
{
	logout();
}

bool JabberProtocol::validateUserID(QString& uid)
{
	XMPP::Jid j = uid;
	if(j.isValid())
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

	if (newStatus.isOffline() && status().isOffline())
	{
		//if (NetworkConnecting == state())
		//	networkDisconnected(false);
		return;
	}

	if (NetworkConnecting == state())
		return;

	if (status().isOffline())
	{
		login();
		return;
	}

	//if (newStatus.isOffline())
	//	networkDisconnected(false);

	statusChanged(newStatus);
}

void JabberProtocol::changePrivateMode()
{
	changeStatus();
}

QPixmap JabberProtocol::statusPixmap(Status status)
{
	QString pixmapName(dataPath("kadu/modules/data/jabber_protocol/"));

	switch (status.type())
	{
		case Status::Online:
			pixmapName.append("online.png");
			break;
		case Status::Busy:
			pixmapName.append("away.png");
			break;
		case Status::Invisible:
			pixmapName.append("invisible.png");
			break;
		default:
			pixmapName.append("offline.png");
			break;
	}

	return icons_manager->loadPixmap(pixmapName);
}