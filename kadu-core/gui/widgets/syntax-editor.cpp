/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>

#include "gui/windows/message-box.h"
#include "gui/windows/syntax-editor-window.h"
#include "misc/misc.h"
#include "misc/syntax-list.h"

#include "syntax-editor.h"

SyntaxEditor::SyntaxEditor(QWidget *parent) :
		QWidget(parent), syntaxList(0)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	syntaxListCombo = new QComboBox(this);
	connect(syntaxListCombo, SIGNAL(activated(const QString &)), this, SLOT(syntaxChangedSlot(const QString &)));

	QPushButton *editButton = new QPushButton(tr("Edit"), this);
	deleteButton = new QPushButton(tr("Delete"), this);
	connect(editButton, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));

	layout->addWidget(syntaxListCombo, 100);
	layout->addWidget(editButton);
	layout->addWidget(deleteButton);
}

SyntaxEditor::~SyntaxEditor()
{
	if (syntaxList)
	{
		delete syntaxList;
		syntaxList = 0;
	}
}

void SyntaxEditor::setCurrentSyntax(const QString &syntax)
{
	syntaxListCombo->setCurrentIndex(syntaxListCombo->findText(syntax));
	syntaxChangedSlot(syntax);
}

QString SyntaxEditor::currentSyntax()
{
	return syntaxListCombo->currentText();
}

void SyntaxEditor::setCategory(const QString &category)
{
	this->category = category;
	updateSyntaxList();
}

void SyntaxEditor::setSyntaxHint(const QString &syntaxHint)
{
	this->syntaxHint = syntaxHint;
}

void SyntaxEditor::editClicked()
{
	SyntaxEditorWindow *editor = new SyntaxEditorWindow(syntaxList, syntaxListCombo->currentText(), category, syntaxHint);
	connect(editor, SIGNAL(updated(const QString &)), this, SLOT(setCurrentSyntax(const QString &)));

	emit onSyntaxEditorWindowCreated(editor);
	editor->refreshPreview();
	editor->show();
}

void SyntaxEditor::deleteClicked()
{
	if (!syntaxList)
		return;

	if (syntaxList->deleteSyntax(currentSyntax()))
		setCurrentSyntax(*(syntaxList->keys().begin()));
	else
		MessageBox::msg(tr("Unable to remove syntax: %1").arg(currentSyntax()), true, "Warning");
}

void SyntaxEditor::syntaxChangedSlot(const QString &newSyntax)
{
	if (!syntaxList)
		return;

	if (!syntaxList->contains(newSyntax))
		return;

	QFile file;
	QString fileName;
	QString content;

	SyntaxInfo info = (*syntaxList)[newSyntax];
	if (info.global)
		fileName = dataPath("kadu") + "/syntax/" + category.toLower() + "/" + newSyntax + ".syntax";
	else
		fileName = ggPath() + "/syntax/" + category.toLower() + "/" + newSyntax + ".syntax";

	file.setFileName(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return;

	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	content = stream.readAll();
	file.close();

	content.replace(QRegExp("%o"),  " ");

	deleteButton->setEnabled(!info.global);
	emit syntaxChanged(content);
}

void SyntaxEditor::updateSyntaxList()
{
	if (syntaxList)
		delete syntaxList;
	syntaxList = new SyntaxList(category.toLower());

	syntaxListCombo->clear();
	syntaxListCombo->addItems(syntaxList->keys());

	connect(syntaxList, SIGNAL(updated()), this, SLOT(syntaxListUpdated()));
}

void SyntaxEditor::syntaxListUpdated()
{
	syntaxListCombo->clear();
	syntaxListCombo->addItems(syntaxList->keys());
}
