#ifndef SEARCH_H
#define SEARCH_H

#include <qradiobutton.h>
#include <qdialog.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include "libgadu.h"

/**
	Dialog umożliwiający wyszukiwanie w katalogu publicznym
**/
class SearchDialog : public QDialog {
	Q_OBJECT
	public:
		SearchDialog(QWidget *parent=0, const char *name=0, uin_t whoisSearchUin = 0);
		~SearchDialog(void);

	private:
		QCheckBox *only_active;
		QLineEdit *e_name;
		QLineEdit *e_nick;
		QLineEdit *e_byr;
		QLineEdit *e_surname;
		QComboBox *c_gender;
		QLineEdit *e_city;
		QListView *results;
		QLabel *progress;
		QRadioButton *r_uin;
		QRadioButton *r_pers;
		QPushButton *b_chat;
		QPushButton *b_sendbtn;
		QPushButton *b_nextbtn;
		QPushButton *b_addbtn;
		uin_t _whoisSearchUin;
		uint32_t seq;

		QLineEdit *e_uin;

		uin_t fromUin;
		bool searchhidden;

	private slots:
		void clearResults(void);
		void prepareMessage(QListViewItem*);
		void uinTyped(void);
		void personalDataTyped(void);
		void AddButtonClicked();
		void updateInfoClicked();
		void openChat();

	public slots:
		void firstSearch(void);
		void nextSearch(void);
		void showResults(gg_pubdir50_t res);
		void selectionChanged(QListViewItem *);

	protected:
		void closeEvent(QCloseEvent * e);

};

#endif
