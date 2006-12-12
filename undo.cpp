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

/**
 * This file contains the classes necessary to handle undo/redo
 */

#include "undo.h"
#include "buffer.h"
#include "action.h"
#include "view.h"
#include "debug.h"

QString YZBufferOperation::toString() const {
	QString ots;
	switch( type ) {
		case ADDTEXT: ots= "ADDTEXT"; break;
		case DELTEXT: ots= "DELTEXT"; break;
		case ADDLINE: ots= "ADDLINE"; break;
		case DELLINE: ots= "DELLINE"; break;
	}
	return QString("%1 '%2' line %3, col %4").arg(ots).arg(text).arg(line).arg(col) ;
}

void YZBufferOperation::performOperation( YZView* pView, bool opposite)
{
	OperationType t = type;

	yzDebug("YZUndoBuffer") << "YZBufferOperation: " << (opposite ? "undo " : "redo ") << toString() << endl;

	if (opposite == true) {
		switch( type ) {
			case ADDTEXT: t = DELTEXT; break;
			case DELTEXT: t = ADDTEXT; break;
			case ADDLINE: t = DELLINE; break;
			case DELLINE: t = ADDLINE; break;
		}
	}

	switch( t) {
		case ADDTEXT:
			pView->myBuffer()->action()->insertChar( pView, col, line, text );
			break;
		case DELTEXT:
			pView->myBuffer()->action()->deleteChar( pView, col, line, text.length() );
			break;
		case ADDLINE:
			pView->myBuffer()->action()->insertNewLine( pView, 0, line );
			break;
		case DELLINE:
			pView->myBuffer()->action()->deleteLine( pView, line, 1, QList<QChar>() );
			break;
	}

//	yzDebug("YZUndoBuffer") << "YZBufferOperation::performOperation Buf -> '" << buf->getWholeText() << "'\n";
}

// -------------------------------------------------------------------
//                          YZUndoItem
// -------------------------------------------------------------------

UndoItem::UndoItem()
{
	startCursorX = startCursorY = 0;
	endCursorX = endCursorY = 0;
}

// -------------------------------------------------------------------
//                          YZUndoBuffer
// -------------------------------------------------------------------

YZUndoBuffer::YZUndoBuffer( YZBuffer * buffer )
:  mBuffer(buffer), mFutureUndoItem( 0L ) {
	mCurrentIndex = 0;
	mInsideUndo = false;

	// Create the mFutureUndoItem
	commitUndoItem(0,0);
}

YZUndoBuffer::~YZUndoBuffer() {
	if ( mFutureUndoItem ) {
		for ( UndoItem::Iterator itr = mFutureUndoItem->begin(); itr != mFutureUndoItem->end(); ++itr ) {
			delete *itr;
		}
		delete mFutureUndoItem;
	}
	
	for ( YZList<UndoItem*>::Iterator itr = mUndoItemList.begin(); itr != mUndoItemList.end(); ++itr ) {
		delete *itr;
	}
}

void YZUndoBuffer::commitUndoItem(uint cursorX, uint cursorY ) {
	if (mInsideUndo == true) return;
	if (mFutureUndoItem && mFutureUndoItem->count() == 0) return;

	if (mFutureUndoItem) {
		removeUndoItemAfterCurrent();
		mFutureUndoItem->endCursorX = cursorX;
		mFutureUndoItem->endCursorY = cursorY;
		mUndoItemList.push_back( mFutureUndoItem );
		mCurrentIndex = mUndoItemList.size();
//		yzDebug("YZUndoBuffer") << "UndoItem::commitUndoItem" << toString() << endl;
	}
	mFutureUndoItem = new UndoItem();
	mFutureUndoItem->startCursorX = cursorX;
	mFutureUndoItem->startCursorY = cursorY;
}

void YZUndoBuffer::addBufferOperation( YZBufferOperation::OperationType type,
									 const QString & text,
									 uint col, uint line ) {
	if (mInsideUndo == true) return;
	YZASSERT( mFutureUndoItem != NULL );
	YZBufferOperation *bufOperation = new YZBufferOperation();
	bufOperation->type = type;
	bufOperation->text = text;
	bufOperation->line = line;
	bufOperation->col = col;
	mFutureUndoItem->push_back( bufOperation );
	removeUndoItemAfterCurrent();
}

