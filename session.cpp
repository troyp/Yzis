/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

/*
 * $Id$
 */

#include "session.h"
#include "debug.h"
#include "schema.h"
#include "buffer.h"
#include "internal_options.h"
#include "registers.h"
#include "view.h"
#include "swapfile.h"
#include "mapping.h"
#include "ex_lua.h"
#if QT_VERSION < 0x040000
#include <qapplication.h>
#include <qdir.h>
#else
#include <QApplication>
#include <QDir>
#endif

#include "mode_command.h"
#include "mode_ex.h"
#include "mode_insert.h"
#include "mode_search.h"
#include "mode_visual.h"

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;
YZInternalOptionPool *YZSession::mOptions = 0;
YZRegisters *YZSession::mRegisters = 0;
YZSession *YZSession::me = 0;
YZEvents *YZSession::events = 0;

YZSession::YZSession( const QString& _sessionName ) {
	yzDebug() << "If you see me twice in the debug , then immediately call the police because it means yzis is damn borked ..." << endl;
	initModes();
	mSearch = new YZSearch();
	mSessionName = _sessionName;
	mCurView = 0;
	mCurBuffer = 0;
	me = this;
	events = new YZEvents();
	mSchemaManager = new YzisSchemaManager();
	mOptions = new YZInternalOptionPool();
	mRegisters = new YZRegisters();
}

YZSession::~YZSession() {
/*	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
#if QT_VERSION < 0x040000
		YZBuffer *b = ( it.data() );
#else
		YZBuffer *b = ( it.value() );
#endif
		//can't call deleteBuffer from destructor
		delete b;
	}*/

	endModes();
	delete YzisHlManager::self();
	delete mSchemaManager;
	delete mSearch;
	delete events;
	delete mRegisters;
	delete mOptions;
	delete YZMapping::self();
	delete YZExLua::instance();
	delete YZDebugBackend::instance();
}

void YZSession::initModes() {
	mModes[ YZMode::MODE_INTRO ] = new YZModeIntro();
	mModes[ YZMode::MODE_COMMAND ] = new YZModeCommand();
	mModes[ YZMode::MODE_EX ] = new YZModeEx();
	mModes[ YZMode::MODE_INSERT ] = new YZModeInsert();
	mModes[ YZMode::MODE_REPLACE ] = new YZModeReplace();
	mModes[ YZMode::MODE_VISUAL ] = new YZModeVisual();
	mModes[ YZMode::MODE_VISUAL_LINE ] = new YZModeVisualLine();
	mModes[ YZMode::MODE_SEARCH ] = new YZModeSearch();
	mModes[ YZMode::MODE_SEARCH_BACKWARD ] = new YZModeSearchBackward();
	mModes[ YZMode::MODE_COMPLETION ] = new YZModeCompletion();
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it )
#if QT_VERSION < 0x040000
		it.data()->init();
#else
		it.value()->init();
#endif
}
void YZSession::endModes() {
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it )
#if QT_VERSION < 0x040000
		delete it.data();
#else
		delete it.value();
#endif
	mModes.clear();
}
YZModeMap YZSession::getModes() {
	return mModes;
}
YZModeEx* YZSession::getExPool() {
	return (YZModeEx*)mModes[ YZMode::MODE_EX ];
}
YZModeCommand* YZSession::getCommandPool() {
	return (YZModeCommand*)mModes[ YZMode::MODE_COMMAND ];
}

void YZSession::guiStarted() {
	//read init files
#if QT_VERSION < 0x040000
	if (QFile::exists(QDir::rootDirPath() + "/etc/yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::rootDirPath() + "/etc/yzis/init.lua" );
	if (QFile::exists(QDir::homeDirPath() + "/.yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::homeDirPath() + "/.yzis/init.lua" );
#else
	if (QFile::exists(QDir::rootPath() + "/etc/yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::rootPath() + "/etc/yzis/init.lua" );
	if (QFile::exists(QDir::homePath() + "/.yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::homePath() + "/.yzis/init.lua" );
#endif
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer " << b->fileName() << endl;
	mBuffers.insert(b->fileName(), b);
	mCurBuffer = b;
}

void YZSession::rmBuffer( YZBuffer *b ) {
//	yzDebug() << "Session : rmBuffer " << b->fileName() << endl;
	if ( mBuffers.contains( b->fileName() ) ) {
			mBuffers.remove( b->fileName() );
			deleteBuffer( b );
	}
	if ( mBuffers.isEmpty() )
		exitRequest( );
//	delete b; // kinda hot,no?
}

void YZSession::registerView( YZView *v) {
	if (!mViews.contains(v->myId) ) {
		mViews[v->myId] = v;
		yzDebug() << "Registering view : "<< v->myId << endl;
	}
}

void YZSession::unregisterView( YZView *v ) {
	yzDebug() << "Unregistering view : "<< v->myId << endl;
	mViews.remove(v->myId);	
}

QString YZSession::saveBufferExit() {
	if ( saveAll() )
		quit();
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
	if ( mViews.contains(uid) )
		return mViews[uid];
	return NULL;
}

void YZSession::setCurrentView( YZView* view ) {
	yzDebug() << "Session : setCurrentView" << endl;
	mCurView = view;
	mCurBuffer = view->myBuffer();
	changeCurrentView( view );
	mCurBuffer->filenameChanged();
}

YZView* YZSession::prevView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
	
	int i = mCurView->myId;
	if ( i == 0 ) 
		i = mNbViews;
	else
		i--;
	
	YZView *v = NULL;
	while (!v && i >= 0 ) {
		if (mViews.contains(i))
			v = mViews[i];
		else
			i--;
	}
	return v;
}

YZView* YZSession::nextView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}

	int i = mCurView->myId;
	if ( i == mNbViews - 1 ) 
		i = 0;
	else
		i++;
	
	YZView *v = NULL;
	while (!v && i < mNbViews ) {
		if (mViews.contains(i))
			v = mViews[i];
		else
			i++;
	}
	return v;
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileName() == path ) return b;
	}
	return NULL; //not found
}

void YZSession::updateBufferRecord( const QString& oldname, const QString& newname, YZBuffer *buffer ) {
	mBuffers.remove( oldname );
	mBuffers.insert( newname, buffer );
}

bool YZSession::saveAll() {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	bool savedAll=true;
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( !b->fileIsNew() ) {
			if ( !b->save() ) savedAll=false;
		}
	}
	return savedAll;
}

bool YZSession::isOneBufferModified() {
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileIsNew() ) return true;
	}
	return false;
}

void YZSession::exitRequest( int errorCode ) {
	yzDebug() << "Preparing for final exit with code " << errorCode << endl;
	//prompt unsaved files XXX
	QMap<QString,YZBuffer*>::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		//might be better with deleteBuffer(b) no ? //mm
		//delete b;
		deleteBuffer( b );
	}
	mBuffers.clear();
	quit( errorCode );
}

void YZSession::sendMultipleKeys ( const QString& text) {
//	QStringList list = QStringList::split("<ENTER>", text);
/*	QStringList::Iterator it = list.begin(), end = list.end();
	for (; it != end; ++it) {*/
		YZView* cView = YZSession::me->currentView();
		cView->sendMultipleKey(/* *it + "<ENTER>" */ text);
		qApp->processEvents();
/*	}*/
}

