#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <qtoolbar.h>
#include "dockarea.h"
#include "toolbutton.h"
#include "usergroup.h"

class ToolButton;

class ToolBar : public QToolBar
{
	Q_OBJECT

	private:
		ToolButton *dragButton;

	private slots:
		void addButtonClicked(int action_index);
		void deleteToolbar(void);

	protected:
		virtual void dragEnterEvent(QDragEnterEvent* event);
		virtual void dropEvent(QDropEvent* event);
		virtual void contextMenuEvent(QContextMenuEvent* e);
		virtual void dragLeaveEvent(QDragLeaveEvent *e);

	public:
		ToolBar(QWidget* parent, const char *name);
		~ToolBar();
		DockArea* dockArea();
		void loadFromConfig(QDomElement parent_element);
		/**
			Returns list of users that will be affected by activated action.
			It depends on where the toolbar is located. If toolbar is in chat
			window, selected users are the users in chat. If toolbar is the
			main toolbar, selected users are the selected ones in contact
			list, and so on...
			Returns NULL if toolbar is no connected to user list.
		**/
		const UserGroup* selectedUsers() const;
		QPopupMenu* createContextMenu(QWidget* parent);

	public slots:
		void writeToConfig(QDomElement parent_element);
};

#endif