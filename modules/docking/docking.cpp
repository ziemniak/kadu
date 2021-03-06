/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QMouseEvent>
#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#endif

#ifdef Q_WS_X11
#include "x11tools.h"
// TODO: hack :/
#undef Status
#endif
#include "activate.h"
#include "accounts/account-manager.h"
#include "chat/message/pending-messages-manager.h"
#include "configuration/configuration-file.h"
#include "core/core.h"
#include "debug.h"
#include "gui/windows/kadu-window.h"
#include "gui/widgets/status-menu.h"
#include "gui/windows/main-configuration-window.h"
#include "icons-manager.h"
#include "misc/misc.h"
#include "protocols/protocol.h"
#include "status/status-container-manager.h"
#include "status_changer.h"

#include "docking.h"

/**
 * @ingroup docking
 * @{
 */
extern "C" KADU_EXPORT int docking_init(bool firstLoad)
{
	docking_manager = new DockingManager();
	MainConfigurationWindow::registerUiFile(dataPath("kadu/modules/configuration/docking.ui"));

	return 0;
}

extern "C" KADU_EXPORT void docking_close()
{
	kdebugf();

	MainConfigurationWindow::unregisterUiFile(dataPath("kadu/modules/configuration/docking.ui"));
	delete docking_manager;
	docking_manager = 0;
}

