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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSslCertificate>

class Ui_SettingsDialog;
class QListWidgetItem;

namespace dialogs {

class SettingsDialog : public QDialog
{
	Q_OBJECT
	public:
		SettingsDialog(const QList<QAction*>& actions, QWidget *parent=0);
		~SettingsDialog();

	private slots:
		void rememberSettings();
		void saveCertTrustChanges();

		void validateShortcut(int row, int col);
		void viewCertificate(QListWidgetItem *item);
		void markTrustedCertificates();
		void removeCertificates();
		void certificateSelectionChanged();
		void importTrustedCertificate();

	private:
		Ui_SettingsDialog *_ui;
		QList<QAction*> _customactions;

		QStringList _removeCerts;
		QStringList _trustCerts;
		QList<QSslCertificate> _importCerts;
};

}

#endif

