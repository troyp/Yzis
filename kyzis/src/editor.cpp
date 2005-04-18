/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include "factory.h"

#include <kglobalsettings.h>
#include <math.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <kaction.h>
#include <qsignalmapper.h>
#include <kshortcut.h>
#include <ctype.h>

#include <kaccel.h>

#include "settings.h"

#define GETX( x ) ( isFontFixed ? ( x ) * fontMetrics().maxWidth() : x )

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QWidget( parent, name)
{
	mTransparent = false;
	mParent = parent;

	marginLeft = 0;

	setFocusPolicy( StrongFocus );
	QWidget::setCursor( IbeamCursor );

	rootxpm = new KRootPixmap( this );
	setTransparent( false );

	// for Input Method
	setInputMethodEnabled( true );

	initKeys();
	mCell.clear();
	mCursor = new KYZisCursor( this, KYZisCursor::SQUARE );
	updateCursor();

	defaultCell.isValid = true;
	defaultCell.selected = false;
	defaultCell.c = " ";
	defaultCell.flag = 0;
}


KYZisEdit::~KYZisEdit() {
	delete mCursor;
	mCell.clear();
	delete signalMapper;
	// dont delete rootxpm, this is done by KDE ;)
//	delete rootxpm;
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->take( actionCollection->action(i) );
	delete actionCollection;
}

void KYZisEdit::setTransparent ( bool t, double opacity, const QColor& color ) {
	if ( opacity == 1 )	t = false;	// opactity is max, let use scroll optimisation
	mTransparent = t;
	if ( t ) {
		rootxpm->setFadeEffect( opacity, color );
		rootxpm->start();
	} else rootxpm->stop();
}

void KYZisEdit::resizeEvent(QResizeEvent* ) {
	updateArea();
}

KYZisCursor::shape KYZisEdit::cursorShape() {
	KYZisCursor::shape s;
	if ( !isFontFixed ) {
		s = KYZisCursor::VBAR;
	} else {
		QString shape;
		YZMode::modeType m = mParent->modePool()->current()->type();
		switch( m ) {
			case YZMode::MODE_INSERT :
				shape = mParent->getLocalStringOption("cursorinsert");
				break;
			case YZMode::MODE_REPLACE :
				shape = mParent->getLocalStringOption("cursorreplace");
				break;
			case YZMode::MODE_COMPLETION :
				shape = "keep";
				break;
			default :
				shape = mParent->getLocalStringOption("cursor");
				break;
		}
		if ( shape == "hbar" ) {
			s = KYZisCursor::HBAR;
		} else if ( shape == "vbar" ) {
			s = KYZisCursor::VBAR;
		} else if ( shape == "keep" ) {
			s = mCursor->type();
		} else {
			if ( hasFocus() ) 
				s = KYZisCursor::SQUARE;
			else
				s = KYZisCursor::RECT;
		}
	}
	return s;
}
void KYZisEdit::updateCursor() {
	KYZisCursor::shape s = cursorShape();
	if ( s != mCursor->type() ) {
		mCursor->setCursorType( s );
		mCursor->refresh();
	}
}


void KYZisEdit::updateArea( ) {

	isFontFixed = fontInfo().fixedPitch();
	mParent->setFixedFont( isFontFixed );
	spaceWidth = mParent->spaceWidth;
	mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	updateCursor();

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / GETX( 1 ) - marginLeft;
	erase( );
	mCell.clear();
	mParent->setVisibleArea( columns, lines );
}

void KYZisEdit::paintEvent( QPaintEvent * pe ) {
	QRect r = pe->rect( );
	int clipx = r.x();
	int clipy = r.y();
	int clipw = r.width();
	int cliph = r.height();
	if ( isFontFixed ) {
		unsigned int linespace = fontMetrics().lineSpacing();
		unsigned int maxwidth = fontMetrics().maxWidth();
		clipx = clipx ? clipx / maxwidth : 0;
		clipy = clipy ? clipy / linespace : 0;
		clipw = clipw ? clipw / maxwidth + ( int )ceil( clipw % maxwidth ) : 0;
		cliph = cliph ? cliph / linespace + ( int )ceil( cliph % linespace ) : 0;
	}
	drawContents( clipx, clipy, clipw, cliph, pe->erased() );
}

