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
static char	sccsid[] = "@(#)gcc.c	2.9 91/10/15 Copyright 1989 Sun Microsystems";
#endif

#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<xview/xview.h>
#include	<xview/panel.h>
#include	<xview/cms.h>
#include	<xview/svrimage.h>
#include	<devguide/gcm.h>
#include	<devguide/gcc_ui.h>

void			(*Callback)();
static caddr_t		Client_data;
static char		Current_color[MAXPATHLEN];
static gcc_gccwin_objects	*Gcc_win;
static void		set_color_field();
static void		activate_values();
static void		fill_color_list();

/*
 * Initialize the gcc popup window with a title.
 */
void
#ifdef __STDC__
gcc_initialize(Xv_opaque owner, const char *title)
#else
gcc_initialize(owner, title)
	Xv_opaque	owner;
	const char		*title;
#endif
{
	if(!Gcc_win)
		Gcc_win = gcc_gccwin_objects_initialize(NULL, owner);

	xv_set(Gcc_win->gccwin, XV_LABEL, title, NULL);

	fill_color_list();

	set_color_field(Gcm_colornames[0]);
}

/*
 * Activate the color popup.  Set footer messages, current color, and
 * register a callback function to be called when the apply button is
 * pressed.  Store some client data for the caller.
 */
void
#ifdef __STDC__
gcc_activate(const char *left, const char *right, void (*func)(),
	     caddr_t client_data, char *color)
#else
gcc_activate(left, right, func, client_data, color)
	const char	*left;
	const char	*right;
	void	(*func)();
	caddr_t	client_data;
	char	*color;
#endif
{
int	index;

	xv_set(Gcc_win->gccwin, FRAME_LEFT_FOOTER, left, NULL);
	xv_set(Gcc_win->gccwin, FRAME_RIGHT_FOOTER, right, NULL);
	xv_set(Gcc_win->gccwin, WIN_SHOW, TRUE, WIN_FRONT, NULL);

	activate_values(TRUE);

	if((index = gcm_color_index(color)) == -1)
		index = gcm_color_index("black");

	xv_set(Gcc_win->color_list, PANEL_LIST_SELECT, index, TRUE, NULL);

	strcpy(Current_color, color);
	set_color_field(Current_color);

	Callback = func;
	Client_data = client_data;
}

/*
 * Deactivate the color popup.
 */
void
gcc_deactivate()
{
	Callback = NULL;
	Client_data = NULL;
	xv_set(Gcc_win->gccwin, FRAME_LEFT_FOOTER, "", NULL);
	xv_set(Gcc_win->gccwin, FRAME_RIGHT_FOOTER, "", NULL);

	set_color_field("");
	activate_values(FALSE);
}

/*
 * Suspend or re-enable color chooser if active or was active.
 */
void
#ifdef __STDC__
gcc_suspend(int suspend)
#else
gcc_suspend(suspend)
	int	suspend;
#endif
{
static int	was_active = FALSE;

	if(!Gcc_win)
		return;

	if((suspend && (int)xv_get(Gcc_win->gccwin, WIN_SHOW)) ||
	   (!suspend && was_active))
	{
		xv_set(Gcc_win->gccwin,
			FRAME_CMD_PUSHPIN_IN,	!suspend,
			WIN_SHOW,		!suspend,
			NULL);
		was_active = suspend;
	}
}

/*
 * Notify callback function for `color_list'.
 */
/* ARGSUSED */
int
#ifdef __STDC__
gcc_list_proc(Panel_item item, const char *string, Xv_opaque client_data,
	      Panel_list_op op, Event *event)
#else
gcc_list_proc(item, string, client_data, op, event)
	Panel_item	item;
	const char		*string;
	Xv_opaque	client_data;
	Panel_list_op	op;
	Event		*event;
#endif
{
	switch(op)
	{
	case PANEL_LIST_OP_DESELECT:
		break;

	case PANEL_LIST_OP_SELECT:
		set_color_field(string);
		break;

	case PANEL_LIST_OP_VALIDATE:
		break;

	case PANEL_LIST_OP_DELETE:
		break;
	}
	return XV_OK;
}

/*
 * Notify callback function for `apply'.
 */
/*ARGSUSED*/
void
#ifdef __STDC__
gcc_apply(Panel_item item, Event *event)
#else
gcc_apply(item, event)
	Panel_item	item;
	Event		*event;
