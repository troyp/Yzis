/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#include "internal_options.h"
#include "debug.h"
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

YZInternalOption::YZInternalOption( const QString& key, const QString& group, const QString& value, const QString& defaultValue, option_t type, value_t vtype) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValue = value;
	mValueType = vtype;
	mDefaultValue = defaultValue;
}

YZInternalOption::YZInternalOption( const QString& key, const QString& group, const QStringList& value, const QStringList& defaultValue, option_t type, value_t vtype) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValueType = vtype;
	mValue = value.join( "/YZ/" );
	mDefaultValue = defaultValue.join( "/YZ/" );
}

YZInternalOption::YZInternalOption( const QString& key, const QString& group, int value, int defaultValue, option_t type, value_t vtype) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValueType = vtype;
	mValue = QString::number( value );
	mDefaultValue = QString::number( defaultValue );
}

YZInternalOption::YZInternalOption( const QString& key, const QString& group, bool value, bool defaultValue, option_t type, value_t vtype) {
	mKey = key;
	mGroup = group;
	mType = type;
	mValueType = vtype;
	mValue = value ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
	mDefaultValue = defaultValue ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
}

YZInternalOptionPool::YZInternalOptionPool() {
	init();
	setGroup("Global");
}

YZInternalOptionPool::~YZInternalOptionPool() {
	cleanup();
	mOptions.clear();
}

YZInternalOptionPool::YZInternalOptionPool(const YZInternalOptionPool&) {
}

void YZInternalOptionPool::loadFrom(const QString& file ) {
	QFile f( file );

	if ( !f.exists() ) return;

	if ( f.open( IO_ReadOnly ) ) {
		QTextStream stream( &f );
		QRegExp rx("\\[(.*)\\]");
		QRegExp rx2( "(.*)=(.*)" );
		uint idx = 0;
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			if ( line.simplifyWhiteSpace().startsWith( "#" ) || line.isEmpty() ) continue; //skip comment and empty lines
			if ( rx.exactMatch( line ) )
				setGroup(rx.cap(1).simplifyWhiteSpace());
			else {
				if ( rx2.exactMatch( line ) ) {
					//we got an option there
					if ( rx2.cap( 2 ).simplifyWhiteSpace() == "true" ) {
						setBoolOption(rx2.cap( 1 ).simplifyWhiteSpace(), true);
					} else if ( rx2.cap( 2 ).simplifyWhiteSpace() == "false" ) {
						setBoolOption(rx2.cap( 1 ).simplifyWhiteSpace(), false);
					} else
						setQStringOption(rx2.cap( 1 ).simplifyWhiteSpace(), rx2.cap( 2 ).simplifyWhiteSpace());
				} else
					yzDebug( "YZInternalOptionPool" ) << "Error parsing line " << idx << " of " << file << endl;
			}
			idx++;
		}
		f.close();
	}
}

void YZInternalOptionPool::saveTo(const QString& file, const QString& what, bool force ) {
	QFile f( file );

	if ( f.exists() && !force ) return;

	if ( f.open( IO_WriteOnly ) ) {
		QTextStream stream( &f );
		QValueList<QString> keys = mOptions.keys();
		qHeapSort( keys );
		QValueList<QString>::iterator it;
		QString cGroup = "";
		for (it = keys.begin(); it != keys.end() ; ++it) {
			QString myGroup = QStringList::split( "\\", ( *it ) )[ 0 ];
			if ( !myGroup.startsWith( what ) ) continue; //filter !
			
			if ( myGroup != cGroup ) { // changing group
				stream << "[" << myGroup << "]\n";
				cGroup = myGroup;
			}
			//dump the option + value
			stream << QStringList::split( "\\", ( *it ) )[ 1 ] << "=" << mOptions[ ( *it ) ]->getValue() << "\n";
		}
		f.close();
	}
}

void YZInternalOptionPool::init() {
	YZInternalOption *tabstop = new YZInternalOption("tabstop", "Global", 8, 8, view_opt, int_t );
	YZInternalOption *number = new YZInternalOption("number","Global", false, false, view_opt, int_t );
	YZInternalOption *wrap = new YZInternalOption( "wrap", "Global", false, false, view_opt, bool_t );
	YZInternalOption *backspace = new YZInternalOption( "backspace", "Global", QString( "eol" ), QString( "eol" ), view_opt, string_t );
	YZInternalOption *updatecount = new YZInternalOption( "updatecount", "Global", 200, 200, buffer_opt, int_t );
	YZInternalOption *matchpairs = new YZInternalOption( "matchpairs", "Global", QString( "(){}[]" ), QString( "(){}[]" ), buffer_opt, string_t );
	YZInternalOption *cindent = new YZInternalOption( "cindent", "Global", true, true, view_opt, bool_t );
	YZInternalOption *printer = new YZInternalOption( "printer", "Global", QString("qtprinter"),QString("qtprinter"), global_opt, string_t );

	mOptions[ "Global\\tabstop" ] = tabstop;
	mOptions[ "Global\\number" ] = number;
	mOptions[ "Global\\wrap" ] = wrap;
	mOptions[ "Global\\backspace" ] = backspace;
	mOptions[ "Global\\updatecount" ] = updatecount;
	mOptions[ "Global\\matchpairs" ] = matchpairs;
	mOptions[ "Global\\cindent" ] = cindent;
	mOptions[ "Global\\printer" ] = printer;
	setGroup("Global");

	//read config files now
	initConfFiles();
}

