#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H
/**
 * $id$
 */

#include <qstringlist.h>
#include <qptrlist.h>
#include "yz_events.h"

class YZView;

class YZBuffer {

	friend class YZView;
	friend class YZSession;

public:
	/** opens a buffer using the given file */
	YZBuffer(QString _path=QString::null);

	void addChar (int x, int y, QChar c);
	void chgChar (int x, int y, QChar c);

	void load(void);
	void save(void);

	void addLine(QString &l);

	QString fileName() {return path;}

	void addView (YZView *v);

	QPtrList<YZView> views() { return view_list; }

protected:
	QString path;
	QPtrList<YZView> view_list;
	int	view_nb;

	void	postEvent(yz_event e);
	void	updateView(YZView *v);
	void	updateAllViews();
	QString	findLine(int line);

	/* readonly?, change, load, save, isclean?, ... */
	/* locking stuff will be here, too */
	QStringList text;
};

#endif /*  YZ_BUFFER_H */