DockingManager::DockingManager()
	: newMessageIcon(StaticEnvelope), icon_timer(new QTimer()), blink(false)
{
	kdebugf();

	createDefaultConfiguration();

	connect(icon_timer, SIGNAL(timeout()), this, SLOT(changeIcon()));

	connect(Core::instance(), SIGNAL(mainIconChanged(const QIcon &)),
		this, SLOT(statusPixmapChanged(const QIcon &)));
	connect(PendingMessagesManager::instance(), SIGNAL(messageFromUserAdded(Contact)), this, SLOT(pendingMessageAdded()));
	connect(PendingMessagesManager::instance(), SIGNAL(messageFromUserDeleted(Contact)), this, SLOT(pendingMessageDeleted()));

	connect(Core::instance(), SIGNAL(searchingForTrayPosition(QPoint&)), this, SIGNAL(searchingForTrayPosition(QPoint&)));

	DockMenu = new QMenu;

#ifdef Q_OS_MAC
	OpenChatAction = new QAction(IconsManager::instance()->loadIcon("OpenChat"), tr("Show Pending Messages"));
	connect(OpenChatAction, SIGNAL(triggered()), chat_manager, SLOT(openPendingMsgs()));
#endif
	CloseKaduAction = new QAction(IconsManager::instance()->loadIcon("Exit"), tr("&Exit Kadu"), this);
	connect(CloseKaduAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	connect(this, SIGNAL(mousePressMidButton()), PendingMessagesManager::instance(), SLOT(openMessages()));

	configurationUpdated();

	updateContextMenu();

	kdebugf2();
}

DockingManager::~DockingManager()
{
	kdebugf();

	disconnect(Core::instance(), SIGNAL(mainIconChanged(const QIcon &)),
		this, SLOT(statusPixmapChanged(const QIcon &)));
	disconnect(PendingMessagesManager::instance(), SIGNAL(messageFromUserAdded(Contact)), this, SLOT(pendingMessageAdded()));
	disconnect(PendingMessagesManager::instance(), SIGNAL(messageFromUserDeleted(Contact)), this, SLOT(pendingMessageDeleted()));

//	disconnect(kadu, SIGNAL(searchingForTrayPosition(QPoint&)), this, SIGNAL(searchingForTrayPosition(QPoint&)));

	disconnect(icon_timer, SIGNAL(timeout()), this, SLOT(changeIcon()));

	delete DockMenu;

	delete icon_timer;
	icon_timer = NULL;
	kdebugf2();
}

void DockingManager::changeIcon()
{
	kdebugf();
	if (PendingMessagesManager::instance()->pendingMsgs() && !icon_timer->isActive())
	{
		switch (newMessageIcon)
		{
			case AnimatedEnvelope:
				emit trayMovieChanged(IconsManager::instance()->iconPath("MessageAnim"));
				break;
			case StaticEnvelope:
				emit trayPixmapChanged(IconsManager::instance()->loadIcon("Message"));
				break;
			case BlinkingEnvelope:
				if (!blink)
				{
					emit trayPixmapChanged(IconsManager::instance()->loadIcon("Message"));
					icon_timer->setSingleShot(true);
					icon_timer->start(500);
					blink = true;
				}
				else
				{
					Account *account = AccountManager::instance()->defaultAccount();
					if (!account || !account->protocol())
						return;

					const Status &stat = account->protocol()->status();
					emit trayPixmapChanged(QIcon(account->protocol()->statusPixmap(stat)));
					icon_timer->setSingleShot(true);
					icon_timer->start(500);
					blink = false;
				}
				break;
		}
	}
	else
		kdebugmf(KDEBUG_INFO, "OFF\n");
	kdebugf2();
}

void DockingManager::pendingMessageAdded()
{
	changeIcon();
}

void DockingManager::pendingMessageDeleted()
{
	if (!PendingMessagesManager::instance()->pendingMsgs())
	{
		Account *account = AccountManager::instance()->defaultAccount();
		if (!account || !account->protocol())
			return;

		const Status &stat = account->protocol()->status();
		emit trayPixmapChanged(QIcon(account->protocol()->statusPixmap(stat)));
	}
}

void DockingManager::defaultToolTip()
{
	if (config_file.readBoolEntry("General", "ShowTooltipInTray"))
	{
		Status status = AccountManager::instance()->status();

		QString tiptext;
		tiptext.append(tr("Current status:\n%1")
			.arg(qApp->translate("@default", Status::name(status).toLocal8Bit().data())));

		if (!status.description().isEmpty())
			tiptext.append(tr("\n\nDescription:\n%2").arg(status.description()));

		emit trayTooltipChanged(tiptext);
	}
}

void DockingManager::trayMousePressEvent(QMouseEvent * e)
{
	kdebugf();
	if (e->button() == Qt::MidButton)
	{
		emit mousePressMidButton();
		return;
	}

	if (e->button() == Qt::LeftButton)
	{
		QWidget *kadu = Core::instance()->kaduWindow();

		emit mousePressLeftButton();
		kdebugm(KDEBUG_INFO, "minimized: %d, visible: %d\n", kadu->isMinimized(), kadu->isVisible());

		if (PendingMessagesManager::instance()->pendingMsgs() && (e->modifiers() != Qt::ControlModifier))
		{
			PendingMessagesManager::instance()->openMessages();
			return;
		}

		if (kadu->isMinimized())
		{
			kadu->showNormal();
			_activateWindow(kadu);
			return;
		}
		#ifdef Q_WS_X11
		else if (
			( X11_getDesktopOfWindow(QX11Info::display(), kadu->winId()) == X11_getCurrentDesktop( QX11Info::display() ) ) &&
			( X11_isWindowFullyVisible(QX11Info::display(), kadu->winId()) ) )
			kadu->hide();
		#else
		else if (kadu->isVisible())
			kadu->hide();
		#endif
		else
		{
			kadu->show();
			_activateWindow(kadu);
		}
		return;
	}

	if (e->button() == Qt::RightButton)
	{
		emit mousePressRightButton();
		//showPopupMenu(dockMenu);
		return;
	}
	kdebugf2();
}

void DockingManager::statusPixmapChanged(const QIcon &icon)
{
 	kdebugf();
	emit trayPixmapChanged(icon);
	defaultToolTip();
	changeIcon();
}

QIcon DockingManager::defaultPixmap()
{
	Account *account = AccountManager::instance()->defaultAccount();
	if (!account || !account->protocol())
	{
		return QIcon();
	}
	return QIcon(account->protocol()->statusPixmap(account->protocol()->status()));
}

void DockingManager::setDocked(bool docked)
{
	kdebugf();
	if (docked)
	{
		changeIcon();
		defaultToolTip();
		if (config_file.readBoolEntry("General", "RunDocked"))
			Core::instance()->setShowMainWindowOnStart(false);
	}
	else
	{
// 		kdebugm(KDEBUG_INFO, "closing: %d\n", Kadu::closing());
		Core::instance()->kaduWindow()->show(); // isClosing? TODO: 0.6.6
	}
	Core::instance()->kaduWindow()->setDocked(docked);
	kdebugf2();
}

void DockingManager::updateContextMenu()
{
	DockMenu->clear();
	qDeleteAll(StatusContainerMenus.values());
	StatusContainerMenus.clear();

    #ifdef Q_OS_MAC
	DockMenu->addAction(OpenChatAction);
	DockMenu->insertSeparator();
    #endif

	int statusContainersCount = StatusContainerManager::instance()->statusContainers().count();
	StatusMenu *statusMenu;

	if (statusContainersCount == 1)
	{
		statusMenu = new StatusMenu(StatusContainerManager::instance()->statusContainers()[0], DockMenu);
		statusMenu->addToMenu(DockMenu);
	}
	else
	{
		foreach (StatusContainer *container, StatusContainerManager::instance()->statusContainers())
		{
			statusMenu = new StatusMenu(container, DockMenu);
			QMenu *menu = new QMenu(container->statusContainerName());
			menu->setIcon(container->statusPixmap());
			statusMenu->addToMenu(menu);
			StatusContainerMenus[container] = DockMenu->addMenu(menu);
			connect(container, SIGNAL(statusChanged()), this, SLOT(containerStatusChanged()));

		}
		if (statusContainersCount > 1)
			containersSeparator = DockMenu->addSeparator();

		statusMenu = new StatusMenu(StatusContainerManager::instance(), DockMenu);
		statusMenu->addToMenu(DockMenu);
	}

	DockMenu->addAction(CloseKaduAction);
}

void DockingManager::containerStatusChanged()
{
	StatusContainer *container;
	if (sender() && (container = dynamic_cast<StatusContainer *>(sender())) && StatusContainerMenus[container])
		StatusContainerMenus[container]->setIcon(container->statusPixmap());
}

void DockingManager::statusContainerRegistered(StatusContainer *statusContainer)
{
	if (StatusContainerManager::instance()->statusContainers().count() < 3)
		updateContextMenu();
	else
	{
		StatusMenu *statusMenu = new StatusMenu(statusContainer, DockMenu);
		QMenu *menu = new QMenu(statusContainer->statusContainerName());
		menu->setIcon(statusContainer->statusPixmap());
		statusMenu->addToMenu(menu);
		StatusContainerMenus[statusContainer] = DockMenu->insertMenu(containersSeparator, menu);
		connect(statusContainer, SIGNAL(statusChanged()), this, SLOT(containerStatusChanged()));
	}
}

void DockingManager::statusContainerUnregistered(StatusContainer *statusContainer)
{
	if (StatusContainerManager::instance()->statusContainers().count() < 2)
		updateContextMenu();
	else
	{
		QAction *menuAction = StatusContainerMenus[statusContainer];
		if (!menuAction)
			    return;

		menuAction->menu()->clear();
		StatusContainerMenus.remove(statusContainer);
		DockMenu->removeAction(menuAction);
		delete menuAction;
	}
}

void DockingManager::configurationUpdated()
{
	if (config_file.readBoolEntry("General", "ShowTooltipInTray"))
		defaultToolTip();
	else
		emit trayTooltipChanged(QString::null);

	IconType it = (IconType)config_file.readNumEntry("Look", "NewMessageIcon");
	if (newMessageIcon != it)
	{
		newMessageIcon = it;
		changeIcon();
	}
}

void DockingManager::createDefaultConfiguration()
{
	config_file.addVariable("General", "RunDocked", false);
	config_file.addVariable("General", "ShowTooltipInTray", true);
	config_file.addVariable("Look", "NewMessageIcon", 0);
}

DockingManager *docking_manager = NULL;

/** @} */