void KYZisEdit::paintEvent( unsigned int clipx, unsigned int clipy, unsigned int clipw, unsigned int cliph ) {
	clipx = clipx - mParent->getDrawCurrentLeft( ) + marginLeft;
	unsigned int dTop = mParent->getDrawCurrentTop();
	clipy = clipy > dTop ? clipy - dTop : 0;
	drawContents( clipx, clipy, clipw, cliph, false );
}

void KYZisEdit::setCursor( int c, int l ) {
//	yzDebug() << "setCursor" << endl;
	c = c - mParent->getDrawCurrentLeft() + marginLeft;
	l -= mParent->getDrawCurrentTop();
	unsigned int x = GETX( c );
	if ( mParent->getLocalBooleanOption( "rightleft" ) ) {
		x = width() - x - mCursor->width();
	}
	mCursor->move( x, l * fontMetrics().lineSpacing() );

	// need for InputMethod (OverTheSpot)
	setMicroFocusHint( mCursor->x(), mCursor->y(), mCursor->width(), mCursor->height() );
}

QPoint KYZisEdit::cursorCoordinates( ) {
	QPoint position( mCursor->x(), mCursor->y() );
	return position;
}

void KYZisEdit::scrollUp( int n ) {
	if ( ! mTransparent ) {
		mCursor->hide();
		bitBlt( this, 0, n * fontMetrics().lineSpacing(),
			this, 0, 0,
			width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
			Qt::CopyROP, true );
		unsigned int lv = mParent->getLinesVisible();
		unsigned int i;
		for( i = lv; (int)i >= n; i-- )
			mCell[ i ] = mCell[ i - n ];
		setCursor( mParent->getCursor()->x(), mParent->getCursor()->y() );
		drawContents( 0, 0, mParent->getColumnsVisible(), n, false );
	} else {
		mParent->abortPaintEvent();
		mCell.clear();
		drawContents( 0, 0, mParent->getColumnsVisible(), mParent->getLinesVisible(), false );
	}
}

void KYZisEdit::scrollDown( int n ) {
	if ( ! mTransparent ) {
		mCursor->hide();
		bitBlt( this, 0, 0,
			this, 0, n * fontMetrics().lineSpacing(),
			width(), ( mParent->getLinesVisible() - n ) * fontMetrics().lineSpacing(),
			Qt::CopyROP, true );
		unsigned int lv = mParent->getLinesVisible();
		unsigned int maxl = lv - (unsigned int)n;
		unsigned int i;
		for( i = 0; i < maxl; i++ )
			mCell[ i ] = mCell[ i + n ];
		setCursor( mParent->getCursor()->x(), mParent->getCursor()->y() );
		drawContents( 0, maxl, lv, n, false );
	} else {
		mParent->abortPaintEvent();
		mCell.clear();
		drawContents( 0, 0, mParent->getColumnsVisible(), mParent->getLinesVisible(), false );
	}
}

bool KYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Key_Tab ) {
			keyPressEvent(ke);
			return TRUE;
		}
	}
	return QWidget::event(e);
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	ButtonState st = e->state();
	QString modifiers;
	if ( st & Qt::ShiftButton )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltButton )
		modifiers += "<ALT>";
	if ( st & Qt::ControlButton )
		modifiers += "<CTRL>";

	int lmode = mParent->modePool()->currentType();
	QString k;
	if ( keys.contains( e->key() ) ) //to handle some special keys
		k = keys[ e->key() ];
	else
		k = e->text();

	mParent->sendKey(k, modifiers);
	if ( lmode == YZMode::MODE_INSERT || lmode == YZMode::MODE_REPLACE ) {
		KYZisDoc *d = static_cast<KYZisDoc*>(mParent->document());
		emit d->emitChars(mCursor->y(), mCursor->x(),k);
	}
	e->accept();
}

