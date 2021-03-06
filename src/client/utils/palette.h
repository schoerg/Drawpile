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
#ifndef LOCALPALETTE_H
#define LOCALPALETTE_H

#include <QList>
#include <QColor>

class QFileInfo;

struct PaletteColor {
	QColor color;
	QString name;

	PaletteColor(const QColor &c=QColor(), const QString &n=QString()) : color(c), name(n) { }
};

class Palette {
public:
	//! Construct a blank palette
	Palette();
	explicit Palette(const QString& name, const QString& filename=QString());

	//! Load a palette from a file
	static Palette fromFile(const QFileInfo& file);

	//! Generate a default palette
	static Palette makeDefaultPalette();

	//! Set the name of the palette
	void setName(const QString& name);

	//! Get the name of the palette
	const QString& name() const { return _name; }

	//! Get the filename of the palette
	const QString& filename() const { return _filename; }

	//! Get the optimum number of columns to display the colors in
	int columns() const { return _columns; }

	//! Set the optimum number of columns
	void setColumns(int columns);

	//! Has the palette been modified since it was last saved/loaded?
	bool isModified() const { return _modified; }

	//! Save palette to file
	bool save(const QString& filename);

	//! Get the number of colors
	int count() const { return _colors.size(); }

	//! Get the color at index
	PaletteColor color(int index) const { return _colors.at(index); }

	//! Set a color at the specified index
	void setColor(int index, const PaletteColor &color);
	void setColor(int index, const QColor& color, const QString &name=QString()) { setColor(index, PaletteColor(color, name)); }

	//! Insert a new color
	void insertColor(int index, const QColor& color, const QString &name=QString());

	//! Append anew color to the end of the palette
	void appendColor(const QColor &color, const QString &name=QString());

	//! Remove a color
	void removeColor(int index);

private:
	QList<PaletteColor> _colors;
	QString _name;
	QString _filename;
	int _columns;
	bool _modified;
};

#endif

