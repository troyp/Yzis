/* This file is part of the Yzis libraries
 *  Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>
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

#ifndef KYZISCOMMAND_H
#define KYZISCOMMAND_H

#include "viewwidget.h"
#include <klineedit.h>

class KYZisView;

/**
 * KYzis command line
 */
class KYZisCommand : public KLineEdit {
	Q_OBJECT

	public :
		KYZisCommand(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisCommand();

	protected:
		void keyPressEvent (QKeyEvent *);
		virtual void focusInEvent (QFocusEvent *);
		virtual void focusOutEvent (QFocusEvent *);

	private :
		KYZisView *_parent;
};

#endif
