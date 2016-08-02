#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)sb_set.c 1.52 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Module:	sb_set.c
 * 
 * Description: sets scrollbar attributes
 * 
 */

/*
 * Include files:
 */

#include <xview_private/sb_impl.h>
#include <xview_private/draw_impl.h>
#include <xview/xv_error.h>

/*
 * Declaration of Functions Defined in This File (in order):
 */

Pkg_private Xv_opaque scrollbar_set_internal();

static int      scrollbar_parse_attr();

/******************************************************************/

Pkg_private     Xv_opaque
scrollbar_set_internal(scroll_public, avlist)
    Scrollbar       scroll_public;
    Attr_avlist     avlist;
{
    Xv_scrollbar_info *sb = SCROLLBAR_PRIVATE(scroll_public);

    scrollbar_parse_attr(sb, avlist);

    return (XV_OK);
}

static int
scrollbar_parse_attr(sb, argv)
    Xv_scrollbar_info *sb;
    Attr_avlist     argv;
{
    Attr_attribute   attr;
    int              position_elevator = FALSE;
    long unsigned    view_start = 0;
    int              view_start_set = FALSE;
    
    while (attr = *argv++) {
	switch (attr) {
	  case SCROLLBAR_INACTIVE:
	    sb->inactive = (int) *argv;
	    sb->elevator_state = (sb->inactive) ? OLGX_INACTIVE : 0;
	    if (!sb->creating) {
		position_elevator = TRUE;	/* repaint elevator */
	    }
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_PIXELS_PER_UNIT:
	    {
		int pixels = (int)*argv;
		
		if (pixels > 0) {
		    if (!sb->creating) {
			sb->view_length = sb->view_length * 
			  sb->pixels_per_unit / pixels;
			sb->object_length = sb->object_length *
			  sb->pixels_per_unit / pixels;
			sb->pixels_per_unit = pixels;
			scrollbar_paint(SCROLLBAR_PUBLIC(sb));
		    } else
		      sb->pixels_per_unit = pixels;
		}
	    }
	    argv = attr_skip(attr, argv);
	    break;
	    
	  case SCROLLBAR_OBJECT_LENGTH:
	    if ((int) *argv >= 0 && (int) *argv != sb->object_length) {
		sb->object_length = (long unsigned) *argv;
		if (!sb->creating) {
		    position_elevator = TRUE;
		}
	    }
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_OVERSCROLL:
	    /* This is only here for binary compat */
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_VIEW_START:
	    view_start = (long unsigned) *argv;
	    view_start_set = TRUE;
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_VIEW_LENGTH:
	    if ((int) *argv >= 0 && (int) *argv != sb->view_length) {
		sb->view_length = (int) *argv;
	    }
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_PAGE_LENGTH:
	    if ((int) *argv >= 0) {
		sb->page_length = (int) *argv;
	    }
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_SPLITTABLE:
	    sb->can_split = (int) *argv;
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_NORMALIZE_PROC:
	    sb->normalize_proc = (int (*) ()) *argv;
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_COMPUTE_SCROLL_PROC:
	    sb->compute_scroll_proc = (void (*) ()) *argv;
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_NOTIFY_CLIENT:
	    sb->managee = (Xv_opaque) * argv;
	    argv = attr_skip(attr, argv);
	    break;

	  case SCROLLBAR_DIRECTION:
	    sb->direction = (Scrollbar_setting) * argv;
	    if (!sb->creating) {
		scrollbar_init_positions(sb);
	    }
	    argv = attr_skip(attr, argv);
	    break;
	  
          case SCROLLBAR_PERCENT_OF_DRAG_REPAINTS:
	    sb->drag_repaint_percent = (int)*argv;
	    if(sb->drag_repaint_percent > 100)
		sb->drag_repaint_percent = 100;
	    if(sb->drag_repaint_percent < 0)
		sb->drag_repaint_percent = 0;
	    argv = attr_skip(attr, argv);
            break;

	  case WIN_CMS_CHANGE:
	    if (sb->ginfo)
	      /* should also clear window */
	      scrollbar_paint(SCROLLBAR_PUBLIC(sb));
	    break;
	    
	  case XV_END_CREATE:
	    sb->creating = FALSE;
	    sb->length = (int)xv_get(SCROLLBAR_PUBLIC(sb),
				     (sb->direction == SCROLLBAR_VERTICAL) ?
				     XV_HEIGHT : XV_WIDTH, NULL);
	    scrollbar_init_positions(sb);	    
	    argv = attr_skip(attr, argv);
	    break;

	  default:
	    /* both vertical and horizontal share the same attrs */
	    xv_check_bad_attr(SCROLLBAR, attr);
	    argv = attr_skip(attr, argv);
	    break;
	}
    }

    /* Process a change in view_start */
    if (view_start_set) {
	if (!sb->creating) {
	    /* normalize first */
	    if (sb->normalize_proc != NULL)
	      sb->normalize_proc(SCROLLBAR_PUBLIC(sb),
				 view_start, SCROLLBAR_ABSOLUTE, &view_start);
	    if (view_start > Max_offset(sb))
	      view_start = Max_offset(sb);
	    if (scrollbar_scroll_to_offset(sb, (long unsigned) view_start) !=
		SCROLLBAR_POSITION_UNCHANGED)
	      position_elevator = TRUE;
	} else {
	    if (view_start > Max_offset(sb))
	      view_start = Max_offset(sb);
	    sb->view_start = view_start;
	}
    }
    /*
     * Calculate new elevator position, but don't paint unless already
     * painted.
     */
    if (position_elevator && sb->ginfo) {
	  scrollbar_position_elevator(sb, sb->painted, SCROLLBAR_NONE);
    }
    
    if (!sb->managee && sb->can_split) {
	sb->can_split = FALSE;
	xv_error(0,
		 ERROR_STRING,
		     XV_MSG("Cannot split a scrollbar created with scrollbar_create()"),
		 ERROR_PKG, SCROLLBAR,
		 NULL);
    }
}
