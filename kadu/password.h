#ifndef PASSWORD_H
#define PASSWORD_H

#include <qobject.h>
#include <qsocketnotifier.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdialog.h>

#include "misc.h"

class remindPassword : public QObject {
	Q_OBJECT
	public:
		remindPassword();
		~remindPassword();

	private:
		void deleteSocketNotifiers();
		void createSocketNotifiers();
		void showErrorMessageBox();

		struct gg_http *h;
		struct gg_pubdir *p;

		QSocketNotifier *snr;
		QSocketNotifier *snw;

	public slots:
		void start();

	private slots:
		void socketEvent();
		void dataReceived();
		void dataSent();
};

class changePassword : public QDialog {
	Q_OBJECT
	public:
		changePassword(QDialog *parent = 0, const char *name = 0);

	private:
		void doGetToken();
		void deleteSocketNotifiers();
		void createSocketNotifiers();
		void showMessage(QString &msg);

		ImageWidget *tokenimage;
		QLineEdit *emailedit, *newpwd, *newpwd2, *tokenedit;
		QLabel *status;
		QString token_id;
		token token_handle;

		struct gg_http *h;
		struct gg_pubdir *p;

		QSocketNotifier *snr;
		QSocketNotifier *snw;

	private slots:
		void start();
		void socketEvent();
		void dataReceived();
		void dataSent();
		void gotTokenReceived(struct gg_http *h);
		void tokenErrorReceived();

	protected:
		void closeEvent(QCloseEvent *e);
};

#endif