void KYZisEdit::mousePressEvent ( QMouseEvent * e ) {
	/*
	FIXME: How to handle mouse events commented out now so kyzis will compile
	
	if ( mParent->myBuffer()->introShown() ) {
		mParent->myBuffer()->clearIntro();
		mParent->gotodxdy( 0, 0 );
		return;
	}
	*/

	// leave visual mode if the user clicks somewhere
	// TODO: this should only be done if the left button is used. Right button
	// should extend visual selection, like in vim.
	if ( mParent->modePool()->current()->isSelMode() )
		mParent->modePool()->pop();
	
	if (( e->button() == Qt::LeftButton ) || ( e->button() == Qt::RightButton )) {
		if (mParent->modePool()->currentType() != YZMode::MODE_EX) {
			mParent->gotodxdy( e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 ) + mParent->getDrawCurrentLeft( ) - marginLeft,
						e->y() / fontMetrics().lineSpacing() + mParent->getDrawCurrentTop( ) );
			mParent->updateStickyCol();
		}
	} else if ( e->button() == Qt::MidButton ) {
		QString text = QApplication::clipboard()->text( QClipboard::Selection );
		if ( text.isNull() )
			text = QApplication::clipboard()->text( QClipboard::Clipboard );
		if ( ! text.isNull() ) {
			if ( mParent->modePool()->current()->isEditMode() ) {
				QChar reg = '\"';
				YZSession::mRegisters->setRegister( reg, QStringList::split( "\n", text ) );
				mParent->paste( reg, false );
				mParent->moveRight();
			}
		}
	}
}

void KYZisEdit::mouseMoveEvent( QMouseEvent *e ) {
	if (e->state() == Qt::LeftButton) {
		if (mParent->modePool()->currentType() == YZMode::MODE_COMMAND) {
			// start visual mode when user makes a selection with the left mouse button
			mParent->modePool()->push( YZMode::MODE_VISUAL );
		} else if (mParent->modePool()->current()->isSelMode() ) {
			// already in visual mode - move cursor if the mouse pointer has moved over a new char
			unsigned int newX = e->x() / ( isFontFixed ? fontMetrics().maxWidth() : 1 )
				+ mParent->getDrawCurrentLeft() - marginLeft;
			unsigned int newY = e->y() / fontMetrics().lineSpacing()
				+ mParent->getDrawCurrentTop();

			if (newX != mParent->getCursor()->x() || newY != mParent->getCursor()->y()) {
				mParent->gotodxdy( newX, newY );
			}
		}
	}
}

