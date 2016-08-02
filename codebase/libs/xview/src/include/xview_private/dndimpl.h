#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)dndimpl.h 1.15 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *      file for terms of the license.
 */

#ifndef xview_dndimpl_DEFINED
#define xview_dndimpl_DEFINED

#include <sys/time.h>
#include <X11/Xlib.h>
#include <xview/pkg.h>
#include <xview/attr.h>
#include <xview/window.h>
#include <xview/dragdrop.h>

#define DND_PRIVATE(dnd_public) XV_PRIVATE(Dnd_info, Xv_dnd_struct, dnd_public)
#define DND_PUBLIC(dnd)         XV_PUBLIC(dnd)

#define DND_POINT_IN_RECT(r, xx, yy) \
			((xx) >= (r)->x && (yy) >= (r)->y && \
	  		(xx) < (r)->x+(r)->w && (yy) < (r)->y+(r)->h)

#define DND_IS_TRANSIENT(event) (event->ie_xevent->xclient.data.l[4] & \
				 DND_TRANSIENT_FLAG)

#define DND_NO_SITE	-1

#define SUN_DND_TRANSIENT_TEMPLATE	"_SUN_DRAGDROP_TRANSIENT_%d_%d"

		/* Index into atom array */
#define TRIGGER			0
#define PREVIEW			1
#define ACK			2
#define DONE			3
#define WMSTATE			4
#define INTEREST		5
#define DSDM			6
#define NUM_ATOMS		DSDM +1

typedef enum {
    Dnd_Trigger_Remote,
    Dnd_Trigger_Local,
    Dnd_Preview,
} DndMsgType;

typedef struct dndrect {
    int		x, y;
    unsigned    w, h;
} DndRect;

typedef struct dnd_site_desc {
    Window	 window;
    long	 site_id;
    unsigned int nrects;
    DndRect	*rect;
    unsigned long flags;
} Dnd_site_desc;

typedef struct dndWaitEvent {
    Window 	window;
    int 	eventType;
    Atom	target;
} DnDWaitEvent;

typedef struct dnd_site_rects {
    long	screen_number;
    long	site_id;
    long	window;
    long	x, y;
    long	w, h;
    long	flags;
} DndSiteRects;

typedef struct dnd_info {
    Dnd			 public_self;
    Xv_window		 parent;
    DndDragType		 type;
    Atom		 atom[NUM_ATOMS];
    Xv_opaque		 cursor;
    Cursor		 xCursor;
    Xv_opaque		 affCursor;
    Cursor		 affXCursor;
    short		 transientSel;
    int			 drop_target_x;
    int			 drop_target_y;
    Dnd_site_desc   	 dropSite;
    struct timeval	 timeout;
    Xv_opaque		 window;
    Selection_requestor	 sel;
    DndSiteRects	*siteRects;
    int			 lastSiteIndex;
    int			 eventSiteIndex;
    unsigned int	 numSites;
    /* DND_HACK begin */
    short		 is_old;
    /* DND_HACK end */
    int			 incr_size;
    int			 incr_mode;  	/* Response from dsdm in INCR. */
    Window               lastRootWindow;
    int                  screenNumber;
} Dnd_info; 

Pkg_private int		dnd_init();
Pkg_private Xv_opaque	dnd_set_avlist();
Pkg_private Xv_opaque	dnd_get_attr();
Pkg_private int		dnd_destroy();

#endif /* ~xview_dndimpl_DEFINED */
