#ifndef YZ_VIEW_H
#define YZ_VIEW_H
/**
 * YZView - abstraction of a view. binded to a buffer of course
 */


#include "yz_events.h"
#include "yz_buffer.h"


#define YZ_EVENT_EVENTS_MAX 400


#ifdef __cplusplus
class YZView {
public:
	/**
	  * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
	  * this view can display
	  */
	YZView(YZBuffer *_b, int _lines);

	/**
	 * transfer a key event from gui to core
	 */
	void	send_char( unicode_char_t);

	/**
	  * Used by the buffer to post events
	  */
	void	post_event (yz_event );
	/**
	 * get a event to handle from the core.
	 * that's the way the core is sending messages to the gui
	 */
	/* for the qt/kde gui, we should create QEvents from that? */
	yz_event *	fetch_event(/* asasdfasf */);

	/**
	  * returns the number of the line displayed on top of this view
	  * (refering to the whole file/buffer
	  */
	int	get_current(void) { return current; }

	/**
	  * returns the number of line this view can display
	  */
	int	get_lines_displayed(void) { return lines; }

protected:
	YZBuffer 	*buffer; 	/** buffer we use */
	int		lines;		/** number of visible lines */
	int		current;	/** current line on top of view */
	int		x,y;		/** current cursor position */
	enum {
		YZ_VIEW_MODE_INSERT,
		YZ_VIEW_MODE_REPLACE,
		YZ_VIEW_MODE_COMMAND 
	}		mode;		/** mode of this view */


	/* fifo event loop. really basic now */
	yz_event	events[YZ_EVENT_EVENTS_MAX];
	int		events_nb_begin;
	int		events_nb_last;

	/*
	 * mode handling (on a per-view basis, no?)
	 * most of command handling and drawing stuff...
	 * yzview should update the core as to how many lines it can display (changegeometry stuff)
	 */
};


#endif // __cplusplus



/*
 * C API
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef int yz_view;

#if 0
#define CHECK_VIEW(yz_view) YZView *view; if (!yz_view || (view=(YZView*)(yz_view))->magic != MAGIC_YZVIEW) panic("");
#else
#define CHECK_VIEW(yz_view) YZView *view=(YZView*)(yz_view);
#endif




/**
 * constructor. Each view is binded to a buffer, @param lines is the initial number of lines that
 * this view can display
 */
yz_view create_view(yz_buffer , int lines);

/**
 * transfer a key event from gui to core
 */
void yz_view_send_char(yz_view , unicode_char_t c);

/**
 * get a event to handle from the core.
 * that's the way the core is sending messages to the gui
 *
 * returns the next event to be handled, or NULL if none
 */
yz_event * yz_view_fetch_event(yz_view );

/**
  * this returns the geometry of this yz_view, that is
  * 	the first line displayed
  * 	the number of line displayed
  */
void yz_view_get_geometry(yz_view , int *current, int *lines);

/*
 * used by the yz_buffer to stock events for this view
 */
void yz_view_post_event (yz_view , yz_event );


#ifdef __cplusplus
}
#endif // __cplusplus




#endif //  YZ_VIEW_H

