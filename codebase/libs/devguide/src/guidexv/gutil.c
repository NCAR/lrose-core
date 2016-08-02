/*
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 * 
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef lint
static char	sccsid[] = "@(#)gutil.c	2.6 91/10/15 Copyright 1989 Sun Microsystems";
#endif


#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<xview/xview.h>
#include	<xview/panel.h>


/*
 * Returns TRUE if the given xview event is a double click
 * event.
 * The time and drag thresholds are initialized to the values
 * specified in the .Xdefaults file.
 */
int
#ifdef __STDC__
doubleclick(Event *event)
#else
doubleclick(event)
	Event	*event;
#endif
{
	int			ret_value = FALSE;
	struct	timeval		current_select_time;

	static	short		first_time = TRUE;
	static	int		time_threshold;
	static	int		drag_threshold;
	static	struct timeval 	last_select_time;
	static 	Rect		last_area;

	if (event_action(event) != ACTION_SELECT || !event_is_down(event))
		return FALSE;

	/*
	 * Init the thresholds the first time this is called.
	 */
	if( first_time )
	{
		time_threshold =
			defaults_get_integer("openwindows.multiclicktimeout",
					     "OpenWindows.MultiClickTimeout",
					     4);

		/*
		 * Convert for timeval.tv_usec.
		 */
		time_threshold *= 100000;
				
		drag_threshold = 
			defaults_get_integer("openwindows.dragthreshold",
					     "OpenWindows.DragThreshold", 5);

		first_time	= FALSE;
	}
	
	current_select_time = event_time(event);

	/* 
	 * Make sure we have not moved too far in between clicks.
	 */
	if (rect_includespoint(&last_area, event_x(event), event_y(event)))
	{
		if (current_select_time.tv_sec == last_select_time.tv_sec)
		{
			/* Check if micro seconds are in range */
			if ((current_select_time.tv_usec -
			     last_select_time.tv_usec) < time_threshold)
				ret_value = TRUE;
		}
		else if (current_select_time.tv_sec ==
			 (last_select_time.tv_sec + 1))
		{
			/* Check for rollover condition. */
			if ((1000000 - last_select_time.tv_usec +
			     current_select_time.tv_usec) < time_threshold)
				ret_value = TRUE;
			
		}
	}
	last_select_time = current_select_time;
	last_area.r_left = event_x(event) - drag_threshold;
	last_area.r_top  = event_y(event) - drag_threshold;
	last_area.r_width = last_area.r_height = 2 * drag_threshold;

	return ret_value;
}
