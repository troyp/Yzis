#ifndef YZ_EVENTS_H
#define YZ_EVENTS_H
/**
 * view.h
 *
 * yzis events
 *
 */



/** list of all events */
enum yz_events {
	YZ_EV_SETLINE,
	YZ_EV_SETCURSOR,
};

/** Here are some struct used for event args handling
  * we can use long name as we shouldn't need to use those anyway
  */

#ifdef __cplusplus
	class YZLine;
#else
	struct yz_line;
#endif // __cplusplus


struct yz_event_setline {
#ifdef __cplusplus
	YZLine 	*line;
#else
	yz_line	*line;
#endif // __cplusplus
};

struct yz_event_setcursor {
	int x,y;
};




/**
  * The yz_event struct, mainly an union of all event args
  *
  * exemple of use : 
  * (e is of type yz_event)
  * switch (e.id) {
	 case YZ_EV_SETLINE:
		here we use e.u.setline.line_nb;
		here we use e.u.setline.line;
		break;

	case YZ_EV_SETCURSOR:
   }

  */
 struct yz_event_t {
	enum yz_events		id;
	struct yz_event_t	*next;
	union {
		struct yz_event_setline;
		struct yz_event_setcursor;
	} u;
};

typedef struct yz_event_t yz_event;


/**
  * Event pool
  */

class EventPool {
public:
	EventPool(void) { nb=0;}

	yz_event *getone(void) { pool[nb].next=NULL; nb++; return &pool[nb-1];}


protected:
	/** ok, it's really basic now :) */
	yz_event	pool[4000];
	int		nb;
};


#endif //  YZ_EVENTS_H

