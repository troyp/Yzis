/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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

#include "debug.h"
#include <qstringlist.h>
#include <ctype.h>

//the one which writes to the files ...
static void YZDebugBackend(int area, const char* data) {
	if ( 1 ) { //output to file
		const int BUFSIZE=4096;
		char buf[ BUFSIZE ];
		int nSize = snprintf(buf, BUFSIZE,"%s", data );
		QFile out ( "/tmp/yzisdebug.log" );
		out.open(IO_WriteOnly | IO_Append );
		if ( ( nSize == -1 ) || ( nSize >= BUFSIZE ) )
			out.writeBlock( buf, BUFSIZE-1 );
		else
			out.writeBlock( buf, nSize );
/*		QTextStream stream( &out );
		stream << data << "\n";*/
		out.close();
	} else {
		FILE *output=stderr;
		fputs( data ,output);
	}
}

YZDebugStream::YZDebugStream( int _area, int _level ) {
	area = _area;
	level = _level;
}

YZDebugStream::~YZDebugStream() {
	if ( !output.isEmpty() )
		*this << "\n"; //flush
}

YZDebugStream& YZDebugStream::operator << (const QCString& string) {
	*this << string.data();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const QString& string) {
	output+=string;
	if ( output.at( output.length() - 1 ) == '\n' )
		flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const QStringList& string) {
	*this << "(";
	*this << string.join( "," );
	*this << ")";
	return *this;
}

YZDebugStream& YZDebugStream::operator << (const char* string) {
	output+=QString::fromUtf8( string );
//	if ( output.at( output.length() - 1 ) == '\n' ) 
		flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (bool i) {
	output+=QString::fromLatin1(i ? "true" : "false");
	return *this;
}

YZDebugStream& YZDebugStream::operator << (char i) {
	if ( isprint( i ) )
		output+= "\\x" + QString::number(static_cast<uint>( i )+0x100,16 ).right( 2 );
	else
		output+= i;
	if ( i == '\n' ) flush();
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned char i) {
	return operator<<(static_cast<char>(i));
}

YZDebugStream& YZDebugStream::operator << (unsigned short i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (short i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned int i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (int i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (unsigned long i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (long i) {
	QString tmp;
	tmp.setNum( i );
	output+=tmp;
	return *this;
}

YZDebugStream& YZDebugStream::operator << (double d) {
	QString tmp;
	tmp.setNum( d );
	output+=tmp;
	return *this;
}

void YZDebugStream::flush() {
	if ( output.isEmpty() ) return;
	YZDebugBackend(area, output.local8Bit().data());
	output=QString::null;
}

YZDebugStream yzDebug( int area ) {
	return YZDebugStream( area, YZ_DEBUG_LEVEL );
}
YZDebugStream yzWarning( int area ) {
	return YZDebugStream( area, YZ_WARNING_LEVEL );
}
YZDebugStream yzError( int area ) {
	return YZDebugStream( area, YZ_ERROR_LEVEL );
}
YZDebugStream yzFatal( int area ) {
	return YZDebugStream( area, YZ_FATAL_LEVEL );
}

