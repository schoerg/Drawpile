/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2006-2014 Calle Laakkonen

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

#include "widgets/netstatus.h"
#include "widgets/popupmessage.h"
#include "dialogs/certificateview.h"
#include "utils/whatismyip.h"
#include "utils/icon.h"

#include <QAction>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTimer>

namespace widgets {

namespace {
	QPixmap netstatusIcon(const char *name) {
		return icon::fromTheme(QLatin1Literal("network-") + name).pixmap(16, 16);
	}
}

NetStatus::NetStatus(QWidget *parent)
	: QWidget(parent), _sentbytes(0), _recvbytes(0), _activity(0)
{
	setMinimumHeight(16+2);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(1);
	layout->setSpacing(4);

	// Data transfer progress (not always shown)
	QVBoxLayout *progresslayout = new QVBoxLayout;
	progresslayout->setContentsMargins(0, 0, 0 ,0);
	progresslayout->setSpacing(0);

	// Upload progress bar
	_upload = new QProgressBar(this);
	_upload->setMaximumWidth(120);
	_upload->setSizePolicy(QSizePolicy());
	_upload->setTextVisible(false);
	_upload->hide();
	{
		// Red progress bar for upload
		QPalette pal = _upload->palette();
		pal.setColor(QPalette::Highlight, QColor(198, 48, 48));
		_upload->setPalette(pal);
	}
	progresslayout->addWidget(_upload);

	// Download progress bar
	_download = new QProgressBar(this);
	_download->setMaximumWidth(120);
	_download->setSizePolicy(QSizePolicy());
	_download->setTextVisible(false);
	_download->hide();
	{
		// Blue progress bar for download
		QPalette pal = _upload->palette();
		pal.setColor(QPalette::Highlight, QColor(48, 140, 198));
		_download->setPalette(pal);
	}
	progresslayout->addWidget(_download);

	layout->addLayout(progresslayout);

	// Host address label
	_label = new QLabel(tr("not connected"), this);
	_label->setTextInteractionFlags(
			Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard
			);
	_label->setCursor(Qt::IBeamCursor);
	_label->setContextMenuPolicy(Qt::ActionsContextMenu);
	layout->addWidget(_label);

	// Action to copy address to clipboard
	_copyaction = new QAction(tr("Copy address to clipboard"), this);
	_copyaction->setEnabled(false);
	_label->addAction(_copyaction);
	connect(_copyaction,SIGNAL(triggered()),this,SLOT(copyAddress()));

	// Discover local IP address
	_discoverIp = new QAction(tr("Get externally visible IP address"), this);
	_discoverIp->setVisible(false);
	_label->addAction(_discoverIp);
	connect(_discoverIp, SIGNAL(triggered()), this, SLOT(discoverAddress()));
	connect(WhatIsMyIp::instance(), SIGNAL(myAddressIs(QString)), this, SLOT(externalIpDiscovered(QString)));

	// Network connection status icon
	_icon = new QLabel(QString(), this);
	_icon->setFixedSize(QSize(16, 16));
	_icon->hide();
	layout->addWidget(_icon);

	// Security level icon
	_security = new QLabel(QString(), this);
	_security->setFixedSize(QSize(16, 16));
	_security->hide();
	layout->addWidget(_security);

	_security->setContextMenuPolicy(Qt::ActionsContextMenu);

	QAction *showcert = new QAction(tr("Show certificate"), this);
	_security->addAction(showcert);
	connect(showcert, SIGNAL(triggered()), this, SLOT(showCertificate()));

	// Popup label
	_popup = new PopupMessage(this);

	// Some styles are buggy and have bad tooltip colors, so we force the colors here.
	QPalette popupPalette;
	popupPalette.setColor(QPalette::ToolTipBase, Qt::black);
	popupPalette.setColor(QPalette::ToolTipText, Qt::white);
	_popup->setPalette(popupPalette);

	// Timer for activity update
	_timer = new QTimer(this);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateStats()));
}

/**
 * Set the label to display the address.
 * A context menu to copy the address to clipboard will be enabled.
 * @param address the address to display
 */
void NetStatus::connectingToHost(const QString& address, int port)
{
	_address = address;
	_port = port;
	_label->setText(tr("Connecting to %1...").arg(fullAddress()));
	_copyaction->setEnabled(true);
	message(_label->text());

	// Enable "discover IP" item for local host
	bool isLocal = WhatIsMyIp::isMyPrivateAddress(address);
	_discoverIp->setEnabled(isLocal);
	_discoverIp->setVisible(isLocal);

	// reset statistics
	_recvbytes = 0;
	_sentbytes = 0;
	_icon->show();
	updateIcon();
}

void NetStatus::loggedIn()
{
	_label->setText(tr("Host: %1").arg(fullAddress()));
	message(tr("Logged in!"));
}

