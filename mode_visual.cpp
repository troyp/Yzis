/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#include "mode_visual.h"

#include "debug.h"

#include "buffer.h"
#include "selection.h"
#include "viewcursor.h"

#if QT_VERSION < 0x040000
#include <qkeysequence.h>
#include <qclipboard.h>
#else
#include <QX11Info>
#include <QApplication>
#include <QClipboard>
#endif


YZModeVisual::YZModeVisual() : YZModeCommand() {
	mType = YZMode::MODE_VISUAL;
	mString = QObject::tr( "[ Visual ]" );
	mMapMode = visual;
	commands.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete(true);
#endif
}
YZModeVisual::~YZModeVisual() {
#if QT_VERSION >= 0x040000
	for ( int ab = 0 ; ab < commands.size(); ++ab)
		delete commands.at(ab);
#endif
	commands.clear();
}

void YZModeVisual::toClipboard( YZView* mView ) {
	YZInterval interval = mView->getSelectionPool()->visual()->bufferMap()[0];
#ifndef WIN32
#if QT_VERSION < 0x040000
	if ( QPaintDevice::x11AppDisplay() )
#else
	if ( QX11Info::display() )
#endif
#endif
		QApplication::clipboard()->setText( mView->myBuffer()->getText( interval ).join( "\n" ), QClipboard::Selection );

}

YZInterval YZModeVisual::buildInterval( const YZCursor& from, const YZCursor& to ) {
	YZInterval ret( from, to );
	return ret;
}

void YZModeVisual::enter( YZView* mView ) {
	YZViewCursor* visualCursor = mView->visualCursor();
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();

	visual->clear();

	visualCursor->setBuffer( *mView->getBufferCursor() );
	visualCursor->setScreen( *mView->getCursor() );
	YZCursor buffer( *visualCursor->buffer() );
	YZCursor screen( *visualCursor->screen() );
	visual->addInterval( buildInterval(buffer,buffer), buildInterval(screen,screen) );
	mView->sendPaintEvent( visual->screenMap(), false );

	toClipboard( mView );
}
void YZModeVisual::leave( YZView* mView ) {
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();
	mView->setPaintAutoCommit( false );
	mView->sendPaintEvent( visual->screenMap(), false );
	visual->clear();
	mView->commitPaintEvent();
}
void YZModeVisual::cursorMoved( YZView* mView ) {
	YZDoubleSelection* visual = mView->getSelectionPool()->visual();

	YZInterval bufI = buildInterval(qMin(*mView->visualCursor()->buffer(),*mView->getBufferCursor()), 
					qMax(*mView->visualCursor()->buffer(),*mView->getBufferCursor()) );
	YZInterval scrI = buildInterval(qMin(*mView->visualCursor()->screen(),*mView->getCursor()), 
					qMax(*mView->visualCursor()->screen(),*mView->getCursor()) );
	YZInterval curI = visual->screenMap()[0];

	visual->clear();
	visual->addInterval( bufI, scrI );

	YZSelection tmp("tmp");
	if ( scrI.contains( curI ) ) {
		tmp.addInterval( scrI );
		tmp.delInterval( curI );
	} else {
		tmp.addInterval( curI );
		tmp.delInterval( scrI );
	}
	mView->sendPaintEvent( tmp.map(), false );

	toClipboard( mView );
}

void YZModeVisual::initCommandPool() {
	commands.append( new YZCommand("<ALT>:", (PoolMethod) &YZModeVisual::movetoExMode) );
	commands.append( new YZCommand("<CTRL>[", &YZModeCommand::gotoCommandMode) );
	commands.append( new YZCommand("<DEL>", &YZModeCommand::del) );
	commands.append( new YZCommand("<ESC>", (PoolMethod) &YZModeVisual::escape) );
	commands.append( new YZCommand(":", (PoolMethod) &YZModeVisual::gotoExMode ) );
	commands.append( new YZCommand("A", (PoolMethod) &YZModeVisual::commandAppend ) );
	commands.append( new YZCommand("I", (PoolMethod) &YZModeVisual::commandInsert ) );
	commands.append( new YZCommand("c", &YZModeCommand::change) );
	commands.append( new YZCommand("d", &YZModeCommand::del) );
	commands.append( new YZCommand("y", &YZModeCommand::yank) );
	commands.append( new YZCommand("x", &YZModeCommand::yank) );
	commands.append( new YZCommand(">", &YZModeCommand::indent) );
	commands.append( new YZCommand("<", &YZModeCommand::indent) );
}
void YZModeVisual::commandAppend( const YZCommandArgs& args ) {
	YZCursor pos = qMax( *args.view->visualCursor()->buffer(), *args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.getX(), pos.getY() );
}
void YZModeVisual::commandInsert( const YZCommandArgs& args ) {
	YZCursor pos = qMin( *args.view->visualCursor()->buffer(), *args.view->getBufferCursor() );
	args.view->modePool()->change( MODE_INSERT );
	args.view->gotoxy( pos.getX(), pos.getY() );
}
void YZModeVisual::escape( const YZCommandArgs& args ) {
	args.view->modePool()->pop();
}
void YZModeVisual::gotoExMode( const YZCommandArgs& args ) {
	args.view->modePool()->push( MODE_EX );
	args.view->setCommandLineText( "'<,'>" );
}
void YZModeVisual::movetoExMode( const YZCommandArgs& args ) {
	args.view->modePool()->change( MODE_EX );
}

YZInterval YZModeVisual::interval(const YZCommandArgs& args ) {
	return args.view->getSelectionPool()->visual()->bufferMap()[0];
}

/**
 * MODE VISUAL LINES
 */

YZModeVisualLine::YZModeVisualLine() : YZModeVisual() {
	mType = YZMode::MODE_VISUAL_LINE;
	mString = QObject::tr("[ Visual Line ]");
}
YZModeVisualLine::~YZModeVisualLine() {
}

YZInterval YZModeVisualLine::buildInterval( const YZCursor& from, const YZCursor& to ) {
	YZBound bf( from );
	YZBound bt( to, true );
	bf.setPos( 0, from.getY() );
	bt.setPos( 0, to.getY() + 1 );
	YZInterval ret( bf, bt );
	return ret;
}


