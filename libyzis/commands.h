/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
 *  Thomas Capricelli <orzel@freehackers.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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

/**
 * This file contains the list of mappings between keystrokes and commands
 */


#ifndef YZ_COMMANDS_H
#define YZ_COMMANDS_H

#include "selection.h"
#include "cursor.h"
#if QT_VERSION < 0x040000
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#else
#include <QList>
#include <QStringList>
#endif

class YZBuffer;
class YZView;
class YZCursor;
class YZCommand;

/** holds the arguments a command needs in order to execute */
struct YZCommandArgs {
	//the command that is executed
	const YZCommand *cmd;
	//the origin of inputs
	YZView *view;
	//the registers to operate upon
#if QT_VERSION < 0x040000
	QValueList<QChar> regs;
#else
	QList<QChar> regs;
#endif
	//exec this number of times the command
	unsigned int count;
	//was the count gave by the user
	bool usercount;
	//the argument
	QString arg;
	//the visual mode selection
	YZSelectionMap selection;

#if QT_VERSION < 0x040000
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QValueList<QChar> &r, unsigned int c, bool user, QString a) {
#else
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QList<QChar> &r, unsigned int c, bool user, QString a) {
#endif
		cmd=_cmd;
		view=v;
		regs=r;
		count=c;
		arg=a;
		usercount=user;
	}
#if QT_VERSION < 0x040000
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QValueList<QChar> &r, unsigned int c, bool user, const YZSelectionMap &s) {
#else
	YZCommandArgs(const YZCommand *_cmd, YZView *v, const QList<QChar> &r, unsigned int c, bool user, const YZSelectionMap &s) {
#endif
		cmd=_cmd;
		view=v;
		regs=r;
		count=c;
		selection=s;
		usercount=user;
	}
};

class YZCommandPool;
typedef QString (YZCommandPool::*PoolMethod) (const YZCommandArgs&);

enum cmd_arg {
	ARG_NONE,
	ARG_MOTION,
	ARG_CHAR,
	ARG_MARK,
	ARG_REG,
};

enum cmd_state {
	/** The command does not exist */
	CMD_ERROR,
	/** The user hasn't entered a valid, non-ambigous command yet. */
	NO_COMMAND_YET,
	/** Waiting for a motion/text object. */
	OPERATOR_PENDING,
	/** The command has been successfully executed. */
	CMD_OK,
};

/** Contains all the necessary information that makes up a normal command. @ref YZCommandPool
 * creates a list of them at startup. Note that the members of the command cannot be changed
 * after initialization. */
class YZCommand {
public:
	YZCommand( const QString &keySeq, PoolMethod pm, cmd_arg a=ARG_NONE) {
		mKeySeq=keySeq;
		mPoolMethod=pm;
		mArg=a;
	}
	virtual ~YZCommand() {}

	QString keySeq() const { return mKeySeq; }
	const PoolMethod &poolMethod() const { return mPoolMethod; }
	cmd_arg arg() const { return mArg; }

	static bool isMark(const QChar &c) {
		return c >= 'a' && c <= 'z';
	}
protected:
	/** the key sequence the command "listens to" */
	QString mKeySeq;
	/** the method of @ref YZCommandPool which will be called in order to execute the command */
	PoolMethod mPoolMethod;
	/** indicates what sort of argument this command takes */
	cmd_arg mArg;
};

class YZNewMotionArgs;

//oh please don't instanciate me twice !
class YZCommandPool {

public:
	YZCommandPool();
	~YZCommandPool();

	// this is not a QValueList because there is no constructor with no arguments for YZCommands
#if QT_VERSION < 0x040000
	QPtrList<const YZCommand> commands;
#else
	QList<YZCommand*> commands;
#endif
	QStringList textObjects;

	void initPool();

	/** This function is the entry point to execute any normal command in Yzis */
	cmd_state execCommand(YZView *view, const QString& inputs);

	/** Parses the string inputs, which must be a valid motion + argument,
	 * and executes the corresponding motion function. */
	YZCursor move(YZView *view, const QString &inputs, unsigned int count, bool usercount );

	// methods implementing motions
	YZCursor moveLeft(const YZNewMotionArgs &args);
	YZCursor moveRight(const YZNewMotionArgs &args);
	YZCursor moveLeftWrap(const YZNewMotionArgs &args);
	YZCursor moveRightWrap(const YZNewMotionArgs &args);
	YZCursor moveDown(const YZNewMotionArgs &args);
	YZCursor moveUp(const YZNewMotionArgs &args);
	YZCursor movePageUp(const YZNewMotionArgs &args);
	YZCursor movePageDown(const YZNewMotionArgs &args);
	YZCursor moveWordForward(const YZNewMotionArgs &args);
	YZCursor moveWordBackward(const YZNewMotionArgs &args);
	YZCursor gotoSOL(const YZNewMotionArgs &args);
	YZCursor gotoEOL(const YZNewMotionArgs &args);
	//YZCursor find(const YZNewMotionArgs &args);
	YZCursor findNext(const YZNewMotionArgs &args);
	YZCursor findBeforeNext(const YZNewMotionArgs &args);
	YZCursor findPrevious(const YZNewMotionArgs &args);
	YZCursor findAfterPrevious(const YZNewMotionArgs &args);
	YZCursor repeatFind(const YZNewMotionArgs &args);
	YZCursor matchPair(const YZNewMotionArgs &args);
	YZCursor firstNonBlank(const YZNewMotionArgs &args);
	YZCursor gotoMark(const YZNewMotionArgs &args);
	YZCursor firstNonBlankNextLine(const YZNewMotionArgs &args);
	YZCursor gotoLine(const YZNewMotionArgs &args);
	YZCursor searchWord(const YZNewMotionArgs &args);
	YZCursor searchNext(const YZNewMotionArgs &args);
	YZCursor searchPrev(const YZNewMotionArgs &args);

