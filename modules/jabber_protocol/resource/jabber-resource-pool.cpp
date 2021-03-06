 /*
  * jabberresourcepool.cpp
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */
  
//Kadu include
#include "debug.h"

#include "jabber-resource-pool.h"
#include "jabber-resource.h"
#include "jabber-protocol.h"
#include <QtCore/QList>
// #include "jabbercapabilitiesmanager.h"

/**
 * This resource will be returned if no other resource
 * for a given JID can be found. It's an empty offline
 * resource.
 */
XMPP::Resource JabberResourcePool::EmptyResource("", XMPP::Status("", "", 0, false));

JabberResourcePool::JabberResourcePool(JabberProtocol *protocol)
	: protocol(protocol), pool(), lockList()
{
}

JabberResourcePool::~JabberResourcePool()
{
	// Delete all resources in the pool upon removal
	qDeleteAll(pool);
}

void JabberResourcePool::slotResourceDestroyed (QObject *sender)
{
	kdebug("Resource has been destroyed, collecting the pieces.");

	JabberResource *oldResource = static_cast<JabberResource *>(sender);

	// remove this resource from the lock list if it existed
	lockList.removeAll(oldResource);
}

void JabberResourcePool::slotResourceUpdated(JabberResource *resource)
{
/*	QList<JabberBaseContact*> list = protocol->contactPool()->findRelevantSources(resource->jid());

	foreach(JabberBaseContact *mContact, list)
	{
		mContact->updateResourceList();
	}

	// Update capabilities
	if( !resource->resource().status().capsNode().isEmpty())
	{
		kdebug(JABBER_DEBUG_GLOBAL) << "Updating capabilities for JID: " << resource->jid().full();
		protocol->protocol()->capabilitiesManager()->updateCapabilities( protocol, resource->jid(), resource->resource().status());
	}*/
}

void JabberResourcePool::notifyRelevantContacts(const XMPP::Jid &jid, bool removed)
{
	/*QList<JabberBaseContact*> list = protocol->contactPool()->findRelevantSources(jid);

	foreach(JabberBaseContact *mContact, list)
	{
		if(removed)
			mContact->setSendsDeliveredEvent(false);

		mContact->reevaluateStatus();
	}*/
}

void JabberResourcePool::addResource(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebugf();
	// see if the resource already exists
	foreach(JabberResource *mResource, pool)
	{
		if((mResource->jid().bare().toLower() == jid.bare().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()))
		{
			kdebug("Updating existing resource %s for %s\n", resource.name().toLocal8Bit().data(), jid.bare().toLocal8Bit().data());

			// It exists, update it. Don't do a "lazy" update by deleting
			// it here and readding it with new parameters later on,
			// any possible lockings to this resource will get lost.
			mResource->setResource(resource);

			// we still need to notify the contact in case the status
			// of this resource changed
			notifyRelevantContacts(jid);

			return;
		}
	}

	kdebug("Adding new resource %s for %s\n", resource.name().toLocal8Bit().data(), jid.bare().toLocal8Bit().data());

	// Update initial capabilities if available.
	// Called before creating JabberResource so JabberResource wouldn't ask for disco information. 
	if( !resource.status().capsNode().isEmpty())
	{
		kdebug("Initial update of capabilities for JID: %s\n", jid.full().toLocal8Bit().data());
///		protocol->protocol()->capabilitiesManager()->updateCapabilities( protocol, jid, resource.status());
	}
	// create new resource instance and add it to the dictionary
	JabberResource *newResource = new JabberResource(protocol, jid, resource);
	connect(newResource, SIGNAL(destroyed (QObject *)), this, SLOT(slotResourceDestroyed (QObject *)));
	connect(newResource, SIGNAL(updated (JabberResource *)), this, SLOT(slotResourceUpdated (JabberResource *)));
	pool.append(newResource);
	// send notifications out to the relevant contacts that
	// a new resource is available for them
	notifyRelevantContacts(jid);
	kdebugf2();
}

