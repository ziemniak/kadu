 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Copyright (C) 2003  Justin Karneges
 */

#ifndef CERTIFICATEDISPLAYDIALOG_H
#define CERTIFICATEDISPLAYDIALOG_H

#include <QtCrypto>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>

class CertificateDisplayDialog : public QDialog
{
		Q_OBJECT

    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QLabel *textLabel4;
    QLabel *lb_valid;
    QLabel *textLabel2;
    QLabel *lb_notBefore;
    QLabel *textLabel3;
    QLabel *lb_notAfter;
    QLabel *textLabel1;
    QLabel *lb_sn;
    QSpacerItem *spacerItem;
    QTextBrowser *tb_cert;
    QFrame *line1;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem1;
    QPushButton *pb_close;

	public:
		CertificateDisplayDialog(const QCA::Certificate &, int result, QCA::Validity, QWidget *parent=0);

	protected:
		static void setLabelStatus(QLabel& l, bool ok);
		static QString makePropEntry(QCA::CertificateInfoType var, const QString &name, const QCA::CertificateInfo &list);
		QString makePropTable(const QString &heading, const QCA::CertificateInfo &props);
};

#endif
