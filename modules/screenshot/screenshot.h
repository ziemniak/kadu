/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QtGui/QWidget>

#include <X11/extensions/shape.h>

class QLabel;
class QMenu;

class ActionDescription;
class ChatWidget;
class ConfigurationUiHandler;

enum ScreenShotMode
{
	Standard = 0,
	WithChatWindowHidden = 1,
	SingleWindow = 2
};

class ShotSizeHint : public QWidget
{
	friend class ScreenShot;

	QLabel *geom;
	QLabel *fileSize;

public:
	ShotSizeHint();

};

class ScreenShot : public QWidget
{
	Q_OBJECT

	ConfigurationUiHandler *UiHandler;
	ActionDescription *screenShotAction;
	ChatWidget *chatWidget;

	ScreenShotMode shotMode;
	bool ShowPaintRect;

	bool buttonPressed;
	QRect region;
	QPixmap pixmap;
	ShotSizeHint *sizeHint;
	QTimer *hintTimer;
	QMenu *menu;
	bool wasMaximized;
	bool warnedAboutSize;
	int minSize;

	void checkShotsSize();
	void handleShot(QPixmap p);

	void pasteImageClause(const QString &path);
	void checkConferenceImageSizes(int size);
	bool checkSingleUserImageSize(int size);

	// TABS module fixing methods
	void minimize(QWidget *w);
	void restore(QWidget *w);
	bool isMaximized(QWidget *w);

	// Methods from KSnapShot
	QPixmap grabCurrent();
	Window findRealWindow(Window w, int depth = 0);
	void getWindowsRecursive(std::vector<QRect> &windows, Window w, int rx = 0, int ry = 0, int depth = 0);
	QPixmap grabWindow(Window child, int x, int y, uint w, uint h, uint border);
	Window windowUnderCursor(bool includeDecorations);

	void createDefaultConfiguration();

protected:
	virtual void paintEvent(QPaintEvent *e);

private slots:
	void takeShot();
	void takeShotWithChatWindowHidden();
	void takeWindowShot();


	void takeShot_Step2();
	void takeWindowShot_Step2();

	void grabMouseSlot();
	void updateHint();

	void screenshotActionActivated(QAction *sender, bool toggled);

public:
	ScreenShot(bool firstLoad);
	virtual ~ScreenShot();

	ConfigurationUiHandler * configurationUiHandler() { return UiHandler; }

	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void keyPressEvent(QKeyEvent *e);

};

extern ScreenShot* screenShot;

#endif