void JabberResourcePool::removeResource(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebugf();
	kdebug("Removing resource %s for %s\n", resource.name().toLocal8Bit().data(), jid.bare().toLocal8Bit().data());

	foreach(JabberResource *mResource, pool)
	{
		if((mResource->jid().bare().toLower() == jid.bare().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()))
		{
			JabberResource *deletedResource = pool.takeAt( pool.indexOf(mResource));
			delete deletedResource;

			notifyRelevantContacts(jid, true);
			return;
		}
	}

	kdebug("WARNING: No match found!");
	kdebugf2();
}

void JabberResourcePool::removeAllResources(const XMPP::Jid &jid)
{
	kdebug("Removing all resources for %s\n", jid.bare().toLocal8Bit().data());

	foreach(JabberResource *mResource, pool)
	{
		if(mResource->jid().bare().toLower() == jid.bare().toLower())
		{
			// only remove preselected resource in case there is one
			if(jid.resource().isEmpty() ||(jid.resource().toLower() == mResource->resource().name().toLower()))
			{
				kdebug("Removing resource %s / %s\n", jid.bare().toLocal8Bit().data(), mResource->resource().name().toLocal8Bit().data());
				JabberResource *deletedResource = pool.takeAt(pool.indexOf(mResource));
				delete deletedResource;
			}
		}
	}
}

void JabberResourcePool::clear()
{
	kdebug("Clearing the resource pool.");

	/*
	 * Since many contacts can have multiple resources, we can't simply delete
	 * each resource and trigger a notification upon each deletion. This would
	 * cause lots of status updates in the GUI and create unnecessary flicker
	 * and API traffic. Instead, collect all JIDs, clear the dictionary
	 * and then notify all JIDs after the resources have been deleted.
	 */

	QStringList jidList;

	foreach(JabberResource *mResource, pool)
	{
		jidList += mResource->jid().full();
	}

	/*
	 * The lock list will be cleaned automatically.
	 */
	qDeleteAll(pool);
	pool.clear();

	/*
	 * Now go through the list of JIDs and notify each contact
	 * of its status change
	 */
	for(QStringList::Iterator it = jidList.begin(); it != jidList.end(); ++it)
	{
		notifyRelevantContacts(XMPP::Jid(*it), true);
	}

}

void JabberResourcePool::lockToResource(const XMPP::Jid &jid, const XMPP::Resource &resource)
{
	kdebug("Locking %s to %s\n", jid.full().toLocal8Bit().data(), resource.name().toLocal8Bit().data());
	// remove all existing locks first
	removeLock(jid);

	// find the resource in our dictionary that matches
	foreach(JabberResource *mResource, pool)
	{
		if((mResource->jid().bare().toLower() == jid.full().toLower()) && (mResource->resource().name().toLower() == resource.name().toLower()))
		{
			lockList.append(mResource);
			return;
		}
	}

	kdebug("WARNING: No match found!");
}

void JabberResourcePool::removeLock(const XMPP::Jid &jid)
{
	kdebug("Removing resource lock for %s\n", jid.bare().toLocal8Bit().data());

	// find the resource in our dictionary that matches
	foreach(JabberResource *mResource, pool)
	{
		if((mResource->jid().bare().toLower() == jid.bare().toLower()))
		{
			lockList.removeAll (mResource);
		}
	}

	kdebug("No locks found.");
}

