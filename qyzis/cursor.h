/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id: cursor.h 1737 2005-03-14 22:12:59Z panard $
 */

#ifndef QYZISCURSOR_H
#define QYZISCURSOR_H

//#include "editor.h"
#include <qpixmap.h>

class QYZisEdit;
struct QYZViewCell;

class QYZisCursor {

	public :

		enum shape {
			SQUARE,
			VBAR,
			HBAR,
			RECT,
		};

		QYZisCursor( QYZisEdit* parent, shape type );
		virtual ~QYZisCursor();

		void setCursorType( shape type );
		void resize( unsigned int w, unsigned int h );
		void move( unsigned int x, unsigned int y );
		void hide();
		void refresh();

		unsigned int width() const;
		unsigned int height() const;
		shape type() const;

		const QYZViewCell& cell() const;

		inline unsigned int x() const { return mX; }
		inline unsigned int y() const { return mY; }
		inline unsigned int visible() const { return shown; }

	private :

		void drawCursor( QPixmap* orig );
		bool prepareCursors();

		QYZisEdit* mParent;

		QPixmap* bg;
		QPixmap* cursor;

		unsigned int mX;
		unsigned int mY;
		bool shown;
		shape mCursorType;

};

#endif

