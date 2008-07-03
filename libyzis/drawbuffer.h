/*  This file is part of the Yzis libraries
*  Copyright (C) 2006 Loic Pauleve <panard@inzenet.org>
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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

/* Qt */
#include <QList>

/* Yzis */
#include "color.h"
#include "font.h"
#include "selection.h"
#include "viewcursor.h"

class YDrawLine;
typedef QList<YDrawLine> YDrawSection;

typedef QMap<YSelectionPool::SelectionLayout, YSelection> YSelectionLayout;

class YZIS_EXPORT YDrawBuffer
{

public:

    YDrawBuffer();
    ~YDrawBuffer();

	YCursor bufferBegin() const;
	YCursor bufferEnd() const;

	inline const QList<YDrawSection> sections() { return mContent; }
	void setBufferDrawSection( int lid, YDrawSection ds );

private :
	QList<YDrawSection> mContent;

    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

};

YDebugStream& operator<< ( YDebugStream& out, const YDrawBuffer& buff );

struct YDrawCell
{
    bool valid;
    int flag;
    YFont font;
    QString c;
    YColor bg;
    YColor fg;
    int sel;
    YDrawCell():
            flag( 0 ),
            font(), c(), bg(), fg()
    {}
}
;

class YZIS_EXPORT YDrawLine {
public :
	YDrawLine();
	~YDrawLine();

    void setFont( const YFont& f );
    void setColor( const YColor& c );
    void setBackgroundColor( const YColor& c );
	// TODO: setOutline
    void setSelection( int sel );

	void clear();

    int push( const QString& c );
	void flush();

	YViewCursor beginViewCursor() const;
	YViewCursor endViewCursor() const;

	YDrawSection arrange( int columns ) const;

	inline const QList<int> steps() const { return mSteps; }
	inline const QList<YDrawCell> cells() const { return mCells; }

private:

    void insertCell( int pos = -1 );

    /*
     * copy YColor @param c into YColor* @param dest.
     * Returns true if *dest has changed, false else
     */
    static bool updateColor( YColor* dest, const YColor& c );

	void setLineCursor( int bufferY, int screenY );

	QList<YDrawCell> mCells;
	QList<int> mSteps;

	/* current cell */
    YDrawCell mCur;
    /* working cell */
    YDrawCell* mCell;

    bool changed;

	YViewCursor mBeginViewCursor;
	YViewCursor mEndViewCursor;

	friend class YDrawBuffer;
    friend YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );
};
YDebugStream& operator<< ( YDebugStream& out, const YDrawLine& dl );

#endif