void YZUndoBuffer::removeUndoItemAfterCurrent() {
	while( (uint)mUndoItemList.size() > mCurrentIndex )
		mUndoItemList.pop_back();
	//delete pointer XXX
}

template<typename T>
static YZList<T> reverse(const YZList<T> &yzlist)
{
	YZList<T> rev;
	for ( typename YZList<T>::ConstIterator itr = yzlist.begin(); itr != yzlist.end(); ++itr ) {
		rev.push_front( *itr );
	}
	
	return rev;
}

void YZUndoBuffer::undo( YZView* pView ) {
	if (mayUndo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );
	pView->setPaintAutoCommit(false);

	UndoItem *item = mUndoItemList[ mCurrentIndex - 1 ];
	UndoItemBase reversed = reverse( *item );
	for ( UndoItem::Iterator itr = reversed.begin(); itr != reversed.end(); ++itr ) {
		(*itr)->performOperation( pView, true );
	}
	/*
	UndoItem * undoItem = mUndoItemList.at(mCurrentIndex-1);
	UndoItemContentIterator it( *undoItem );
	it.toBack();
	while( it.hasPrevious() ) {
		bufOp = it.previous();
		bufOp->performOperation( pView, true );
	}
	*/
	mCurrentIndex--;
	pView->gotoxy(item->endCursorX, item->endCursorY);
	pView->commitPaintEvent();
	setInsideUndo( false );
}

void YZUndoBuffer::redo( YZView* pView ) {
	if (mayRedo() == false) {
		// notify the user that undo is not possible
		return;
	}
	setInsideUndo( true );
	pView->setPaintAutoCommit(false);

	++mCurrentIndex;
	
	UndoItem * undoItem = mUndoItemList[ mCurrentIndex - 1 ];
	for ( UndoItem::Iterator itr = undoItem->begin(); itr != undoItem->end(); ++itr ) {
		(*itr)->performOperation( pView, false );
	}
	
	setInsideUndo( false );
	pView->commitPaintEvent();
}

bool YZUndoBuffer::mayRedo() const {
	bool ret;
	ret = mCurrentIndex < (uint)mUndoItemList.count();
	return ret;
}

bool YZUndoBuffer::mayUndo() const {
	bool ret;
	ret = mCurrentIndex > 0;
	return ret;
}

QString YZUndoBuffer::undoItemToString( UndoItem * undoItem ) const {
	QString s;
	QString offsetS = "  ";
	s += offsetS + offsetS + "UndoItem:\n";
	if (! undoItem ) return s;
	s += offsetS + offsetS + QString("start cursor: line %1 col %2\n").arg(undoItem->startCursorX).arg(undoItem->startCursorY);
	
	UndoItemContentIterator it = undoItem->begin();
	for ( ; it != undoItem->end(); ++it ) {
		s += offsetS + offsetS + offsetS + (*it)->toString() + "\n";
	}
	s += offsetS + offsetS + QString("end cursor: line %1 col %2\n").arg(undoItem->endCursorX).arg(undoItem->endCursorY);
	
	return s;
}

QString YZUndoBuffer::toString(const QString& msg) const {
	QString s = msg + " YZUndoBuffer:\n";
	QString offsetS = "  ";
	s += offsetS + "mUndoItemList\n";

	YZList<UndoItem*>::ConstIterator it = mUndoItemList.begin();
	for ( ; it != mUndoItemList.end(); ++it ) {
		s += undoItemToString( *it );
	}

	s += offsetS + "mFutureUndoItem\n";
	s += undoItemToString( mFutureUndoItem );
	s += offsetS + "current UndoItem\n";
	s += (mCurrentIndex > 0) ? undoItemToString( mUndoItemList[ mCurrentIndex-1 ] )
		:offsetS + offsetS + "None\n";
	s += "\n";
	return s;
}

