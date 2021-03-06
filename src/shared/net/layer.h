/*
   DrawPile - a collaborative drawing program.

   Copyright (C) 2013 Calle Laakkonen

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
#ifndef DP_NET_LAYER_H
#define DP_NET_LAYER_H

#include <cstdint>
#include <QString>
#include <QList>

#include "message.h"

namespace protocol {

/**
 * \brief Canvas size adjustment command
 * 
 * This is the first command that must be sent to initialize the session.
 *
 * This affects the size of all existing and future layers.
 *
 * The new canvas size is relative to the old one. The four adjustement
 * parameters extend or rectract their respective borders.
 * Initial canvas resize should be (0, w, h, 0).
 */
class CanvasResize : public Message {
public:
	CanvasResize(uint8_t ctx, int16_t top, int16_t right, int16_t bottom, int16_t left)
		: Message(MSG_CANVAS_RESIZE, ctx), _top(top), _right(right), _bottom(bottom), _left(left)
		{}

	static CanvasResize *deserialize(const uchar *data, uint len);

	int16_t top() const { return _top; }
	int16_t right() const { return _right; }
	int16_t bottom() const { return _bottom; }
	int16_t left() const { return _left; }

	bool isOpCommand() const { return true; }
	bool isUndoable() const { return true; }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;	

private:
	int16_t _top;
	int16_t _right;
	int16_t _bottom;
	int16_t _left;
};

/**
 * \brief Layer creation command.
 * 
 * A session starts with zero layers, so a layer creation command is typically
 * the second command to be sent, right after setting the canvas size.
 *
 * When this command is sent by the client, the ID should be set to zero
 * to indicate the server should assign a free ID. An exception to this
 * is when generating a snapshot: in that case the client is free to use any
 * available ID number. (Typically when creating the snapshot, the layers have
 * already been assigned IDs by the server, or the session has just started meaning
 * all IDs are free.)
 *
 * If layer controls are locked, this command requires session operator privileges.
 */
class LayerCreate : public Message {
public:
	LayerCreate(uint8_t ctxid, uint8_t id, uint32_t fill, const QString &title)
		: Message(MSG_LAYER_CREATE, ctxid), _id(id), _fill(fill), _title(title.toUtf8())
		{}

	static LayerCreate *deserialize(const uchar *data, uint len);

	uint8_t id() const { return _id; }
	uint32_t fill() const { return _fill; }
	QString title() const { return QString::fromUtf8(_title); }

	void setId(uint8_t id) { _id = id; }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	uint8_t _id;
	uint32_t _fill;
	QByteArray _title;
};

/**
 * @brief Layer attribute change command
 *
 * If the current layer or layer controls in general are locked, this command
 * requires session operator privileges.
 */
class LayerAttributes : public Message {
public:
	LayerAttributes(uint8_t ctx, uint8_t id, uint8_t opacity, uint8_t blend)
		: Message(MSG_LAYER_ATTR, ctx), _id(id),
		_opacity(opacity), _blend(blend)
		{}

	static LayerAttributes *deserialize(const uchar *data, uint len);

	uint8_t id() const { return _id; }
	uint8_t opacity() const { return _opacity; }
	uint8_t blend() const { return _blend; }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	uint8_t _id;
	uint8_t _opacity;
	uint8_t _blend;
};

/**
 * @brief Layer title change command
 *
 * If the current layer or layer controls in general are locked, this command
 * requires session operator privileges.
 */
class LayerRetitle : public Message {
public:
	LayerRetitle(uint8_t ctx, uint8_t id, const QByteArray &title)
		: Message(MSG_LAYER_RETITLE, ctx), _id(id), _title(title)
		{}
	LayerRetitle(uint8_t ctx, uint8_t id, const QString &title)
		: LayerRetitle(ctx, id, title.toUtf8())
		{}

	static LayerRetitle *deserialize(const uchar *data, uint len);

	uint8_t id() const { return _id; }
	QString title() const { return QString::fromUtf8(_title); }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	uint8_t _id;
	QByteArray _title;
};

/**
 * @brief Layer order change command
 *
 * New layers are always added to the top of the stack.
 * This command includes a list of layer IDs that define the new stacking order.
 * A order change command sent by the server must list all layers.
 * A command sent by the client may be missing (newly created) layers,
 * in which case the missing layers will be included at the top of the stack in
 * their existing relative order.
 *
 * For example: if the current stack is [1,2,3,4,5] and the client sends
 * a reordering command [3,2,1], the server must add the missing layers: [3,2,1,4,5].
 *
 * If layer controls are locked, this command requires session operator privileges.
 */
class LayerOrder : public Message {
public:
	LayerOrder(uint8_t ctx, const QList<uint8_t> &order)
		: Message(MSG_LAYER_ORDER, ctx),
		_order(order)
		{}
	
	static LayerOrder *deserialize(const uchar *data, uint len);

	const QList<uint8_t> &order() const { return _order; }
	void setOrder(const QList<uint8_t> order) { _order = order; }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	QList<uint8_t> _order;
};

/**
 * @brief Layer deletion command
 *
 * If the merge attribute is set, the contents of the layer is merged
 * to the layer below it. Merging the bottom-most layer does nothing.
 *
 * If the current layer or layer controls in general are locked, this command
 * requires session operator privileges.
 */
class LayerDelete : public Message {
public:
	LayerDelete(uint8_t ctx, uint8_t id, uint8_t merge)
		: Message(MSG_LAYER_DELETE, ctx),
		_id(id),
		_merge(merge)
		{}

	static LayerDelete *deserialize(const uchar *data, uint len);

	uint8_t id() const { return _id; }
	uint8_t merge() const { return _merge; }

	bool isUndoable() const { return true; }
	
protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	uint8_t _id;
	uint8_t _merge;
};

/**
 * @brief Change layer access control list
 *
 * This is a meta command. It is used to set the general layer lock
 * as well as give exclusive access to selected users.
 */
class LayerACL : public Message {
public:
	LayerACL(uint8_t ctx, uint8_t id, uint8_t locked, const QList<uint8_t> &exclusive)
		: Message(MSG_LAYER_ACL, ctx), _id(id), _locked(locked), _exclusive(exclusive)
	{}

	static LayerACL *deserialize(const uchar *data, uint len);

	bool isOpCommand() const { return true; }

	uint8_t id() const { return _id; }
	uint8_t locked() const { return _locked; }
	const QList<uint8_t> exclusive() const { return _exclusive; }

protected:
	int payloadLength() const;
	int serializePayload(uchar *data) const;

private:
	uint8_t _id;
	uint8_t _locked;
	QList<uint8_t> _exclusive;
};

}

#endif
