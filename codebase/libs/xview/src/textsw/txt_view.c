#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)txt_view.c 1.32 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/defaults.h>
#include <xview_private/primal.h>
#include <xview_private/txt_impl.h>
#include <xview/textsw.h>
#include <xview/win_struct.h>
#include <xview/window.h>
#include <xview/text.h>

#define SET_NEW_START(_view, _char_pos) _view->e_view->line_table.seq[0]= (_char_pos == TEXTSW_INFINITY) ? 0 : _char_pos

Pkg_private Textsw_view_handle textsw_view_init_internal();
Pkg_private Notify_value textsw_event();

Pkg_private int textsw_view_init();
Pkg_private Xv_opaque textsw_view_set();
Pkg_private Xv_opaque textsw_view_get();
Pkg_private int textsw_view_destroy();



Pkg_private int
textsw_view_init(parent, textsw_view_public, avlist)
    Textsw          parent;
    Textsw_view     textsw_view_public;
    Attr_attribute  avlist[];
{
    Attr_avlist     attrs;
    Textsw_view_handle view = NEW(struct textsw_view_object);
    Textsw_status   dummy_status;
    Textsw_status  *status = &dummy_status;
    Xv_textsw_view *view_object = (Xv_textsw_view *) textsw_view_public;

    if (!text_notice_key)  {
	text_notice_key = xv_unique_key();
    }

    for (attrs = avlist; *attrs; attrs = attr_next(attrs)) {
	switch (*attrs) {
	  case TEXTSW_STATUS:
	    status = (Textsw_status *) attrs[1];
	    break;
	  default:
	    break;
	}
    }

    if (!view) {
	*status = TEXTSW_STATUS_CANNOT_ALLOCATE;
	return (XV_ERROR);
    }
    /* link to object */
    view_object->private_data = (Xv_opaque) view;
    view->public_self = textsw_view_public;
    view->magic = TEXTSW_VIEW_MAGIC;
/* Alpha compatibility, mbuck@debian.org */
#if defined(__alpha)
    view->window_fd = (Xv_Window) xv_get(textsw_view_public, WIN_FD);
#else
    view->window_fd = (int) xv_get(textsw_view_public, WIN_FD);
#endif
    /*
     * Must initialize rect here else code elsewhere (e.g., textsw_resize)
     * that tries to compute incremental changes gets the wrong answer.
     */
    (void) win_getsize(WIN_FD_FOR_VIEW(view), &view->rect);

    view->folio = FOLIO_ABS_TO_REP(parent);

    view = textsw_view_init_internal(view, status);


    if (!view) {
	return (XV_ERROR);
    }
    xv_set(textsw_view_public,
	   WIN_NOTIFY_SAFE_EVENT_PROC, textsw_event,
	   WIN_NOTIFY_IMMEDIATE_EVENT_PROC, textsw_event,
	   XV_HELP_DATA, "textsw:textsw",
	   NULL);

    view->drop_site = xv_create(VIEW_PUBLIC(view), DROP_SITE_ITEM,
				DROP_SITE_REGION, 	&view->rect,
				NULL);

    /* Grab the keys that can be used to start a ``quick'' selection.  We
     * need to grab them in order for quick selection to work in follow-
     * mouse mode.
     */
    win_grab_quick_sel_keys(VIEW_PUBLIC(view));

    return (XV_OK);
}



Pkg_private	void
textsw_split_init_proc(public_view, public_new_view, position)
    Textsw_view     public_view, public_new_view;
    int             position;
{
    Textsw_view_handle view = VIEW_PRIVATE(public_view);
    Textsw_view_handle new_view = VIEW_PRIVATE(public_new_view);
    Textsw_folio    folio = FOLIO_FOR_VIEW(view);
    int             line_pos, char_pos;

    line_pos = ev_line_for_y(view->e_view, position);
    if (line_pos == view->e_view->line_table.last_plus_one)
	line_pos--;
    char_pos = ev_index_for_line(view->e_view, line_pos);
    SET_NEW_START(new_view, char_pos);
    if (folio->notify_level & TEXTSW_NOTIFY_SPLIT_VIEW)
	textsw_notify(view, TEXTSW_ACTION_SPLIT_VIEW, public_new_view, NULL);


}