#endif
{
	strcpy(Current_color,
		(char *)xv_get(Gcc_win->color_name, PANEL_VALUE));
	set_color_field(Current_color);

	if(Callback)
		(Callback)(Current_color, Client_data);

	xv_set(Gcc_win->apply, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
}

/*
 * Notify callback function for `reset'.
 */
/*ARGSUSED*/
void
#ifdef __STDC__
gcc_reset(Panel_item item, Event *event)
#else
gcc_reset(item, event)
	Panel_item	item;
	Event		*event;
#endif
{
	set_color_field(Current_color);
	xv_set(Gcc_win->reset, PANEL_NOTIFY_STATUS, XV_ERROR, NULL);
}

/*
 * Set a color name into the text field and change the color on the blot.
 */
static void
#ifdef __STDC__
set_color_field(const char *color)
#else
set_color_field(color)
	const char	*color;
#endif
{
int	index;

	if((index = gcm_color_index(color)) == -1)
		index = gcm_color_index("black");

	xv_set(Gcc_win->color_name, PANEL_VALUE, color, NULL);
	xv_set(Gcc_win->color_blot, PANEL_ITEM_COLOR, index, NULL);
}

/*
 * Make panel items active or inactive.
 */
static void
#ifdef __STDC__
activate_values(int state)
#else
activate_values(state)
	int	state;
#endif
{
	xv_set(Gcc_win->apply, PANEL_INACTIVE, !state, NULL);
	xv_set(Gcc_win->reset, PANEL_INACTIVE, !state, NULL);
}

/*
 * Fill the scrolling list with all of the available colors.  Only
 * fill it with the names if color isn't available.
 */
static void
fill_color_list()
{
	Display		*display = (Display *)xv_get(Gcc_win->gccwin, XV_DISPLAY);
	Drawable	xid = (Drawable)xv_get(Gcc_win->gccwin, XV_XID);
	int		depth = (unsigned int)xv_get(Gcc_win->gccwin, XV_DEPTH);
	int		visual_class;
	int		row = 0;
	int		size;
	int		color_available = FALSE;
	int		color_index;
	char		**color;
	unsigned long	*pixel_table;
	GC		gc;
	Pixmap		pixmap;
	Xv_opaque	color_glyph;

	visual_class = (int)xv_get(Gcc_win->gccwin, XV_VISUAL_CLASS);

	if ((depth > 7) &&
	    ((visual_class == StaticColor) || (visual_class == PseudoColor) ||
	     (visual_class == TrueColor) || (visual_class == DirectColor)))
		color_available = TRUE;
	/* MOOSE: SHould we worry about .Xdefaults entries here? Like OpenWindows.3DLook.Color */

	if (color_available)
	{
		gcm_initialize_colors(Gcc_win->controls, NULL, NULL);
		pixel_table = (unsigned long *)xv_get(xv_get(Gcc_win->controls,
							     WIN_CMS),
						      CMS_INDEX_TABLE);
		gc = XCreateGC(display, xid, 0, NULL);
		size = (int)xv_get(Gcc_win->color_list,
				   PANEL_LIST_ROW_HEIGHT) - 2;
	}

	for(color = Gcm_colornames; *color; row++, color++)
	{
		if (color_available)
		{
			pixmap = XCreatePixmap(display, xid, size,
					       size, depth);
			color_index = pixel_table[gcm_color_index(*color)];
			XSetForeground(display, gc, color_index);
			XFillRectangle(display, pixmap, gc, 0, 0, size, size);

			color_glyph =
				xv_create(0, SERVER_IMAGE,
					  SERVER_IMAGE_COLORMAP, gcm_colormap_name(),
					  SERVER_IMAGE_PIXMAP,	pixmap,
						NULL);

			xv_set(Gcc_win->color_list,
			       PANEL_LIST_INSERT, row,
			       PANEL_LIST_GLYPH, row, color_glyph,
			       PANEL_LIST_STRING, row, *color,
			       NULL);
		}
		else
		{
			xv_set(Gcc_win->color_list,
			       PANEL_LIST_INSERT, row,
			       PANEL_LIST_STRING, row, *color,
			       NULL);
		}
	}

	if (color_available)
		XFreeGC(display, gc);
}