void KYZisEdit::drawContents( int /*clipx*/, int clipy, int /*clipw*/, int cliph, bool ) {
	QPainter p;
	p.begin( this );

//	yzDebug() << "==> drawContent at " << clipx << "," << clipy << " " << clipw << " x " << cliph << endl;

	unsigned int linespace = fontMetrics().lineSpacing();
	QRect myRect;
	bool number = mParent->getLocalBooleanOption( "number" );
	bool rightleft = mParent->getLocalBooleanOption( "rightleft" );

	int flag = (rightleft ? Qt::AlignRight : Qt::AlignLeft);

	unsigned int lineCount = mParent->myBuffer()->lineCount();
	unsigned int my_marginLeft = 0;
	if ( number ) { // update marginLeft
		my_marginLeft = ( isFontFixed ? QString::number( lineCount ).length() + 2 : mParent->stringWidth( " " + QString::number( lineCount ) + "  " ) );
	}
	if ( marginLeft != my_marginLeft ) {
		if ( mCursor->visible() ) {
			mCursor->move( mCursor->x() + GETX( marginLeft - my_marginLeft ), mCursor->y() );
		}
		marginLeft = my_marginLeft;
		updateArea();
		return;
	}

	defaultCell.flag = flag;
	defaultCell.bg = backgroundColor();
	defaultCell.fg = foregroundColor();
	defaultCell.font = font();

	unsigned int currentY = mParent->initDrawContents( clipy );
	unsigned int lineNumber = 0;
	unsigned int mY = mParent->getCursor()->y() - mParent->getDrawCurrentTop();
	unsigned int w;

	while ( cliph > 0 && mParent->drawNextLine( ) ) {
		lineNumber = mParent->drawLineNumber();
		unsigned int currentX = 0;

		if ( number ) { // draw current line number
			myRect.setRect( 0, currentY * linespace, GETX( marginLeft - spaceWidth ), linespace );
			if ( rightleft ) {
				w = myRect.width();
				myRect.setLeft( width() - w );
				myRect.setWidth( w );
			}
			erase( myRect );
			QPen old_pen = p.pen( );
			if ( mParent->lineHeight() == 1 ) {
				p.setPen( Qt::yellow );
				p.setBackgroundMode( Qt::TransparentMode );
				p.setFont(font());
				p.drawText( myRect, (rightleft ? Qt::AlignLeft : Qt::AlignRight), QString::number( lineNumber ) );
			}
			currentX += marginLeft;
			p.setPen( old_pen );
		}
		myRect.setRect( GETX( currentX ), currentY * linespace, width() - GETX( currentX ), linespace );
		if ( rightleft ) {
			w = myRect.width();
			myRect.setLeft( width() - myRect.left() - w );
			myRect.setWidth( w );
		}
		erase( myRect );

		KYZViewCell cell = defaultCell;
		mCell[ currentY ].clear();

		while ( mParent->drawNextCol( ) ) {

			QString disp = mParent->drawChar();
			cell.c = disp;

			myRect.setLeft( GETX( currentX ) );
			myRect.setWidth( GETX( mParent->drawLength() ) );
			if ( rightleft ) {
				myRect.moveRight( width() - myRect.x() );
			}
			QColor c = mParent->drawColor( );
			cell.fg = c.isValid() ? c : foregroundColor();
			QColor bgColor = mParent->drawBgColor();
			cell.bg = bgColor.isValid() ? bgColor : backgroundColor();
			//other flags
			QFont myFont(font());
			if ( mParent->drawItalic() )
				myFont.setItalic( true );
			if ( mParent->drawBold() )
				myFont.setBold( true );
			myFont.setOverline(mParent->drawOverline());
			myFont.setStrikeOut(mParent->drawStrikeOutLine());
			myFont.setUnderline(mParent->drawUnderline());
			cell.font = myFont;

			cell.selected = mParent->drawSelected();

			if ( rightleft )
				disp = disp.rightJustify( mParent->drawLength(), mParent->fillChar() );
			else
				disp = disp.leftJustify( mParent->drawLength(), mParent->fillChar() );

			p.setFont(myFont);
			if ( cell.selected ) {
				p.setPen( cell.bg );
				p.setBackgroundMode( Qt::OpaqueMode );
				p.setBackgroundColor( cell.fg );
				p.eraseRect( myRect );
			} else {
				p.setPen( cell.fg );
				if ( cell.bg != backgroundColor() ) {
					p.setBackgroundMode( Qt::OpaqueMode );
					p.setBackgroundColor( cell.bg );
					p.eraseRect( myRect );
				} else {
					p.setBackgroundMode( Qt::TransparentMode );
				}
			}
			p.drawText( myRect, flag, disp );

			mCell[ currentY ][ rightleft ? myRect.right() : myRect.left() ] = cell;

			currentX += mParent->drawLength( );
		}
		if ( currentY == mY ) mCursor->refresh();
		currentY += mParent->drawHeight( );
		cliph -= mParent->lineIncrement( );
	}
	p.setPen( Settings::colorFG() );
	if ( number && clipy < currentY ) {
		if ( rightleft )
			w = width() - GETX( marginLeft ) + GETX( spaceWidth ) / 2;
		else
			w = GETX( marginLeft ) - GETX( spaceWidth ) / 2;
		p.drawLine( w, clipy * linespace, w, currentY * linespace );
	}
	unsigned int fh = height() / linespace;
	while ( cliph > 0 && currentY < fh ) {
		myRect.setRect ( 0, currentY * linespace, width(), linespace );
		erase( myRect );
		p.setPen( Qt::cyan );
		p.drawText( myRect, flag, "~" );
		++currentY;
		--cliph;
	}
	p.end( );
}

