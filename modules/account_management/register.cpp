/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qvbox.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>

#include "register.h"
#include "debug.h"
#include "config_file.h"
#include "misc.h"

#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

void Register::createConfig()
{
	kdebugf();
	char *home = getenv("HOME");
	struct passwd *pw;

	kdebugmf(KDEBUG_INFO, "$HOME=%s\n", home);
	if (!home) {
		if (!(pw = getpwuid(getuid())))
			return;
		home = pw->pw_dir;
	}

	struct stat buf;
	QString ggpath = ggPath("");
	stat(ggpath.local8Bit(), &buf);
	if (S_ISDIR(buf.st_mode))
		kdebugmf(KDEBUG_INFO, "Directory %s exists\n", (const char *)ggpath.local8Bit());
	else
	{
		kdebugmf(KDEBUG_INFO, "Creating directory\n");
		if (mkdir(ggpath.local8Bit(), 0700) != 0 )
		{
			perror("mkdir");
			return;
		}
	}

	kdebugmf(KDEBUG_INFO, "Writing config files...\n");
	config_file.sync();

	qApp->mainWidget()->setCaption(QString("Kadu: %1").arg((UinType)config_file.readNumEntry("General","UIN")));

	kdebugf2();
}

Register::Register(QDialog * /*parent*/, const char * /*name*/)
{
	kdebugf();
	setWFlags(Qt::WDestructiveClose);
	setCaption(tr("Register user"));

	// create main QLabel widgets (icon and app info)
	QVBox *left=new QVBox(this);
	left->setMargin(10);
	left->setSpacing(10);

	QLabel *l_icon = new QLabel(left);
	QWidget *blank=new QWidget(left);
	blank->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding));

	QVBox *center=new QVBox(this);
	center->setMargin(10);
	center->setSpacing(10);

	QLabel *l_info = new QLabel(center);
	l_icon->setPixmap(icons_manager.loadIcon("RegisterWindowIcon"));
	l_info->setText(tr("This dialog box allows you to register a new account."));
	l_info->setAlignment(Qt::WordBreak);
	// end create main QLabel widgets (icon and app info)

	//our QVGroupBox
	QVGroupBox *vgb_email = new QVGroupBox(center);
	vgb_email->setTitle(tr("Email"));
	QVGroupBox *vgb_password = new QVGroupBox(center);
	vgb_password->setTitle(tr("Password"));
	center->setStretchFactor(vgb_password, 1);
	//end our QGroupBox

	// create needed fields

	new QLabel(tr("New email:"), vgb_email);
	mailedit = new QLineEdit(vgb_email);

	new QLabel(tr("New password:"), vgb_password);
	pwd = new QLineEdit(vgb_password);
	pwd->setEchoMode(QLineEdit::Password);

	new QLabel(tr("Retype new password:"), vgb_password);
	pwd2 = new QLineEdit(vgb_password);
	pwd2->setEchoMode(QLineEdit::Password);
	// end create needed fields

	cb_updateconfig = new QCheckBox(center);
	cb_updateconfig->setChecked(center);
	cb_updateconfig->setText(tr("Create config file"));
	QToolTip::add(cb_updateconfig, tr("Write the newly obtained UIN and password into a clean configuration file\nThis will erase your current config file contents if you have one"));

	// buttons
	QHBox *bottom = new QHBox(center);
	QWidget *blank2 = new QWidget(bottom);
	bottom->setSpacing(5);
	blank2->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
	QPushButton *pb_register = new QPushButton(icons_manager.loadIcon("RegisterAccountButton"), tr("Register"), bottom, "register");
	QPushButton *pb_close = new QPushButton(icons_manager.loadIcon("CloseWindow"), tr("&Close"), bottom, "close");
	// end buttons

	connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
	connect(pb_register, SIGNAL(clicked()), this, SLOT(doRegister()));
	connect(gadu, SIGNAL(registered(bool, UinType)), this, SLOT(registered(bool, UinType)));

 	loadGeometry(this, "General", "RegisterDialogGeometry", 0, 0, 400, 400);
	kdebugf2();
}

Register::~Register()
{
	kdebugf();
	saveGeometry(this, "General", "RegisterDialogGeometry");
	kdebugf2();
}

void Register::keyPressEvent(QKeyEvent *ke_event)
{
	if (ke_event->key() == Qt::Key_Escape)
		close();
}

void Register::doRegister() {
	kdebugf();

	if (pwd->text() != pwd2->text())
	{
		QMessageBox::information(0, tr("Register user"),
				tr("Error data typed in required fields.\n\nPasswords typed in "
				"both fields (\"New password\" and \"Retype new password\") "
				"should be the same!"), tr("OK"), 0, 0, 1);
		return;
	}

	if (pwd->text().isEmpty())
	{
		QMessageBox::warning(this, "Kadu", tr("Please fill out all fields"), tr("OK"), 0, 0, 1);
		return;
	}

	QString Password, Email;

	Password = pwd->text();
	Email = mailedit->text();

	setEnabled(false);
	gadu->registerAccount(Email, Password);
	kdebugf2();
}

void Register::registered(bool ok, UinType uin)
{
	kdebugf();
	if (ok)
	{
		this->uin = uin;
		QMessageBox::information(this, "Kadu", tr("Registration was successful. Your new number is %1.\nStore it in a safe place along with the password.\nNow add your friends to the userlist.").arg(uin), tr("OK"), 0, 0, 1);
		ask();
		close();
	}
	else
	{
		QMessageBox::warning(0, tr("Register user"),
				tr("An error has occured while registration. Please try again later."), tr("OK"), 0, 0, 1);
		setEnabled(true);
	}
	kdebugf2();
}


void Register::ask() {
	kdebugf();
	if (cb_updateconfig->isChecked()) {
		config_file.writeEntry("General","UIN",(int)uin);
		config_file.writeEntry("General","Password",pwHash(pwd->text()));
		createConfig();
		}
	kdebugf2();
}