	// methods implementing commands
	QString execMotion(const YZCommandArgs &args);
	QString moveWordForward(const YZCommandArgs &args);
	QString appendAtEOL(const YZCommandArgs &args);
	QString append(const YZCommandArgs &args);
	QString changeLine(const YZCommandArgs &args);
	QString changeToEOL(const YZCommandArgs &args);
	QString deleteLine(const YZCommandArgs &args);
	QString deleteToEOL(const YZCommandArgs &args);
	QString gotoExMode(const YZCommandArgs &args);
	QString gotoLineAtTop(const YZCommandArgs &args);
	QString gotoLineAtCenter(const YZCommandArgs &args);
	QString gotoLineAtBottom(const YZCommandArgs &args);
	QString insertAtSOL(const YZCommandArgs &args);
	QString gotoInsertMode(const YZCommandArgs &args);
	QString gotoCommandMode(const YZCommandArgs &args);
	QString gotoReplaceMode(const YZCommandArgs &args);
	QString gotoVisualLineMode(const YZCommandArgs &args);
	QString gotoVisualMode(const YZCommandArgs &args);
	QString insertLineAfter(const YZCommandArgs &args);
	QString insertLineBefore(const YZCommandArgs &args);
	QString joinLine(const YZCommandArgs &args);
	QString pasteAfter(const YZCommandArgs &args);
	QString pasteBefore(const YZCommandArgs &args);
	QString yankLine(const YZCommandArgs &args);
	QString yankToEOL(const YZCommandArgs &args);
	QString closeWithoutSaving(const YZCommandArgs &args);
	QString saveAndClose(const YZCommandArgs &args);
	QString searchBackwards(const YZCommandArgs &args);
	QString searchForwards(const YZCommandArgs &args);
	QString change(const YZCommandArgs &args);
	QString del(const YZCommandArgs &args);
	QString yank(const YZCommandArgs &args);
	QString mark(const YZCommandArgs &args);
	QString undo(const YZCommandArgs &args);
	QString redo(const YZCommandArgs &args);
	QString macro(const YZCommandArgs &args);
	QString replayMacro(const YZCommandArgs &args);
	QString deleteChar(const YZCommandArgs &args);
	QString redisplay(const YZCommandArgs &args);
	QString changeCase(const YZCommandArgs &args);
	QString replace(const YZCommandArgs &args);
	QString abort(const YZCommandArgs &args);
	QString delkey(const YZCommandArgs &args);
	QString indent( const YZCommandArgs& args );

	friend class YZNewMotion;
};

class YZNewMotionArgs {
	public:
		YZNewMotionArgs(YZView *v, unsigned int cnt=1, QString a=QString::null,QString c=QString::null, bool uc = false, bool s=false) {
			cmd = c;
			view=v;
			count=cnt;
			arg=a;
			standalone=s;
			usercount = uc;
		}

		YZView *view;
		unsigned int count;
		QString arg;
		bool standalone;
		bool usercount;
		QString cmd;
};

typedef YZCursor (YZCommandPool::*MotionMethod) (const YZNewMotionArgs&);

/** This class represents a command that is also a motion. Its new member is
 * mMotionMethod, which is also a pointer to a member function of
 * @ref YZCommandPool, but which does nothing but calculate the new position
 * of the cursor. This way, other commands can easily "call" this motion by executing
 * the function whose pointer they can get with @ref motionMethod().
 * When this motion is executed as a command, the function
 * YZCommandPool::execMotion() is called which itself calls the function pointed
 * to by mMotionMethod.
 */
class YZNewMotion : public YZCommand {
public:
	YZNewMotion(const QString &keySeq, MotionMethod mm, cmd_arg a=ARG_NONE)
	: YZCommand(keySeq, &YZCommandPool::execMotion, a) {
		mMotionMethod=mm;
	}
	virtual ~YZNewMotion() {}
	const MotionMethod &motionMethod() const { return mMotionMethod; }
	/** @return true if s is a valid key sequence + argument */
	bool matches(const QString &s, bool fully=true) const;
protected:
	MotionMethod mMotionMethod;
};


#endif
