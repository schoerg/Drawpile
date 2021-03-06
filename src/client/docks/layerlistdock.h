/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2008-2013 Calle Laakkonen

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
#ifndef LAYERLISTDOCK_H
#define LAYERLISTDOCK_H

#include <QDockWidget>

class QModelIndex;
class QItemSelection;
class Ui_LayerBox;

namespace net {
	class Client;
	struct LayerListItem;
}

namespace docks {

class LayerAclMenu;

class LayerList : public QDockWidget
{
Q_OBJECT
public:
	LayerList(QWidget *parent=0);

	void setClient(net::Client *client);

	//! Initialize the widget for a new session
	void init();

	//! Get the ID of the currently selected layer
	int currentLayer();

	bool isCurrentLayerLocked() const;

	void setOperatorMode(bool op);
	void setControlsLocked(bool locked);

public slots:
	void selectLayer(int id);

signals:
	//! A layer was selected by the user
	void layerSelected(int id);

private slots:
	void onLayerCreate(bool wasfirst);
	void onLayerDelete(int id, int idx);
	void onLayerReorder();

	void addLayer();
	void deleteSelected();
	void opacityAdjusted();
	void blendModeChanged();
	void hiddenToggled();
	void changeLayerAcl(bool lock, QList<uint8_t> exclusive);

	void dataChanged(const QModelIndex &topLeft, const QModelIndex & bottomRight);
	void selectionChanged(const QItemSelection &selected);

private:
	void updateLockedControls();

	QModelIndex currentSelection();

	net::Client *_client;
	int _selected;
	Ui_LayerBox *_ui;
	bool _noupdate;
	LayerAclMenu *_aclmenu;
	bool _op;
	bool _lockctrl;
};

}

#endif

