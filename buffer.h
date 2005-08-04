/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <pfremy@freehackers.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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

#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * $Id$
 */

#include <qvaluevector.h>
#include <qapplication.h>
#include <qstring.h>
#include "yzis.h"
#include "syntaxhighlight.h"
#include "line.h"
#include "debug.h"
#include "option.h"
#include "view.h"
#include "selection.h"

class YZUndoBuffer;
class YZAction;
class YZDocMark;
class YZViewMark;
class YZCursor;
class YZSwapFile;
class YZSession;
class YZView;

typedef QValueVector<YZLine*> YZBufferData;
static QString myNull;

/**
 * A buffer is the implementation of the content of a file
 * A buffer can have multiple views. Every buffer is registered in a
 * @ref YZSession
 */
class YZBuffer {
public:
	//-------------------------------------------------------
	// ----------------- Constructor/Destructor and Id
	//-------------------------------------------------------
	
	/**
	 * Creates a new buffer
	 * @param sess the session to which the buffer belongs to
	 */
	YZBuffer(YZSession *sess);

	/**
	 * Default destructor
	 */
	virtual ~YZBuffer();
	
	/**
	 * Gets the unique identifier for this buffer
	 * @return the unqiue identifier for this buffer
	 */
	unsigned int getId() const { return myId; }

	//-------------------------------------------------------
	// ----------------- Character Operations
	//-------------------------------------------------------

	/**
	 * Inserts a character into the buffer
	 * @param x position on the line where to insert the character
	 * @param y line where the character is to be added
	 * @param c the character to add
	 */
	void insertChar (unsigned int x, unsigned int y, const QString& c);

	/**
	 * Deletes a character in the buffer
	 * @param x position on the line where to delete the character
	 * @param y line where the character is to be deleted
	 * @param count number of characters to delete
	 */
	void delChar (unsigned int x, unsigned int y, unsigned int count);

	//-------------------------------------------------------
	// ----------------- Line Operations
	//-------------------------------------------------------

	/**
	 * Appends a new line at the end of file
	 * @param l the line of text to be appended
	 *
	 * Note: the line is not supposed to contain '\n'
	 */
	void appendLine(const QString &l);

	/**
	 * Insert the text l in the current line
	 * @param l the text to insert
	 * @param line the line which is changed
	 */
	void insertLine(const QString &l, unsigned int line);

	/**
	 * Opens a new line at the indicated position
	 * @param col the position in line where to add a \n
	 * @param line is the line after which a new line is added
	 */
	void insertNewLine( unsigned int col, unsigned int line);

	/**
	 * Deletes the given line
	 * @param line the line number to delete
	 *
	 * Note: the valid line numbers are between 0 and lineCount()-1
	 */
	void deleteLine( unsigned int line );

	/**
	 * Replaces the line at @param line with the given string @param l
	 */
	void replaceLine( const QString& l, unsigned int line );
	
	/**
	 * Finds the @ref YZLine pointer for a line in the buffer
	 * @param line the line to return
	 * @return a YZLine pointer or 0 if none
	 *
	 * Note: the valid line numbers are between 0 and lineCount()-1
	 */
	inline YZLine * yzline(unsigned int line, bool noHL = true) {
		//if you change this method, DO NOT FORGET TO CHANGE THE ONE AFTER !
		if ( line >= mText.size() ) {
			yzDebug() << "ERROR: you are asking for line " << line << " (max is " << mText.size() << ")" << endl;
			// we will perhaps crash after that, but we don't want to disguise bugs!
			// fix the one which call yzline ( or textline ) with a wrong line number instead.
			return NULL;
		}
		YZLine *yl = mText.at( line );
		if ( !noHL && yl && !yl->initialized() ) initHL( line );
		return yl;
	}

	inline YZLine * yzline(unsigned int line) const {
		//if you change this method, DO NOT FORGET TO CHANGE THE ONE BEFORE !
		if ( line >= mText.size() ) {
			yzDebug() << "ERROR: you are asking for line " << line << " (max is " << mText.size() << ")" << endl;
			// we will perhaps crash after that, but we don't want to disguise bugs!
			// fix the one which call yzline ( or textline ) with a wrong line number instead.
			return NULL;
		}
		YZLine *yl = mText.at( line );
		return yl;
	}