JabberResource *JabberResourcePool::lockedJabberResource( const XMPP::Jid &jid)
{
	// check if the JID already carries a resource, then we will have to use that one
	if(!jid.resource().isEmpty())
	{
		// we are subscribed to a JID, find the according resource in the pool
		foreach(JabberResource *mResource, pool)
		{
			if(( mResource->jid().bare().toLower() == jid.bare().toLower()) &&(mResource->resource().name() == jid.resource()))
			{
				return mResource;
			}
		}

		kdebug("WARNING: No resource found in pool, returning as offline.");

		return 0L;
	}

	// see if we have a locked resource
	foreach(JabberResource *mResource, lockList)
	{
		if(mResource->jid().bare().toLower() == jid.bare().toLower())
		{
			kdebug("Current lock for %s is %s\n", jid.bare().toLocal8Bit().data(), mResource->resource().name().toLocal8Bit().data());
			return mResource;
		}
	}

	kdebug("No lock available for %s\n", jid.bare().toLocal8Bit().data());

	// there's no locked resource, return an empty resource
	return 0L;
}

const XMPP::Resource &JabberResourcePool::lockedResource(const XMPP::Jid &jid)
{
	JabberResource *resource = lockedJabberResource( jid);
	return (resource) ? resource->resource() : EmptyResource;
}

JabberResource *JabberResourcePool::bestJabberResource( const XMPP::Jid &jid, bool honourLock)
{
	kdebug("Determining best resource for %s\n", jid.full().toLocal8Bit().data());

	if(honourLock)
	{
		// if we are locked to a certain resource, always return that one
		JabberResource *mResource = lockedJabberResource(jid);
		if(mResource)
		{
			kdebug("We have a locked resource %s for %s\n", mResource->resource().name().toLocal8Bit().data(), jid.full().toLocal8Bit().data());
			return mResource;
		}
	}

	JabberResource *bestResource = 0L;
	JabberResource *currentResource = 0L;

	foreach(currentResource, pool)
	{
		// make sure we are only looking up resources for the specified JID
		if(currentResource->jid().bare().toLower() != jid.bare().toLower())
		{
			continue;
		}

		// take first resource if no resource has been chosen yet
		if(!bestResource)
		{
			kdebug("Taking %s as first available resource.\n", currentResource->resource().name().toLocal8Bit().data());

			bestResource = currentResource;
			continue;
		}

		if(currentResource->resource().priority() > bestResource->resource().priority())
		{
			kdebug("Using %s due to better priority.\n", currentResource->resource().name().toLocal8Bit().data());
			// got a better match by priority
			bestResource = currentResource;
		}
		else
		{
			if(currentResource->resource().priority() == bestResource->resource().priority())
			{
				if(currentResource->resource().status().timeStamp() > bestResource->resource().status().timeStamp())
				{
					kdebug("Using %s due to better timestamp.\n", currentResource->resource().name().toLocal8Bit().data());

					// got a better match by timestamp (priorities are equal)
					bestResource = currentResource;
				}
			}
		}
	}

	return (bestResource) ? bestResource : 0L;
}

const XMPP::Resource &JabberResourcePool::bestResource(const XMPP::Jid &jid, bool honourLock)
{
	JabberResource *bestResource = bestJabberResource( jid, honourLock);
	return (bestResource) ? bestResource->resource() : EmptyResource;
}

//TODO: Find Resources based on certain Features.
void JabberResourcePool::findResources(const XMPP::Jid &jid, JabberResourcePool::ResourceList &resourceList)
{
	foreach(JabberResource *mResource, pool)
	{
		if(mResource->jid().bare().toLower() == jid.bare().toLower())
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if(!jid.resource().isEmpty() &&(jid.resource().toLower() != mResource->resource().name().toLower()))
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append(mResource);
		}
	}
}

void JabberResourcePool::findResources(const XMPP::Jid &jid, XMPP::ResourceList &resourceList)
{
	foreach(JabberResource *mResource, pool)
	{
		if(mResource->jid().bare().toLower() == jid.bare().toLower())
		{
			// we found a resource for the JID, let's see if the JID already contains a resource
			if(!jid.resource().isEmpty() &&(jid.resource().toLower() != mResource->resource().name().toLower()))
				// the JID contains a resource but it's not the one we have in the dictionary,
				// thus we have to ignore this resource
				continue;

			resourceList.append(mResource->resource());
		}
	}
}
