/* This file is part of the Yzis libraries
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */
#include "mapping.h"
#include "yzis.h"
#include "debug.h"
#include "ex_lua.h"
#include "session.h"

YZMapping *YZMapping::me = 0L;

YZMapping *YZMapping::self() {
	if (YZMapping::me == 0L) YZMapping::me = new YZMapping();
	return YZMapping::me;
}

YZMapping::YZMapping() {
	mNoremap = false;
}

YZMapping::~YZMapping() {
}

bool YZMapping::applyMappings( QString& text, QMap<QString,QString>& mappings ) {
	bool pendingMapp = false;
	QString old = text;
	QMap<QString,QString>::Iterator it = mappings.begin(), end = mappings.end();
	bool match = false;
	for (; it != end && !match ; ++it) {
		match = text.contains( it.key() );
		if ( it.data().startsWith("<Script>") && match ) {
			char *result;
			YZExLua::instance()->exe( (const char*)it.data().mid(8, it.data().length()-10), "s>s",  (const char*)it.key(), &result);
			text.replace(it.key(), result);
		} else if ( it.data().startsWith("<Noremap>") && match ) {
			text.replace(it.key(), (const char*)it.data().right(it.data().length()-9));
			mNoremap = true;
		} else if ( match ){
			text.replace(it.key(), it.data());
		} else {
			pendingMapp = pendingMapp || it.key().startsWith(text);
		}
	}
	return pendingMapp;
}

bool YZMapping::applyMappings( QString& text, int modes, bool *mapped ) {
	// yzDebug() << "Text1: " << text << endl;
	bool pendingMapp = false;
	QString old = text;
	if (mNoremap) {
		mNoremap = false;
		return pendingMapp;
	}
	if ( modes & normal )
		pendingMapp = pendingMapp || applyMappings(text, mNormalMappings);
	if ( modes & pendingop )
		pendingMapp = pendingMapp || applyMappings(text, mPendingOpMappings);
	if ( modes & visual )
		pendingMapp = pendingMapp || applyMappings(text, mVisualMappings);
	if ( modes & insert )
		pendingMapp = pendingMapp || applyMappings(text, mInsertMappings);
	if ( modes & cmdline )
		pendingMapp = pendingMapp || applyMappings(text, mCmdLineMappings);
//	yzDebug() << "Text2: " << text << endl << "Pending mapping : " << pendingMapp << endl;
	*mapped = old != text;
	return pendingMapp;
}

void YZMapping::registerModifier(const QString& map) {
	YZSession::me->registerModifier(map);
}

void YZMapping::unregisterModifier(const QString& map) {
	YZSession::me->unregisterModifier(map);
}

