#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)p_imcb.c 50.40 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N
#include <xview_private/panel_impl.h>

Xv_private void
panel_text_start (ic, client_data, callback_data)
    XIC		ic;
    XPointer	client_data;
    XPointer	callback_data;
{
    Panel	panel_public;
    Panel_info	*panel;

    /* Get the panel handle from xim */
    panel_public = (Panel)client_data;
    panel = PANEL_PRIVATE(panel_public);

    /* Ignore all request if there's nothing with keyboard
     * focus, and if the item with the focus is 
     * not panel text item
     */

    if (!panel->kbd_focus_item)
	    return;

    if (panel->kbd_focus_item->item_type != PANEL_TEXT_ITEM)
	    return;

    /*  Save the handle of the item with focus
     *  for implicit commit purpose
     */
    panel->preedit_item = panel->kbd_focus_item;

    /* store the current_caret_offset */
    ml_panel_saved_caret(panel->kbd_focus_item);
}


Xv_private void
panel_text_draw(ic, client_data, callback_data)
    XIC			  ic;
    XPointer		  client_data;
    XPointer		  callback_data;

{
    Panel		  panel_public;
    Panel_info 		  *panel;
    XIMPreeditDrawCallbackStruct  
			  *preedit_changes;
    
    /* Get the panel handle from xim */
    panel_public = (Panel)client_data;
    panel = PANEL_PRIVATE(panel_public);

    
    /* Ignore all request if there's nothing with
     * keyboard focus, and if the item with the focus is
     * not a panel text item
     */

    if (!panel->kbd_focus_item)
	    return;

    if (panel->kbd_focus_item->item_type != PANEL_TEXT_ITEM)
	    return;

    /* focus may have changed from when panel_text_start
     * was called, so need to reassign preedit_item handle
     */
    panel->preedit_item = panel->kbd_focus_item;

    /* Get the preedit text from xim */
    preedit_changes = (XIMPreeditDrawCallbackStruct *)callback_data;

    /* Display the preedit text */
    panel_preedit_display(panel->preedit_item, preedit_changes, FALSE);


}

Xv_private void
panel_text_done(ic, client_data, callback_data)
    XIC		ic;
    XPointer	client_data;
    XPointer	callback_data;

{
    Panel	panel_public;
    Panel_info	*panel;
    
    /* Get the panel handle from xim */
    panel_public = (Panel)client_data;
    panel = PANEL_PRIVATE(panel_public);

    /* Ignore all request if there's no item with keyboard
     * focus
     */

    if (!panel->kbd_focus_item)
	    return;

    /* Ignore all request if this is not a panel
     * text item.
     */

    if (panel->kbd_focus_item->item_type != PANEL_TEXT_ITEM)
	    return; 

    /* save the caret offset, 
     * clear the preedit text cache, clear
     * the text field item of preedit text,
     * then zero out the preedit item handle 
     */
    ml_panel_saved_caret(panel->preedit_item);

    panel->preedit->text->string.wide_char[0] = NULL;
    panel->preedit->text->length = NULL;
    ml_panel_display_interm(panel->preedit_item);
    panel->preedit_item = 0;

}

Xv_private void
panel_preedit_display(ip, preedit_changes, full)
    Item_info			 *ip;
    XIMPreeditDrawCallbackStruct *preedit_changes;
    Bool			  full;
{
    int			  	  length;

    /*  IMLogic gives me only partial update information
     *  for preedit text, therefore I have to piece
     *  together the full preedit text by
     *  calling the function 'cache_text_state'
     *  HAVE TO OPTIMIZE LATER:  panel should not
     *  use cache_text_state, should do my own
     *  partial update to speed things up.
     */

    if (full == TRUE) {
	Text_info	*dp;

	dp = TEXT_FROM_ITEM(ip);
	dp->first_char = 0;
	dp->last_char = -1;
	if (dp->value_wc != NULL)
		dp->value_wc[0] = 0;
	ip->panel->preedit = preedit_changes;
    } else
        cache_text_state(preedit_changes, ip->panel->preedit);

    /* FIX ME: The feedback array is not set correctly */
    if (preedit_changes->text) {
	if (!preedit_changes->text->feedback)
	    XV_BZERO(ip->panel->preedit->text->feedback, 
	    sizeof(XIMFeedback)*ip->panel->preedit->text->length);
    }

    ml_panel_display_interm(ip);
}


Xv_private XIMPreeditDrawCallbackStruct *
panel_get_preedit(ip)
     Item_info		*ip;
{
    /*
     * Whenever this routine called, that means preedit data structure
     * is not owned by the panel anymore.  In this case, just to be
     * sure, panel will not free the preedit data strucure.
     */
    ip->panel->preedit_own_by_others = TRUE;
    return ip->panel->preedit;
}


Xv_private void
panel_set_preedit(ip, preedit)
     Item_info				*ip;
     XIMPreeditDrawCallbackStruct	*preedit;
{
     ip->panel->preedit = preedit;
}


/* Not used right now.

Pkg_private void
panel_text_caret(ic, direction, udata)
    XIC			ic;
    IMTextDirection	direction;
    caddr_t		udata;
{
	interprete direction and calculate row and col;
	set cursor position
}
*/

#endif /* OW_I18N */
