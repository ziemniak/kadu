#ifndef REGISTER_H
#define REGISTER_H

#include <QtGui/QWidget>

#include "../modules/gadu_protocol/gadu.h"

class QCheckBox;
class QLineEdit;

/**
	Dialog umożliwiający rejestrację nowego użytkownika
	@ingroup account_management
	@{
**/
class Register : public QWidget 
{
	Q_OBJECT

		QLineEdit *pwd;
		QLineEdit *pwd2;
		QLineEdit *mailedit;
		UinType uin;
		QCheckBox *cb_updateconfig;

		void ask();
		void createConfig();

	private slots:
		void doRegister();
		void keyPressEvent(QKeyEvent *);

	public:
		Register(QDialog* parent = 0);
		~Register();

	public slots:
		void registered(bool ok, UinType uin);

};

/** @} */

#endif