	/**
	 * Replaces the given regexp @arg what with the given string @with on the specified @arg line
	 * Repeat the change on the line if @arg wholeline is true
	 * @return true if a change was done
	 */
	bool substitute( const QString& what, const QString& with, bool wholeline, unsigned int line );

	/**
	 * Get the length of a line
	 * @param line the line number
	 * @return a unsigned int with the length of the line
	 *
	 * Note: the valid line numbers are between 0 and lineCount()-1
	 */
	inline unsigned int getLineLength(unsigned int line) const {
		// if line is >= lineCount(), we will crash. Read the note from yzline()
		return yzline( line )->length();
	}

	/**
	 * Finds a line in the buffer
	 * @param line the line to search for
	 * @return a QString reference on the line or NULL
	 *
	 * Note: the valid line numbers are between 0 and lineCount()-1
	 */
	inline const QString& textline(unsigned int line) const {
		// if line is >= lineCount(), we will crash. Read the note from yzline()
		return yzline( line )->data();
	}

	/**
	 * Return the column of the first non-blank character in the line
	 */
	uint firstNonBlankChar( uint line ) const;

	//-------------------------------------------------------
	// ----------------- Buffer content
	//-------------------------------------------------------

	/**
	 * Return true if the buffer is empty
	 */
	bool isEmpty() const;

	/**
	 * Get the whole text of the buffer
	 * @return a QString containing the texts
	 */
	QString getWholeText() const;

	/**
	 * Get the length of the entire buffer
	 * @return an unsigned int with the lenght of the buffer
	 */
	uint getWholeTextLength() const;

	/**
	 * Remove all text
	 * @return void
	 */
	void clearText();

	void loadText( QString* content );

	/**
	 * Extract the corresponding 'from' and 'to' YZCursor from the YZInterval i.
	 */
	void intervalToCursors( const YZInterval& i, YZCursor* from, YZCursor* to ) const;

	/**
	 * Get a list of strings between two cursors
	 * @param from the origin cursor
	 * @param to the end cursor
	 * @return a list of strings
	 */
	QStringList getText(const YZCursor& from, const YZCursor& to) const;
	QStringList getText(const YZInterval& i) const;

	/**
	 * Get entire word at given cursor position. Currently behaves like '*' in vim
	 */
	QString getWordAt( const YZCursor& at ) const;

	/**
	 * Number of lines in the buffer
	 * @return the number of lines
	 *
	 * Note that empty buffer always have one empty line.
	 */
	unsigned int lineCount() const { return mText.count(); }

	//-------------------------------------------------------
	// --------------------- File Operations
	//-------------------------------------------------------

	/**
	 * Opens the file and fills the buffer with its content
	 */
	void load(const QString& file=QString::null);

	/**
	 * Save the buffer content into the current filename
	 * @return whether or not the file was saved correctly
	 */
	bool save();

	/**
	 * Notification that some status of the file changed
	 * Can be : file modified, file readonly ? , other ? XXX
	 * This is only informational so that GUIs can display the correct information
	 */
	void statusChanged();

	/**
	 * Get the current filename of the buffer
	 * @return the filename
	 */
	const QString& fileName() const {return mPath;}

	/**
	 * Changes the filename
	 * @param _path the new filename ( and path )
	 */
	void setPath( const QString& _path );

	/**
	 * Called whenever the filename is changed
	 */
	void filenameChanged();

	/**
	 * Is this file a new file
	 */
	bool fileIsNew() const { return mFileIsNew; }

	/**
	 * Is the file modified
	 */
	bool fileIsModified() const { return mModified; }

	/**
	 * Change the modified flag of the file
	 */
	void setChanged(bool v);
	virtual void setModified( bool modified );

	void setEncoding( const QString& name );
	inline const QString& encoding() const { return currentEncoding; }
	
	/**	
	 * Write all text for all buffers into swap file.  The
	 * original file is no longer needed for recovery.
	 */
	void preserve();
	
