#include "kyzisedit.h"
#include <kdebug.h>

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
	: QScrollView( parent, name,WStaticContents | WRepaintNoErase | WResizeNoErase ) {
	setFont(QFont("Fixed",10));
	_parent = parent;

	viewport()->setFocusProxy( this );
	viewport()->setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	viewport()->setPaletteBackgroundColor(QColor("white"));
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines=0;
	lines = s.height() / fontMetrics().lineSpacing();
	_parent->setVisibleLines( lines );
	kdDebug()<<"Update visibles lines to : "<< lines << endl;
}

// PUBLIC API
void KYZisEdit::setCursor(int c, int l) {
	//strange we should not need a -1 //FIXME
	//undraw previous cursor
	updateContents(cursorx*fontMetrics().maxWidth(),cursory*fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing()*2);
	cursorx = c-1;
	cursory = l-1;
	//draw new cursor
	updateContents(cursorx*fontMetrics().maxWidth(),cursory*fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing()*2);
}

void KYZisEdit::setTextLine(int l, const QString &str){
	mText.insert(l,str);
	//fixme :)
	updateContents(0,( l-1 ) * fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing()*2);
// 	updateContents(); flickers ...
// 	 viewport()->repaint(); // the whole view
}

// INTERNAL API
void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->ascii() << endl;
	_parent->send_char(e->ascii());
	e->accept();
}

void KYZisEdit::mousePressEvent ( QMouseEvent * e ) {
	_parent->update_cursor(e->x()/fontMetrics().maxWidth(), e->y()/fontMetrics().lineSpacing());
}

void KYZisEdit::drawCursorAt(QPainter *p, int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),y*fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(),y*fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	//XXX draw text inside the clip
	KYZLine::iterator it;
	for (it = mText.begin(); it!=mText.end(); ++it) {
			kdDebug() << "*** Draw : " << fontMetrics().lineSpacing() * it.key() << " >= " << clipy << endl;
			kdDebug() << "*** Draw : " << fontMetrics().lineSpacing() * it.key() << " <= " << clipy+cliph << endl;
			if (fontMetrics().lineSpacing() * it.key() >= clipy && fontMetrics().lineSpacing() * it.key() <= clipy+cliph ) {
				kdDebug() << "DRAW" << endl;
				p->eraseRect(0,it.key() * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing());
				p->drawText(0,it.key() * fontMetrics().lineSpacing(),it.data());
			}
	}
	
	//XXX draw the cursor if needed
	drawCursorAt(p,cursorx,cursory);
}

#include "kyzisedit.moc"
