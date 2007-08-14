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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef _KYZIS_VIEW_H_
#define _KYZIS_VIEW_H_

#include "kyziscursor.h"

#include <libyzis/view.h>

#include <QWidget>


class KYZisEditor;
class KYZisCommand;
class KYZisInfoBar;
class QSignalMapper;
class QPainter;
class KActionCollection;
class QScrollBar;

class KYZisView : public QWidget, public YZView
{
Q_OBJECT
public:
	KYZisView(YZBuffer*, QWidget*);	
	virtual ~KYZisView();

	virtual void guiScroll(int, int);
	virtual QString guiGetCommandLineText() const;
	virtual void guiSetCommandLineText(const QString&);
	virtual void guiDisplayInfo(const QString&);
	virtual void guiSyncViewInfo();
	virtual bool guiPopupFileSaveAs();
	virtual void guiFilenameChanged();
	virtual void guiHighlightingChanged();
	virtual void guiNotifyContentChanged(const YZSelection&);
	virtual void guiPreparePaintEvent(int, int);
	virtual void guiEndPaintEvent();
	virtual void guiDrawCell(QPoint, const YZDrawCell&, void*);
	virtual void guiDrawClearToEOL(QPoint, const QChar&);
	virtual void guiDrawSetLineNumber(int, int, int);
	virtual void guiDrawSetMaxLineNumber(int);
	virtual void guiModeChanged();

	const QString& convertKey( int key );
	bool containsKey( int key ) { return keys.contains( key ); }
	QString getKey( int key ) { return keys[ key ]; }
	YZDrawCell getCursorDrawCell( );
	void registerModifierKeys( const QString& keys );
	void unregisterModifierKeys( const QString& keys );
	void guiPaintEvent( const YZSelection& drawMap );

	void setFocusMainWindow();
	void setFocusCommandLine();

public slots:
	void sendMultipleKeys( const QString& );
	void scrollView( int );

protected:
	void scrollLineUp( );
	void scrollLineDown( );

private:
	QString keysToShortcut( const QString& keys );

	KYZisEditor* m_editor;
	KYZisCommand* m_command;

	void initKeys();

	// last line number
	QMap<int,QString> keys;
	KActionCollection* actionCollection;
	QSignalMapper* signalMapper;
	QPainter* m_painter;
	QScrollBar* mVScroll;
	KYZisInfoBar* m_infoBar;
};

#endif