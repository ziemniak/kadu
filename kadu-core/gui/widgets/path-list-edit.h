/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PATH_LIST_EDIT_H
#define PATH_LIST_EDIT_H

#include <QtGui/QPushButton>

class QLineEdit;
class QListWidget;

class PathListEditWindow;

class PathListEdit : public QPushButton
{
	Q_OBJECT

	PathListEditWindow *Dialog;
	QStringList PathList;

private slots:
	void showDialog();
	void dialogDestroyed();
	void pathListChanged(const QStringList &pathList);

public:
	PathListEdit(QWidget *parent = 0);
	virtual ~PathListEdit() {}

	QStringList pathList() const { return PathList; }
	void setPathList(const QStringList &pathList);

signals:
	void changed();

};

class PathListEditWindow : public QWidget
{
	Q_OBJECT

	QListWidget *PathListWidget;

	bool validatePath(QString &path);

private slots:
	void addPathClicked();
	void changePathClicked();
	void deletePathClicked();

	void okClicked();

	void closeEvent(QCloseEvent *e);
	void keyPressEvent(QKeyEvent *e);

public:
	PathListEditWindow(const QStringList &pathList, QWidget *parent = 0);
	~PathListEditWindow();

public slots:
	void setPathList(const QStringList &list);

signals:
	void changed(const QStringList &paths);

};

#endif // PATH_LIST_EDIT_H