void KYZisEdit::drawCell( QPainter* p, const KYZViewCell& cell, const QRect& rect, bool reversed  ) {
	if ( ! cell.isValid ) {
		drawCell( p, defaultCell, rect, reversed );
		return;
	}
	p->setFont( cell.font );
	if ( cell.selected || reversed ) {
		p->setPen( cell.bg );
		p->setBackgroundMode( Qt::OpaqueMode );
		p->setBackgroundColor( cell.fg );
		p->eraseRect( rect );
	} else {
		p->setPen( cell.fg );
		if ( cell.bg != backgroundColor() ) {
			p->setBackgroundMode( Qt::OpaqueMode );
			p->setBackgroundColor( cell.bg );
			p->eraseRect( rect );
		} else {
			p->setBackgroundMode( Qt::TransparentMode );
		}
	}
	p->drawText( rect, cell.flag, cell.c );
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
	updateCursor();
}
void KYZisEdit::focusOutEvent ( QFocusEvent * ) {
	updateCursor();
}

void KYZisEdit::initKeys() {
	keys[ Qt::Key_Escape ] = "<ESC>" ;
	keys[ Qt::Key_Tab ] = "<TAB>" ;
	keys[ Qt::Key_Backtab ] = "<BTAB>" ;
	keys[ Qt::Key_Backspace ] = "<BS>" ;
	keys[ Qt::Key_Return ] = "<ENTER>" ;
	keys[ Qt::Key_Enter ] = "<ENTER>" ;
	keys[ Qt::Key_Insert ] = "<INS>" ;
	keys[ Qt::Key_Delete ] = "<DEL>" ;
	keys[ Qt::Key_Pause ] = "<PAUSE>" ;
	keys[ Qt::Key_Print ] = "<PRINT>" ;
	keys[ Qt::Key_SysReq ] = "<SYSREQ>" ;
	keys[ Qt::Key_Home ] = "<HOME>" ;
	keys[ Qt::Key_End ] = "<END>" ;
	keys[ Qt::Key_Left ] = "<LEFT>" ;
	keys[ Qt::Key_Up ] = "<UP>" ;
	keys[ Qt::Key_Right ] = "<RIGHT>" ;
	keys[ Qt::Key_Down ] = "<DOWN>" ;
	keys[ Qt::Key_Prior ] = "<PUP>" ;
	keys[ Qt::Key_Next ] = "<PDOWN>" ;
	keys[ Qt::Key_PageUp ] = "<PUP>" ;
	keys[ Qt::Key_PageDown ] = "<PDOWN>" ;
	keys[ Qt::Key_Shift ] = "<SHIFT>" ;
	keys[ Qt::Key_Control ] = "<CTRL>" ;
	keys[ Qt::Key_Meta ] = "<META>" ;
	keys[ Qt::Key_Alt ] = "<ALT>" ;
//hmm ignore it	keys[ Qt::Key_CapsLock ] = "<CAPSLOCK>" ;
//hmm ignore it	keys[ Qt::Key_NumLock ] = "<NUMLOCK>" ;
//hmm ignore it	keys[ Qt::Key_ScrollLock ] = "<SCROLLLOCK>" ;
	keys[ Qt::Key_Clear ] = "<CLEAR>" ;
	keys[ Qt::Key_F1 ] = "<F1>" ;
	keys[ Qt::Key_F2 ] = "<F2>" ;
	keys[ Qt::Key_F3 ] = "<F3>" ;
	keys[ Qt::Key_F4 ] = "<F4>" ;
	keys[ Qt::Key_F5 ] = "<F5>" ;
	keys[ Qt::Key_F6 ] = "<F6>" ;
	keys[ Qt::Key_F7 ] = "<F7>" ;
	keys[ Qt::Key_F8 ] = "<F8>" ;
	keys[ Qt::Key_F9 ] = "<F9>" ;
	keys[ Qt::Key_F10 ] = "<F10>" ;
	keys[ Qt::Key_F11 ] = "<F11>" ;
	keys[ Qt::Key_F12 ] = "<F12>" ;
	keys[ Qt::Key_F13 ] = "<F13>" ;
	keys[ Qt::Key_F14 ] = "<F14>" ;
	keys[ Qt::Key_F15 ] = "<F15>" ;
	keys[ Qt::Key_F16 ] = "<F16>" ;
	keys[ Qt::Key_F17 ] = "<F17>" ;
	keys[ Qt::Key_F18 ] = "<F18>" ;
	keys[ Qt::Key_F19 ] = "<F19>" ;
	keys[ Qt::Key_F20 ] = "<F20>" ;
	keys[ Qt::Key_F21 ] = "<F21>" ;
	keys[ Qt::Key_F22 ] = "<F22>" ;
	keys[ Qt::Key_F23 ] = "<F23>" ;
	keys[ Qt::Key_F24 ] = "<F24>" ;
	keys[ Qt::Key_F25 ] = "<F25>" ;
	keys[ Qt::Key_F26 ] = "<F26>" ;
	keys[ Qt::Key_F27 ] = "<F27>" ;
	keys[ Qt::Key_F28 ] = "<F28>" ;
	keys[ Qt::Key_F29 ] = "<F29>" ;
	keys[ Qt::Key_F30 ] = "<F30>" ;
	keys[ Qt::Key_F31 ] = "<F31>" ;
	keys[ Qt::Key_F32 ] = "<F32>" ;
	keys[ Qt::Key_F33 ] = "<F33>" ;
	keys[ Qt::Key_F34 ] = "<F34>" ;
	keys[ Qt::Key_F35 ] = "<F35>" ;
//	keys[ Qt::Key_BracketLeft ] = "[";
//	keys[ Qt::Key_BracketRight ] = "]";


	actionCollection = new KActionCollection( this, mParent );
	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKey( const QString& ) ) );
}

