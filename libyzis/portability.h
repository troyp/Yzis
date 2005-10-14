/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef YZIS_WIN32_MSVC

	// boah, we are on windows
	#include <windows.h>
	
	// XXX Phil: I'll fix that later 
	#define PREFIX ""
	#define gettext( s ) (s) 
	
	// emulate chmod
	#define chmod( fname , flag )
	#define S_IRUSR 0 
	#define S_IWUSR 0
	
	// emulate lstat
	#define lstat	stat
	
	// make stat work
	#define S_ISLNK( v )		(0)
	#define S_ISREG( v )		(v & _S_IFREG)
	
	// make geteuid work
	#define CHECK_GETEUID( v )		(1)
#define PREFIX ""

#else 
	// ahh, we are on unix
	#include <unistd.h>
	#include <dirent.h>
	#include <pwd.h>
	//#include "config.h"
	#include "translator.h"
	#include <libintl.h>
	
	#define CHECK_GETEUID( v )		(v == geteuid())
#endif

// Qt4 check
#include <qnamespace.h>

// if Qt3
#if QT_VERSION < 0x040000
	#include <qvaluelist.h>
	template<typename T>
	class YZList : public QValueList<T> {}; 
	
	#include <qvaluevector.h>
	template<typename T>
	class YZVector : public QValueVector<T> {};
#else // Qt4 then
	#include <qlist.h>
	template<typename T>
	class YZList : public QList<T> {};
	
	#include <qvector.h>
	template<typename T>
	class YZVector : public QVector<T> {};
#endif

#endif // PORTABILITY_H
