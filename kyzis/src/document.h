/* This file is part of the Yzis libraries
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KYZISDOC_H
#define KYZISDOC_H

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/configinterfaceextension.h>
#include <buffer.h>
#include <view.h>
#include <session.h>

class KYZisDoc : public KTextEditor::Document, public KTextEditor::EditInterface, public KTextEditor::HighlightingInterface, public KTextEditor::UndoInterface, public KTextEditor::ConfigInterface, public KTextEditor::ConfigInterfaceExtension, public KTextEditor::MarkInterface, public YZBuffer {
	Q_OBJECT

	public:
		KYZisDoc (int kId, QWidget *parentWidget = 0, const char *widgetName=0, QObject *parent=0, const char *name=0);
		virtual ~KYZisDoc ();

		KTextEditor::View *createView ( QWidget *parent, const char *name = 0 );
		QPtrList<KTextEditor::View> views() const { return _views; }
		void removeView( KTextEditor::View * v );
		QWidget *parentWidget() { return m_parent; }
		void setBaseWidget(QWidget *w) { setWidget(w); }

		uint numLines() const;
		QString text() const;
		uint length() const;
		bool clear() ;

		int lineLength(unsigned int line) const;
		bool insertLine( unsigned int line, const QString &s);
		bool removeLine( unsigned int line );
		bool insertText (uint line, uint col, const QString &s);

		QString textLine ( unsigned int line ) const;
		QString text (  uint startLine, uint startCol, uint endLine, uint endCol ) const;
		bool setText (  const QString &text );
		bool removeText (  uint startLine, uint startCol, uint endLine, uint endCol );
		bool popupFileSaveAs();
		void filenameChanged();
		void highlightingChanged();

		/*
		 * @internal
		 * @return the KMdi identifier to be used during DCOP internal communications
		 */
		int getkid() { return mkId; }

		void applyConfig( );


		//HL interface
		unsigned int highlightingInterfaceNumber () const ;
		virtual unsigned int hlMode ();
		virtual bool setHlMode (unsigned int mode);
		virtual unsigned int hlModeCount ();
		virtual QString hlModeName (unsigned int mode);
		virtual QString hlModeSectionName (unsigned int mode);

		//undo interface
		virtual void undo();
		virtual void redo();
		virtual void clearUndo();
		virtual void clearRedo();
		virtual unsigned int undoCount() const;
		virtual unsigned int redoCount() const;
		virtual unsigned int undoSteps() const;
		virtual void setUndoSteps(unsigned int steps);

		//config interface
		virtual void readConfig();
		virtual void writeConfig();
		virtual void readConfig( KConfig *config );
		virtual void writeConfig( KConfig *config );
		virtual void readSessionConfig( KConfig *config );
		virtual void writeSessionConfig( KConfig *config );
		virtual void configDialog();

		//configextension interface
		virtual uint configPages() const;
		virtual KTextEditor::ConfigPage *configPage ( uint number = 0, QWidget *parent = 0, const char *name=0 );
		virtual QString configPageName ( uint number = 0 ) const;
		virtual QString configPageFullName ( uint number = 0 ) const;
		virtual QPixmap configPagePixmap ( uint number = 0, int size = KIcon::SizeSmall ) const;

		/*state modification changes - to comply with undocumented kate features
		this overload emits stateChanged() signal from all views*/
		virtual void setModified(bool modified);

	public slots:
		//signals to emit
//		void configureEditor();

		//KTextEditor::MarkInterface slots
		uint mark( uint line );
		void setMark( uint line, uint markType );
		void clearMark( uint line );
		void addMark( uint line, uint markType );
		void removeMark( uint line, uint markType );
		QPtrList<KTextEditor::Mark> marks();
		void clearMarks();

	protected:
		bool openFile();
		bool saveFile();
		void setupActions();

	private:
		QPtrList<KTextEditor::View> _views;
		QWidget *m_parent;
		//KMdi identifier , used for DCOP communication to identify a buffer/view
		int mkId;

	signals:
		void hlChanged();
		void undoChanged();
		void textChanged ();
		void charactersInteractivelyInserted( int ,int ,const QString& );

		//KTextEditor::MarkInterface slots
		void marksChanged();

};

#endif