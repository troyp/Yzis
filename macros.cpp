/* This file is part of the Yzis libraries
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

#include "macros.h"
#include "debug.h"

YZMacros::YZMacros() {
	mRecording = false;
	current = QChar();
}

YZMacros::~YZMacros() {
}

const QString& YZMacros::macro( QChar c ) {
	if (  mMacros.contains( c ) )
		return mMacros[ c ];
	return QString::null;
}

void YZMacros::appendCurrentMacro( const QString& command ) {
	if ( !mRecording ) return;
	mMacros[ current ] = mMacros[ current ] + command;
}

void YZMacros::startRecording( QChar c ) { 
	mRecording = true; 
	current = c; 
	mMacros[ c ] = ""; 
}

void YZMacros::stopRecording() { 
	mRecording = false; 
	current=QChar(); 
}
