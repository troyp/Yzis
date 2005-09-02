/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/**
 * $Id$
 */

#include "mark.h"
#include "cursor.h"

YZViewMark::YZViewMark( ) {
	clear( );
}
YZViewMark::~YZViewMark( ) {
	clear( );
}

void YZViewMark::clear( ) {
	YZViewMarker::Iterator it = marker.begin(), end = marker.end();
	for( ; it != end; ++it ) {
#if QT_VERSION < 0x040000
		delete it.data().bPos;
		delete it.data().dPos;
#else
		delete it.value().bPos;
		delete it.value().dPos;
#endif
	}
	marker.clear( );
}

void YZViewMark::add( const QString& mark, const YZCursor& bPos, const YZCursor& dPos ) {
	YZCursorPos pos;
	pos.bPos = new YZCursor( bPos );
	pos.dPos = new YZCursor( dPos );
	marker.insert( mark, pos );
}

void YZViewMark::del( const QString& mark ) {
	YZViewMarker::Iterator it = marker.find( mark );
	if ( it != marker.end() ) {
#if QT_VERSION < 0x040000
		delete it.data().bPos;
		delete it.data().dPos;
#else
		delete it.value().bPos;
		delete it.value().dPos;
#endif
	}
	marker.remove( mark );
}

YZCursorPos YZViewMark::get( const QString& mark, bool * found ) {
	YZViewMarker::Iterator it = marker.find( mark );
	*found = it != marker.end();
#if QT_VERSION < 0x040000
	return it.data();
#else
	return it.value();
#endif
}



void YZDocMark::clear( ) {
	marker.clear();
}

void YZDocMark::add( uint line, uint mark ) {
	if (marker.contains(line))
	{
		mark &= ~marker[line];
		if (mark == 0)
			return;
		marker[line] |= mark;
	}
	else
		marker[line] = mark;
}

void YZDocMark::del( uint line, uint mark ) {
	mark &= marker[line];
	if (mark == 0)
		return;
	marker[line] &= ~mark;
	if (marker[line] == 0)
		marker.remove(line);
}

void YZDocMark::del( uint line ) {
	marker.remove(line);
}

uint YZDocMark::get( uint line ) {
	return marker[line];
}