QString KYZisEdit::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
	return ret;
}

void KYZisEdit::registerModifierKeys( const QString& keys ) {
	KAction* k = new KAction( "", KShortcut( keysToShortcut( keys ) ), signalMapper, SLOT( map() ), actionCollection, keys );
	signalMapper->setMapping( k, keys );
}
void KYZisEdit::unregisterModifierKeys( const QString& keys ) {
	KAction* k = actionCollection->action( keys );
	if ( k == NULL ) {
		yzDebug() << "No KAction for " << keys << endl;
		return;
	}
	actionCollection->take( k );
	KAccel* accel = actionCollection->kaccel();
	if ( accel ) {
		accel->remove( keys );
		accel->updateConnections();
	}
	signalMapper->removeMappings( k );
	delete k;
}

void KYZisEdit::sendMultipleKey( const QString& keys ) {
	mParent->sendMultipleKey( keys );
}

const QString& KYZisEdit::convertKey( int key ) {
	return keys[ key ];
}

// for InputMethod (OnTheSpot)
void KYZisEdit::imStartEvent( QIMEvent *e )
{
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imBegin( mParent );
	}
	e->accept();
}

// for InputMethod (OnTheSpot)
void KYZisEdit::imComposeEvent( QIMEvent *e ) {
	//yzDebug() << "KYZisEdit::imComposeEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imCompose( mParent, e->text() );
		e->accept();
	} else {
		e->ignore();
	}
}

// for InputMethod (OnTheSpot)
void KYZisEdit::imEndEvent( QIMEvent *e ) {
//	yzDebug() << "KYZisEdit::imEndEvent text=" << e->text() << " len=" << e->selectionLength() << " pos=" << e->cursorPos() << endl;
	if ( mParent->modePool()->current()->supportsInputMethod() ) {
		mParent->modePool()->current()->imEnd( mParent, e->text() );
	} else {
		mParent->sendKey( e->text() );
	}
	e->accept();
}

#include "editor.moc"
