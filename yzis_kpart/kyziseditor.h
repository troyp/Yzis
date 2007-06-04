/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef _KYZIS_EDITOR_H_
#define _KYZIS_EDITOR_H_

#include "kyziscursor.h"

#include <QWidget>

class KYZisView;
class YZDrawCell;
class YZSelection;

class KYZisEditor : public QWidget
{
Q_OBJECT
public:
	KYZisEditor(KYZisView* parent = 0);
	~KYZisEditor();

	//erase all text, and set new text
	void setText (const QString& );

	//append text
	void append ( const QString& );

	//move cursor to position column, line relative to viewport
	void setCursor(int c,int l);
	void scrollUp( int );
	void scrollDown( int );

	KYZisCursor::shape cursorShape();
	void updateCursor();
	// update text area
	void updateArea( );

	void setPalette( const QColor& fg, const QColor& bg, double opacity );

	unsigned int spaceWidth;

	QPoint cursorCoordinates( );

	QVariant inputMethodQuery ( Qt::InputMethodQuery query );
	void guiDrawCell( int x, int y, const YZDrawCell& cell, QPainter* p );
	void guiDrawClearToEOL( int x, int y, const QChar& clearChar, QPainter* p );
	void guiDrawSetMaxLineNumber( int max );
	void guiDrawSetLineNumber( int y, int n, int h, QPainter* p );
	void drawMarginLeft( int min_y, int max_y, QPainter* p );

	bool insidePaintEvent( ) { return m_insidePaintEvent; }
	void paintEvent( const YZSelection& drawMap );

public slots :
	void sendMultipleKey( const QString& keys ) {}


protected:

	//intercept tabs
	virtual bool event(QEvent*);

	void resizeEvent(QResizeEvent*);
	void paintEvent(QPaintEvent*);

	//normal keypressEvents processing
	void keyPressEvent (QKeyEvent *);

	//mouse events
	void mousePressEvent (QMouseEvent *);

	//mouse move event
	void mouseMoveEvent( QMouseEvent *);

	// mousebutton released
//              void mouseReleaseEvent( QMouseEvent *);
	//insert text at line
	void insetTextAt(const QString&, int line);

	//insert a char at idx on line ....
	void insertCharAt(QChar,int);

	//replace a char at idx on line ....
	void replaceCharAt( QChar,int );

	//number of lines
	long lines();

	virtual void focusInEvent( QFocusEvent * );
	virtual void focusOutEvent( QFocusEvent * );

	// for InputMethod
	void inputMethodEvent ( QInputMethodEvent * );

	bool fakeLine;

private :
	
	KYZisCursor* mCursor;
	QFontMetrics *standard;
	QFontMetrics *standardBold;
	QFontMetrics *standardBoldItalic;

	bool isFontFixed;

	bool m_insidePaintEvent;
	/**
	 * size of the left margin (used to draw line number)
	 */
	int marginLeft;

	KYZisView* m_parent;
};

#endif
