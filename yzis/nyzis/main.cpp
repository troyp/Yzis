#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "nyz_session.h"

static void finish(int sig);
static void finish2(void);
void nyz_init_screen(void);
void handle_event(yz_event *);


FILE *debugstr;

int
main(int argc, char *argv[])
{

	debugstr = fopen("/tmp/yzis.log", "a");

	(void) signal(SIGINT, finish);      /* arrange interrupts to terminate */
	atexit(finish2);

	(new NYZSession(argc,argv)) -> event_loop();

//	error("should never reach this point");
	finish(0);               /* we're done */
}

static void finish2(void)
{
	debug("finish2 called (from atexit)");
	finish(0);
}

static void finish(int sig)
{
	/* ncurses stuff */
	endwin();

	debug("finish called (sigint)");
	/* other */

	/* exit */
	exit(0);
}

 