	//-------------------------------------------------------
	// -------------------------- View Operations
	//-------------------------------------------------------

	/**
	 * Adds a new view to the buffer
	 * @param v the view to be added
	 */
	void addView (YZView *v);

	/**
	 * Removes a view from this buffer
	 * @param v the view to be removed
	 */
	void rmView (YZView *v);

	/**
	 * The list of view for this buffer
	 * @return a QValuelist of pointers to the views
	 */
	YZList<YZView*> views() const { return mViews; }

	/**
	 * Find the first view of this buffer
	 * Temporary function
	 */
	YZView* firstView() const;

	/**
	 * Finds a view by its UID
	 * @param uid the unique ID of the view to search for
	 * @return a pointer to the view or NULL
	 */
	YZView* findView(unsigned int uid) const;

	/**
	 * Refresh all views
	 */
	void updateAllViews();

	//-------------------------------------------------------
	// ------------ Sub-object accessors
	//-------------------------------------------------------
	
	YZUndoBuffer * undoBuffer() const { return mUndoBuffer; }
	YZAction* action() const { return mAction; }
	YZViewMark* viewMarks() const { return mViewMarks; }
	YZDocMark* docMarks() const { return mDocMarks; }
	YzisHighlighting *highlight() const { return m_highlight; }

	//-------------------------------------------------------
	// ------------ Highlighting
	//-------------------------------------------------------
	
	/**
	 * Sets the highlighting mode for this buffer
	 * @param mode the highlighting mode to use
	 * @param warnGUI emit signal to GUI so they can reload the view if necessary
	 */
	void setHighLight(uint mode, bool warnGUI=true);
	void setHighLight( const QString& name );

	bool updateHL( unsigned int line );
	void initHL( unsigned int line );

	/**
	 * Notify GUIs that HL changed
	 */
	virtual void highlightingChanged();

	/**
	 * Detects the correct syntax highlighting for the current file
	 */
	void detectHighLight();

	void makeAttribs();
	
	//-------------------------------------------------------
	// ------------ Local Options Management
	//-------------------------------------------------------
	
	/**
	 * Retrieve an int option
	 */
	int getLocalIntegerOption( const QString& option ) const;

	/**
	 * Retrieve a bool option
	 */
	bool getLocalBooleanOption( const QString& option ) const;

	/**
	 * Retrieve a string option
	 */
	QString getLocalStringOption( const QString& option ) const;

	/**
	 * Retrieve a qstringlist option
	 */
	QStringList getLocalListOption( const QString& option ) const;

	//-------------------------------------------------------
	// ------------ Static
	//-------------------------------------------------------
	
	static QString tildeExpand( const QString& path );
	
protected:
	/**
	 * Sets the line @param line to @param l
	 * @param line is between 0 and lineCount()-1
	 * @param l may not contain '\n'
	 */
	void setTextline( uint line, const QString & l );

	/**
	 * Is a line displayed in any view ?
	 */
	bool isLineVisible(uint line) const;

	// The current filename (absolute path name)
	QString mPath;
	
	// list of all views that are displaying this buffer
	YZList<YZView*> mViews;

	// data structure containing the actual text of the file
	YZBufferData mText;
	
	YZSession *mSession;
	
	// pointers to sub-objects
	YZUndoBuffer *mUndoBuffer;
	YzisHighlighting *m_highlight;
	
	//if a file is new, this one is true ;) (used at saving time)
	bool mFileIsNew;
	//used to prevent redrawing of views during some operations
	bool mUpdateView;
	//is the file modified
	bool mModified;
	bool mLoading;
	
	// flag to disable drawing of updates
	mutable bool m_hlupdating;

private:
	// pointers to sub-objects
	YZAction* mAction;
	YZViewMark* mViewMarks;
	YZDocMark* mDocMarks;
	YZSwapFile *mSwap;
	
	// string containing encoding of the file
	QString currentEncoding;
	
	// unique identifier of the buffer
	unsigned int myId;
};

#endif /*  YZ_BUFFER_H */

