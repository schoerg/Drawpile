/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2008-2014 Calle Laakkonen

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
#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include <cstdint>

#include <QObject>
#include <QList>
#include <QImage>
#include <QPixmap>
#include <QBitArray>

class QDataStream;

namespace paintcore {

class Annotation;
class Layer;
class Savepoint;

/**
 * \brief A stack of layers.
 */
class LayerStack : public QObject {
	Q_OBJECT
	public:
		LayerStack(QObject *parent=0);
		~LayerStack();

		//! Adjust layer stack size
		void resize(int top, int right, int bottom, int left);

		//! Add a new layer of solid color to the top of the stack
		Layer *addLayer(int id, const QString& name, const QColor& color);

		//! Delete a layer
		bool deleteLayer(int id);

		//! Merge the layer to the one below it
		void mergeLayerDown(int id);

		//! Re-order the layer stack
		void reorderLayers(const QList<uint8_t> &neworder);

		//! Get the number of layers in the stack
		int layers() const { return _layers.count(); }

		//! Get a layer by its index
		Layer *getLayerByIndex(int index);

		//! Get a read only layer by its index
		const Layer *getLayerByIndex(int index) const;

		//! Get a layer by its ID
		Layer *getLayer(int id);

		//! Check if there are any annotations
		bool hasAnnotations() const { return !_annotations.isEmpty(); }

		//! Get an annotation by its ID
		Annotation *getAnnotation(int id);

		//! Get all annotations
		const QList<Annotation*> &annotations() const { return _annotations; }

		//! Add a new annotation
		Annotation *addAnnotation(int id, const QRect &initialrect);

		//! Move and/or resize annotation
		void reshapeAnnotation(int id, const QRect &newrect);

		//! Change annotation content
		void changeAnnotation(int id, const QString &newtext, const QColor &bgcolor);

		//! Delete an annotation
		void deleteAnnotation(int id);

		//! Get the index of the specified layer
		int indexOf(int id) const;

		//! Get the width of the layer stack
		int width() const { return _width; }

		//! Get the height of the layer stack
		int height() const { return _height; }

		//! Get the width and height of the layer stack
		QSize size() const { return QSize(_width, _height); }

		//! Paint an area of this layer stack
		void paint(const QRectF& rect, QPainter *painter);

		//! Get the merged color value at the point
		QColor colorAt(int x, int y) const;

		//! Return a flattened image of the layer stack
		QImage toFlatImage(bool includeAnnotations) const;

		//! Mark the tiles under the area dirty
		void markDirty(const QRect &area);

		//! Mark all tiles as dirty and call notifyAreaChanged
		void markDirty();

		//! Mark the tile at the given index as dirty
		void markDirty(int x, int y);

		//! Mark the tile at the given index as dirty
		void markDirty(int index);

		//! Emit areaChanged if anything has been marked as dirty
		void notifyAreaChanged();

		//! Create a new savepoint
		Savepoint *makeSavepoint();

		//! Restore layer stack to a previous savepoint
		void restoreSavepoint(const Savepoint *savepoint);

	public slots:
		//! Set or clear the "hidden" flag of a layer
		void setLayerHidden(int layerid, bool hide);

	signals:
		//! Emitted when the visible layers are edited
		void areaChanged(const QRect &area);

		//! Layer width/height changed
		void resized(int xoffset, int yoffset);

		//! Annotation with the given ID was just changed. (This includes addition and deletion)
		void annotationChanged(int id);

	private:
		void flattenTile(quint32 *data, int xindex, int yindex) const;

		int _width, _height;
		int _xtiles, _ytiles;
		QList<Layer*> _layers;
		QList<Annotation*> _annotations;

		QPixmap _cache;
		QBitArray _dirtytiles;
		QRect _dirtyrect;
};

/// Layer stack savepoint for undo use
class Savepoint {
	friend class LayerStack;
public:
	~Savepoint();

	void toDatastream(QDataStream &out) const;
	static Savepoint *fromDatastream(QDataStream &in, LayerStack *owner);

private:
	Savepoint() {}
	QList<Layer*> layers;
	QList<Annotation*> annotations;
	int width, height;
};

}

#endif

