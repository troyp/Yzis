/**
 * yz_interface.c
 *
 */


#include <stdlib.h>
#include "yz_view.h"

/*
 * C++ api
 */

YZView::YZView(YZBuffer *_b, int _lines_vis)
{
	buffer		= _b;
	lines_vis	= _lines_vis;
	current		= x = y = 0;
	mode 		= YZ_VIEW_MODE_COMMAND;

	events_nb_begin = 0;
	events_nb_last = 0;
	

	buffer->add_view(this);
}


/* Used by the buffer to post events */
void YZView::send_char( unicode_char_t c)
{
	switch(mode) {
		case YZ_VIEW_MODE_INSERT:
			/* handle adding a char */
			return;
		case YZ_VIEW_MODE_REPLACE:
			/* handle replacing a char */
			return;
		case YZ_VIEW_MODE_COMMAND:
			/* will be handled after the switch */
			break;
		default:
			/* ?? */
			error("unknown mode, ignoring");
			return;

	};
	/* ok, here we now we're in command */
	switch (c) {
		default:
			post_event(mk_event_setstatus("*Unknown command*"));
			break;
		case 'A': /* append -> insert mode */
			/* go to end of line */
		/* pass through */
		case 'a': /* append -> insert mode */
//			mode = YZ_VIEW_MODE_INSERT;
			post_event(mk_event_setstatus("-- INSERT --"));
			break;
		case 'R': /* -> replace mode */
//			mode = YZ_VIEW_MODE_REPLACE;
			post_event(mk_event_setstatus("-- REPLACE --"));
			break;
		case 'j': /* move down */
			break;
		case 'k': /* move up */
			break;
		case 'h': /* move left */
			break;
		case 'l': /* move right */
			break;

	}
}

yz_event *YZView::fetch_event(/* asasdfasf */)
{
//	debug("fetch_event");
	if (events_nb_last==events_nb_begin)
		return NULL;

	yz_event *e = &events[events_nb_begin];

	events_nb_begin++;
	if (events_nb_begin>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;

	return e;
}

void YZView::post_event (yz_event e)
{
//	debug("post_event");
	events[events_nb_last++] = e;
	if (events_nb_last>=YZ_EVENT_EVENTS_MAX)
		events_nb_last=0;
	if (events_nb_last==events_nb_begin)
		panic("YZ_EVENT_EVENTS_MAX exceeded");
}


/*
 * C api
 */

/*
 * constructors
 */
yz_view create_view(yz_buffer b, int lines_vis)
{
	CHECK_BUFFER(b);
	return (yz_view)(new YZView(buffer, lines_vis));
}



void yz_view_send_char(yz_view v, unicode_char_t c)
{
	CHECK_VIEW(v);
	view->send_char(c);
}

yz_event * yz_view_fetch_event(yz_view v)
{
	CHECK_VIEW(v);
	return view->fetch_event();
}

void yz_view_get_geometry(yz_view v, int *current, int *lines_vis)
{
	CHECK_VIEW(v);
	*current	= view->get_current();
	*lines_vis		= view->get_lines_visible();
}

void yz_view_post_event (yz_view v, yz_event e )
{
	CHECK_VIEW(v);
	return view->post_event(e);
}