const QString& YZInternalOptionPool::readQStringEntry( const QString& _key, const QString& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		return s;
	} else return def;
}

void YZInternalOptionPool::setQStringOption( const QString& key, const QString& option ) {
	YZInternalOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option);
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new YZInternalOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt, string_t );
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZInternalOptionPool" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

int YZInternalOptionPool::readIntEntry( const QString& _key, int def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		return s.toInt();
	} else return def;
}

void YZInternalOptionPool::setIntOption( const QString& key, int option ) {
	YZInternalOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(QString::number( option ));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new YZInternalOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt, int_t);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZInternalOptionPool" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

bool YZInternalOptionPool::readBoolEntry( const QString& _key, bool def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains(  key ) ) {
		YZInternalOption *opt = mOptions[ key ];
		if ( opt ) {
			QString s = opt->getValue();
			return s == QString::fromLatin1( "true" ) ? true : false;
		} else {
			yzDebug( "YZInternalOptionPool" ) << "Option " << key << " does not exist !" << endl;
		}
	}
	return def;
}

void YZInternalOptionPool::setBoolOption( const QString& key, bool option ) {
	YZInternalOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt )  {
		opt->setValue(option ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" ));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new YZInternalOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt, bool_t);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZInternalOptionPool" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

const QStringList& YZInternalOptionPool::readQStringListEntry( const QString& _key, const QStringList& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		const QStringList& list ( QStringList::split("/YZ/",s) );
		return list;
	} else return def;
}

void YZInternalOptionPool::setQStringListOption( const QString& key, const QStringList& option ) {
	YZInternalOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option.join("/YZ/"));
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new YZInternalOption( currentGroup, key, option , option, getOption( key ) ? getOption( key )->getType() : global_opt, string_t);
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZInternalOptionPool" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

const QColor& YZInternalOptionPool::readQColorEntry( const QString& _key, const QColor& def ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	if ( mOptions.contains( key ) ) {
		const QString& s = mOptions[ key ]->getValue();
		const QColor& col( s );
		return col;
	} else return def;
}

void YZInternalOptionPool::setQColorOption( const QString& key, const QColor& option ) {
	YZInternalOption *opt = mOptions[ currentGroup + '\\' + key ];
	if ( opt ) {
		opt->setValue(option.name());
		mOptions[ currentGroup + '\\' + key ] = opt;
	} else {
		opt = new YZInternalOption( currentGroup, key, option.name() , option.name(), getOption( key ) ? getOption( key )->getType() : global_opt, string_t );
		mOptions[ currentGroup + '\\' + key ] = opt;
//		yzDebug( "YZInternalOptionPool" ) << "New option " << currentGroup + '\\' + key << " added !" << endl;
	}
}

void YZInternalOptionPool::setGroup( const QString& group ) {
	currentGroup = group;
}

void YZInternalOptionPool::initConfFiles() {
	//first, do we have a config directory ?
	QDir homeConf( QDir::homeDirPath()+"/.yzis/" );
	if ( !homeConf.exists( QDir::homeDirPath()+"/.yzis/" ) )
		if ( !homeConf.mkdir(QDir::homeDirPath()+"/.yzis/", true) ) return;
	
	loadFrom(QDir::rootDirPath()+"/etc/yzis.conf");
	loadFrom(QDir::rootDirPath()+"/etc/yzis/yzis.conf");
	loadFrom(QDir::homeDirPath()+"/.yzis/yzis.conf");

	//load cache files
	loadFrom(QDir::homeDirPath()+"/.yzis/hl.conf");
}

bool YZInternalOptionPool::hasGroup( const QString& group ) {
	QValueList<QString> keys = mOptions.keys();
//	qHeapSort( keys );
	QValueList<QString>::iterator it;
	for (it = keys.begin(); it != keys.end() ; ++it) {
		if ( QStringList::split( "\\", ( *it ) )[ 0 ] == group ) return true;
	}
	return false;
}

void YZInternalOptionPool::cleanup() {
	QMap<QString,YZInternalOption*>::Iterator it;
	for ( it = mOptions.begin(); it != mOptions.end(); ++it ) {
		delete it.data();
	}
}

bool YZInternalOptionPool::hasOption( const QString& _key ) {
	QString key = _key;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	return mOptions.contains( key );
}

YZInternalOption *YZInternalOptionPool::getOption( const QString& option ) {
	QString key = option;
	if ( ! key.contains( '\\' ) )
		key.prepend( currentGroup+'\\' );
	YZInternalOption *opt = mOptions[ key ];
	return opt; //may be NULL
}
