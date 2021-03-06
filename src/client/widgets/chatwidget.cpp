/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2007-2014 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "chatlineedit.h"
#include "chatwidget.h"
#include "utils/html.h"

#include <QDebug>
#include <QResizeEvent>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTextDocument>
#include <QUrl>


namespace widgets {

ChatBox::ChatBox(QWidget *parent)
	:  QWidget(parent), _wasCollapsed(false)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	layout->setSpacing(0);
	layout->setMargin(0);

	_view = new QTextBrowser(this);
	_view->setOpenExternalLinks(true);

	layout->addWidget(_view, 1);

	_myline = new ChatLineEdit(this);
	_myline->setPlaceholderText(tr("Chat..."));
	layout->addWidget(_myline);

	setLayout(layout);

	connect(_myline, SIGNAL(returnPressed(QString)), this, SLOT(sendMessage(QString)));

	// Chat window styling
	setStyleSheet(
		"QTextEdit, QLineEdit {"
			"border: none;"
			"background-color: #111;"
			"color: #adadad;"
			"font-family: Monospace"
		"}"
		"QLineEdit {"
			"border-top: 2px solid #3333da"
		"}"
	);

	_view->document()->setDefaultStyleSheet(
		"p { margin: 5px 0; white-space: pre }"
		".marker { color: red }"
		".sysmsg { color: yellow }"
		".announcement { color: white }"
		".nick { font-weight: bold }"
		".nick.me { color: #fff }"
		"a:link { color: #5454FF }"
	);
}

void ChatBox::clear()
{
	_view->clear();
}

void ChatBox::userJoined(int id, const QString &name)
{
	Q_UNUSED(id);
	systemMessage(tr("<b>%1</b> joined the session").arg(name.toHtmlEscaped()));
}

void ChatBox::userParted(const QString &name)
{
	systemMessage(tr("<b>%1</b> left the session").arg(name.toHtmlEscaped()));
}

void ChatBox::kicked(const QString &kickedBy)
{
	systemMessage(tr("You have been kicked by %1").arg(kickedBy.toHtmlEscaped()));
}

/**
 * The received message is displayed in the chat box.
 * @param nick nickname of the user who said something
 * @param message what was said
 * @param announcement is this a public announcement?
 * @param isme if true, the message was sent by this user
 */
void ChatBox::receiveMessage(const QString& nick, const QString& message, bool announcement, bool isme)
{
	_view->append(
		"<p class=\"chat\"><span class=\"nick" + QString(isme ? " me" : "") + "\">&lt;" +
		nick.toHtmlEscaped() +
		"&gt;</span> <span class=\"msg" + QString(announcement ? " announcement" : "") + "\">" +
		htmlutils::linkify(message.toHtmlEscaped()) +
		"</span></p>"
	);
}

void ChatBox::receiveMarker(const QString &nick, const QString &message)
{
	_view->append(
		"<p class=\"marker\"><span class=\"nick\">&lt;" +
		nick.toHtmlEscaped() +
		"&gt;</span> <span class=\"msg\">" +
		htmlutils::linkify(message.toHtmlEscaped()) +
		"</span></p>"
	);
}

/**
 * @param message the message
 */
void ChatBox::systemMessage(const QString& message)
{
	_view->append("<p class=\"sysmsg\"> *** " + message + " ***</p>");
}

void ChatBox::sendMessage(const QString &msg)
{
	if(msg.at(0) == '/') {
		// Special commands

		int split = msg.indexOf(' ');
		if(split<0)
			split = msg.length();

		QString cmd = msg.mid(1, split-1).toLower();
		QString params = msg.mid(split);
		qDebug() << "command:" << cmd << "params" << params;

		if(cmd == "clear") {
			// client side command: clear chat window
			clear();

		} else if(cmd.at(0)=='!') {
			// public announcement
			emit message(msg.mid(2), true);

		} else {
			// operator commands
			emit opCommand(cmd + params);
		}

	} else {
		// A normal chat message
		emit message(msg, false);
	}
}

void ChatBox::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	if(event->size().height() == 0) {
		if(!_wasCollapsed)
			emit expanded(false);
		_wasCollapsed = true;
	} else if(_wasCollapsed) {
		_wasCollapsed = false;
		emit expanded(true);
	}
}

}

