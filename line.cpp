/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Lucijan Busch <luci@yzis.org>
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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
 * $Id$
 */

#include <qregexp.h>
#include "line.h"
#include "syntaxdocument.h"
#include "debug.h"

YZLine::YZLine(const QString &l) :
m_flags( YZLine::flagVisible ) {
	setData(l);
	m_initialized = false;
}

YZLine::YZLine() {
	setData( "" );
	m_initialized = false;
}

YZLine::~YZLine() {
}

void YZLine::setAttribs(uchar attribute, uint start, uint end) {
	m_initialized = true;
	if (end > mAttributes.size())
		end = mAttributes.size();

	for (uint z = start; z < end; z++)
		mAttributes[z] = attribute;
}

void YZLine::setData(const QString &data) {
	mData = data;
	uint len = data.length();
	if ( len == 0 ) len++; //make sure to return a non empty array ... (that sucks)
	mAttributes.resize( len );
	for ( uint i = 0; i < len; i++ )
		mAttributes[ i ] = 0;
}