void NetStatus::setSecurityLevel(net::Server::Security level, const QSslCertificate &certificate)
{
	QString iconname;
	QString tooltip;
	switch(level) {
	case net::Server::NO_SECURITY: break;
	case net::Server::NEW_HOST:
		iconname = "security-low";
		tooltip = tr("A previously unvisited host");
		break;

	case net::Server::KNOWN_HOST:
		iconname = "security-medium";
		tooltip = tr("Host certificate has not changed since the last visit");
		break;

	case net::Server::TRUSTED_HOST:
		iconname = "security-high";
		tooltip = tr("This is a trusted host");
		break;
	}

	if(iconname.isEmpty()) {
		_security->hide();
	} else {
		_security->setPixmap(icon::fromTheme(iconname).pixmap(16, 16));
		_security->setToolTip(tooltip);
		_security->show();
	}

	_certificate = certificate;
}

void NetStatus::hostDisconnecting()
{
	_label->setText(tr("Logging out..."));
	message(tr("Logging out..."));
}

/**
 * Set the label to indicate a lack of connection.
 * Context menu will be disabled.
 */
void NetStatus::hostDisconnected()
{
	_address = QString();
	_label->setText(tr("not connected"));

	_copyaction->setEnabled(false);
	_discoverIp->setVisible(false);

	message(tr("Disconnected"));
	_icon->hide();
	setSecurityLevel(net::Server::NO_SECURITY, QSslCertificate());
}

void NetStatus::expectBytes(int count)
{
	_download->reset();
	_download->setMaximum(count);
	_download->show();
}

void NetStatus::sendingBytes(int count)
{
	_upload->reset();
	// this is count-1, because there always seems to be exactly one extra byte.
	_upload->setMaximum(count - 1);
	_upload->show();
}

void NetStatus::bytesReceived(int count)
{
	// TODO show statistics
	_recvbytes += count;
	if(!(_activity & 2)) {
		_activity |= 2;
		updateIcon();
	}
	if(_download->isVisible()) {
		int val = _download->value() + count;
		if(val>=_download->maximum())
			_download->hide();
		else
			_download->setValue(val);
	}
	_timer->start(500);
}

void NetStatus::bytesSent(int count)
{
	// TODO show statistics
	_sentbytes += count;
	if(!(_activity & 1)) {
		_activity |= 1;
		updateIcon();
	}
	if(_upload->isVisible()) {
		int val = _upload->value() + count;
		if(val>=_upload->maximum())
			_upload->hide();
		else
			_upload->setValue(val);
	}
	_timer->start(500);
}

void NetStatus::updateStats()
{
	_activity = 0;
	updateIcon();
}

void NetStatus::updateIcon()
{
	switch(_activity) {
	case 0: _icon->setPixmap(netstatusIcon("idle")); break;
	case 1: _icon->setPixmap(netstatusIcon("transmit")); break;
	case 2: _icon->setPixmap(netstatusIcon("receive")); break;
	case 3: _icon->setPixmap(netstatusIcon("transmit-receive")); break;
	}
}

/**
 * Copy the current address to clipboard.
 * Should not be called if disconnected.
 */
void NetStatus::copyAddress()
{
	QString addr = fullAddress();
	QApplication::clipboard()->setText(addr);
	// Put address also in selection buffer so it can be pasted with
	// a middle mouse click where supported.
	QApplication::clipboard()->setText(addr, QClipboard::Selection);
}

void NetStatus::discoverAddress()
{
	WhatIsMyIp::instance()->discoverMyIp();
	_discoverIp->setEnabled(false);
}

void NetStatus::externalIpDiscovered(const QString &ip)
{
	// Only update IP if solicited
	if(_discoverIp->isVisible()) {
		_discoverIp->setEnabled(false);

		// TODO handle IPv6 style addresses
		int portsep = _address.lastIndexOf(':');
		QString port;
		if(portsep>0)
			port = _address.mid(portsep);

		_address = ip;
		_label->setText(tr("Host: %1").arg(fullAddress()));
	}
}

QString NetStatus::fullAddress() const
{
	QString addr;
	if(_port>0)
		addr = QString("%1:%2").arg(_address).arg(_port);
	else
		addr = _address;

	return addr;
}

void NetStatus::join(int id, const QString& user)
{
	Q_UNUSED(id);
	message(tr("<b>%1</b> joined").arg(user.toHtmlEscaped()));
}

void NetStatus::leave(const QString& user)
{
	message(tr("<b>%1</b> left").arg(user.toHtmlEscaped()));
}

void NetStatus::kicked(const QString& user)
{
	message(tr("You have been kicked by %1").arg(user.toHtmlEscaped()));
}

void NetStatus::message(const QString& msg)
{
	_popup->showMessage(
				mapToGlobal(_icon->pos() + QPoint(_icon->width()/2, 2)),
				msg);
	emit statusMessage(msg);
}

void NetStatus::showCertificate()
{
	dialogs::CertificateView *certdlg = new dialogs::CertificateView(_address, _certificate, parentWidget());
	certdlg->setAttribute(Qt::WA_DeleteOnClose);
	certdlg->show();
}

}

