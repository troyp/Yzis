/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <mikmak@yzis.org>
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

#include "ex_lua.h"
#include "debug.h"


YZExLua::YZExLua() {
	st = lua_open();
	yzDebug() << "Lua " << lua_version() << " loaded" << endl;
	if ( lua_pcall( st,0,0,0 ) )
		yzDebug() << "YZExLua::lua " << lua_tostring(st, -1) << endl;
}

YZExLua::~YZExLua() {
	lua_close(st);
}

QString YZExLua::lua(YZView *view, const QString& inputs) {
	lua_register(st,"print",print);
	lua_pushstring(st,"print");
	lua_gettable(st,LUA_GLOBALSINDEX);
	if ( lua_pcall( st,0,0,0 ) )
		yzDebug() << "YZExLua::lua " << lua_tostring(st, -1) << endl;
	return QString::null;
}

int YZExLua::print(lua_State *L) {
	yzDebug() << "Lua " << lua_version() << " tested" << endl;
	return 0;
}

#include "ex_lua.moc"

