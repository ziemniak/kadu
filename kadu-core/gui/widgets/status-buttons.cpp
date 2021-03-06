/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QHBoxLayout>

#include "accounts/account-manager.h"
#include "gui/widgets/status-button.h"
#include "status/status-container-manager.h"

#include "status-buttons.h"

StatusButtons::StatusButtons(QWidget *parent) :
		QWidget(parent), Layout(0)
{
	createGui();

	triggerAllStatusContainerRegistered();

	if (0 == StatusContainerManager::instance()->count())
		statusContainerRegistered(StatusContainerManager::instance());
}

StatusButtons::~StatusButtons()
{
}

void StatusButtons::createGui()
{
	Layout = new QHBoxLayout(this);
}

void StatusButtons::statusContainerRegistered(StatusContainer *statusContainer)
{
	if (Buttons.contains(statusContainer) || !Layout)
		return;

	if (1 == StatusContainerManager::instance()->count())
		statusContainerUnregistered(StatusContainerManager::instance());

	StatusButton *button = new StatusButton(statusContainer);
	Layout->addWidget(button);
	Buttons[statusContainer] = button;
}

void StatusButtons::statusContainerUnregistered(StatusContainer *statusContainer)
{
	if (Buttons.contains(statusContainer))
	{
		delete Buttons[statusContainer];
		Buttons.remove(statusContainer);
	}
	if (0 == StatusContainerManager::instance()->count())
		statusContainerRegistered(StatusContainerManager::instance());

}
