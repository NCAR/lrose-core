#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)tty_cb.c %I 93/06/28";
#endif
#endif

/*
 *      (c) Copyright 1990 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#ifdef OW_I18N

#include <sys/types.h>
#include <pixrect/pixrect.h>

#include <xview/tty.h>
#include <xview/ttysw.h>
#include <xview/termsw.h>
#include <xview_private/tty_impl.h>
#include <xview_private/term_impl.h>
#include <xview_private/i18n_impl.h>
#include <xview_private/charimage.h>
#include <xview_private/charscreen.h>

#define ITERM_BUFSIZE   1024

#define	TTYSW_GET_COL(folio)		(curscol)
#define	TTYSW_GET_ROW(folio)		(cursrow)
#define	TTYSW_GET_MAX_COL(folio)	(ttysw_right)
#define	TTYSW_GET_MAX_ROW(folio)	(ttysw_bottom)

/*   
 *    committed_left takes care about the case of implicit commit.
 *   Preedit-callback suspend drawing preedit text until all committed 
 *   string is drawn.
 */
extern	int	committed_left;

Xv_private      void
tty_text_start(ic, client_data, callback_data)
XIC		ic;
XPointer	client_data;
XPointer 	callback_data;
{
    Tty		ttysw_public;
    Ttysw_folio	folio;

    ttysw_public = (Tty)client_data;
    folio = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);


    folio->im_first_col = TTYSW_GET_COL(folio);
    folio->im_first_row = TTYSW_GET_ROW(folio);
    folio->im_len = 0;

    if( !folio->im_store )
    	folio->im_store = (CHAR *)malloc( ITERM_BUFSIZE * sizeof(CHAR) );
    if( !folio->im_attr )
    	folio->im_attr  = (XIMFeedback *)malloc( ITERM_BUFSIZE * sizeof( XIMFeedback) );

    folio->im_store[0] = (CHAR)'\0';

    /*  
     *  preedit_state is used to check pre-editting is on or off.
     */

    folio->preedit_state = TRUE;

    return;
}

Xv_private      void
tty_text_draw(ic, client_data, callback_data)
XIC	ic;
XPointer	client_data;
XIMPreeditDrawCallbackStruct 	*callback_data;
{
    Tty			ttysw_public;
    CHAR		*wcs;
    char		*mbs;
    XIMFeedback		*attr;
    int			len = 0;
    Ttysw_folio		folio;
    static Bool		curs_set = TRUE;
    int			org_len, chg_start, chg_end, chg_length, new_len;
    CHAR		*org_text;
    CHAR		*new_text;
    CHAR		*insert_text;
    XIMFeedback		*org_attr;
    XIMFeedback		*new_attr;
    XIMFeedback		*insert_attr;
    int                 delete_only = 0;
    CHAR		buf2_text[ITERM_BUFSIZE];
    XIMFeedback		buf2_attr[ITERM_BUFSIZE];
    CHAR		*wcs_buf = (CHAR *)NULL;

    ttysw_public = (Tty)client_data;
    folio = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);

    if( !folio->preedit_state ) {
	tty_text_start(ic, client_data, callback_data);
    }

    if( !curs_set ) {
    	folio->im_first_col = TTYSW_GET_COL(folio);
    	folio->im_first_row = TTYSW_GET_ROW(folio);
	curs_set = TRUE;
    }

    org_text = folio->im_store;
    org_attr = folio->im_attr;
    org_len = wslen(org_text);

    chg_start = callback_data->chg_first;
    chg_length = callback_data->chg_length;

    /*
     * Nothing to do if trying to erase an empty preedit. This may happen
     * in mplicit commit in term mode switch. Implicit commit erases preedit
     * for IC reset (it's a hack), and later when switch back, preedit draw
     * will be called to erase the preedit which has been erased.
     */
    if ( callback_data->text == NULL && org_len == 0 )
	return;

    if( callback_data->text == NULL && org_len == chg_length ) {
                                                        /* erase preedit */
        ttysw_removeCursor();
        tty_preedit_erase(folio, folio->im_first_col, folio->im_first_row,
                                folio->im_len );
        ttysw_drawCursor(folio->im_first_row, folio->im_first_col);
        if ( chg_length != 0 ) {
            folio->im_store[0] = (CHAR)'\0';
	    folio->im_len = 0;
	}

        return;
    }

    if ( callback_data->text != NULL ) {
        len = callback_data->text->length;
        wcs = (CHAR *)callback_data->text->string.wide_char;
        mbs = (char *)callback_data->text->string.multi_byte;
        attr = (XIMFeedback *)callback_data->text->feedback;

	if( !callback_data->text->encoding_is_wchar && len != 0 ) {
	    wcs_buf = (CHAR *)calloc(len + 1, sizeof(CHAR));
	    mbstowcs(wcs_buf, mbs, len + 1);
	}
    } else {
        delete_only = 1;
        len = 0;
        wcs = (CHAR *)NULL;
        mbs = (char *)NULL;
        attr = (XIMFeedback *)NULL;
    }

    if( committed_left > 0  && wcs ) {
	if( !callback_data->text->encoding_is_wchar ) {
	    wscpy(org_text, wcs_buf);
            free(wcs_buf);
        } else
	    wscpy( org_text , wcs );

	if ( attr )
	    XV_BCOPY(attr, org_attr, sizeof(XIMFeedback)*org_len);
	else
	    XV_BZERO(org_attr, sizeof(XIMFeedback)*org_len);

	curs_set = FALSE;
	return;
    }

    /*
     * Erase any selection before drawing preedit.
     */
    if ( org_len == 0 ) {
	if (folio->ttysw_primary.sel_made) {
	    ttysel_deselect(&folio->ttysw_primary, SELN_PRIMARY);
	}
	if (folio->ttysw_secondary.sel_made) {
	    ttysel_deselect(&folio->ttysw_secondary, SELN_SECONDARY);
	}
    }

    if( ( !wcs ) && ( !mbs ) && (!delete_only) ) { /* updtate the feedback */
	insert_attr = org_attr + chg_start;
	if ( attr )
	    XV_BCOPY(attr, insert_attr, sizeof(XIMFeedback)*chg_length);
	else
	    XV_BZERO(insert_attr, sizeof(XIMFeedback)*chg_length);
    }
    else { 				/* draw intermediate text */
	new_len = org_len - chg_length + len;
	chg_end = chg_start + chg_length;

	/*
	 * just delete the tail of the preedit.
	 */
	if ( delete_only && ( chg_end == org_len) ) {
	    goto draw_new_preedit;
	}

        /*
	 * 1) attach the new text to the original preedit,
	 * 2) replace the entire preedit string,
	 * 3) replace part of preedit (with same length)
	 * 4) replace the tail of preedit with the new text
	 */
        if ( ( chg_start == org_len
		|| chg_length == org_len
		|| chg_length == len
		|| chg_end == org_len)
				&& len != 0 && new_len <= ITERM_BUFSIZE ) {
            insert_text = org_text + chg_start;
            insert_attr = org_attr + chg_start;

            if( !callback_data->text->encoding_is_wchar ) {
		XV_BCOPY(wcs_buf, insert_text, sizeof(CHAR)*wslen(wcs_buf));
            } else
                XV_BCOPY(wcs, insert_text, sizeof(CHAR)*len );

	    if ( attr )
        	XV_BCOPY(attr, insert_attr, sizeof(XIMFeedback)*len );
	    else
		XV_BZERO(insert_attr, sizeof(XIMFeedback)*len);

            goto draw_new_preedit;
        }
 
	/*
	 * find out the starting point for replacement
	 */
	if ( chg_length < len ) {		/* need a buffer */
	    if ( new_len > ITERM_BUFSIZE ) {
		new_text = (CHAR *)calloc(new_len + 1, sizeof(CHAR));
		new_attr = (XIMFeedback *)calloc(new_len + 1,
						sizeof(XIMFeedback));
	    } else {
		new_text = buf2_text;
		new_attr = buf2_attr;
	    }

	    if ( chg_start > 0 ) {
		XV_BCOPY(org_text, new_text, sizeof(CHAR)*chg_start);
		XV_BCOPY(org_attr, new_attr, sizeof(XIMFeedback)*chg_start);
	    }

	    insert_text = new_text + chg_start;
	    insert_attr = new_attr + chg_start;
	} else {				/* use the org_text */
	    insert_text = org_text + chg_start;
	    insert_attr = org_attr + chg_start;
	}
 
	/*
	 * copy the incoming text for the replacement
	 */
	if ( len != 0 ) {
	    if( !callback_data->text->encoding_is_wchar ) {
		wscpy(insert_text, wcs_buf);
	    } else
		XV_BCOPY(wcs, insert_text, sizeof(CHAR)*len );

	    if ( attr )
		XV_BCOPY(attr, insert_attr, sizeof(XIMFeedback)*len );
	    else
		XV_BZERO(insert_attr, sizeof(XIMFeedback)*len);
	}

	if ( chg_end < org_len ) {
		insert_text += len;
		insert_attr += len;
		XV_BCOPY(org_text + chg_end,
			insert_text,
			sizeof(CHAR)*(org_len - chg_end) );
		XV_BCOPY(org_attr + chg_end,
			insert_attr,
			sizeof(XIMFeedback)*(org_len - chg_end) );
	}

	if ( chg_length < len ) {
	    if ( new_len <= ITERM_BUFSIZE ) {
		XV_BCOPY(new_text, folio->im_store, sizeof(CHAR)*new_len);
		XV_BCOPY(new_attr, folio->im_attr, sizeof(XIMFeedback)*new_len);
	    } else {
		free(folio->im_store);
		free(folio->im_attr);

		folio->im_store = new_text;
		folio->im_attr = new_attr;
	    }
	}

draw_new_preedit :

	if ( wcs_buf )
	    free(wcs_buf);

	folio->im_store[new_len] = (CHAR)'\0';
    }

	new_text = folio->im_store;
	new_attr = folio->im_attr;

	if ( chg_start != 0 ) {
		int	max_col;
		int	d_len, imlen = 0;
		int	n_col = 0, d_col = 0, d_row = 0;
		CHAR	* wcs;
		void	tty_preedit_scr_cal();

		max_col = TTYSW_GET_MAX_COL( folio );
		d_col = folio->im_first_col;
		wcs = new_text;

		tty_preedit_scr_cal(wcs, chg_start, max_col,
                                                &d_col, &d_row, &n_col);

		if ( (d_len = folio->im_len - n_col) > 0 )
			tty_preedit_erase(folio, 
				d_col, folio->im_first_row + d_row, d_len);

		tty_preedit_put_wcs(folio,
			new_text + chg_start, new_attr + chg_start,
			d_col,folio->im_first_row + d_row , &imlen);

		folio->im_len = n_col + imlen;
	} else
		tty_preedit_replace_wcs(folio, new_text, new_attr,
			folio->im_first_col,folio->im_first_row );

    return;			
}

void
tty_preedit_scr_cal(wcs, len, max_col, f_col, f_row, n_col)
CHAR            *wcs;
int             max_col, len;
int             *f_col, *f_row, *n_col;
{
    int     i, col_wchar, end_gap;

    *n_col = 0;
    *f_row = 0;

    for ( i=0; i < len; i++) {
        col_wchar = tty_character_size(*wcs ++);
        *n_col += col_wchar;
        if ( *f_col + col_wchar < max_col ) {
	    *f_col += col_wchar;
	}else {
	    if ( *f_col + col_wchar == max_col ) {
		*f_col = 0;
	    } else {
		end_gap = max_col - *f_col;
		*n_col += end_gap;
		*f_col = col_wchar;
	    }
            *f_row += 1;
        }
    }
}

Xv_private      void
tty_text_done(ic, client_data, callback_data)
XIC		ic;
XPointer	client_data;
XPointer 	callback_data;
{
    Tty		ttysw_public;
    Ttysw_folio	folio;

    ttysw_public = (Tty)client_data;
    folio = TTY_PRIVATE_FROM_ANY_PUBLIC(ttysw_public);

    folio->preedit_state = FALSE;

    /*
     * Erase any preedit
     */  
    if ( folio->im_len != 0 ) {
        ttysw_removeCursor();
        tty_preedit_erase(folio, folio->im_first_col,
                                folio->im_first_row, folio->im_len);
        ttysw_drawCursor(folio->im_first_row, folio->im_first_col);
    }

    folio->im_len = 0;

    curscol = folio->im_first_col;
    cursrow = folio->im_first_row;
    XV_BZERO( folio->im_store, ITERM_BUFSIZE * sizeof( CHAR ));

    return;
}


tty_preedit_erase( folio , first_col , first_row , len )
Ttysw_folio	folio;
int		first_col;
int		first_row;
int		len;
{
    int		fromcol,tocol,row,num_row;
    int 	maxcol = TTYSW_GET_MAX_COL( folio );

    num_row = (first_col + len + 1)/maxcol + 1;

    fromcol = first_col;
    if( num_row == 1 )
	tocol = first_col + len;
    else
	tocol = maxcol;

    for( row = first_row ; row < first_row + num_row ; row++ ) {
	ttysw_pclearline( fromcol , tocol , row );
	len -= (tocol - fromcol - 1);
	fromcol = 0;
	if ( len > maxcol )
		tocol = maxcol;
	else
		tocol = len - 1;
    }
}
    
tty_preedit_replace_wcs( folio, wcs, attr, first_col, first_row )
Ttysw_folio     folio;  
CHAR		*wcs;
XIMFeedback	*attr;
int		first_col;
int		first_row;
{
    tty_preedit_erase( folio ,folio->im_first_col ,folio->im_first_row,
				folio->im_len );
    tty_preedit_put_wcs( folio, wcs , attr, first_col , first_row , &(folio->im_len));
}

tty_preedit_put_wcs( folio, wcs , attr, first_col ,first_row, len)
Ttysw_folio     folio;  
CHAR		*wcs;
XIMFeedback	*attr;
int		first_col;
int		first_row;
int		* len;
{
    int		col,row,maxcol,maxrow;
    int		i,j;
    char	mode;
    static CHAR	buf[4] = { (CHAR)'\0',(CHAR)'\0',
				   (CHAR)'\0',(CHAR)'\0'};
    int		colwidth;
    int		end_gap;

    maxcol = TTYSW_GET_MAX_COL( folio );
    maxrow = TTYSW_GET_MAX_ROW( folio );
    col = first_col;
    row = first_row;

    * len = 0;
    ttysw_removeCursor();

    while( *wcs ) {
    	mode = MODE_CLEAR;
	buf[0] = *wcs++;
	colwidth = tty_character_size( buf[0] );
	/*
	 *	BUG!!
	 *	This code is restricted by the length of buf(--4--)
	 *	, which means you cannot display a character that is bigger
	 *	than 3 times the size of ascii characters
	 */
	if( col+colwidth > maxcol ) {
		end_gap = colwidth - (maxcol-col);
		if( end_gap > 0 )
			* len += end_gap;
		col = 0;
		if( row >= maxrow-1 ) {
			ttysw_cim_scroll(1);
			folio->im_first_row --;
			cursrow--;
		} else {
			row++;
		}
	}
	if( *attr & XIMReverse )
		mode |= MODE_INVERT;
	if( *attr & XIMUnderline )
		mode |= MODE_UNDERSCORE;
	j = 1;
	while( j < colwidth )
		buf[j++] = TTY_NON_WCHAR;
    	ttysw_pstring(buf, mode, col, row, PIX_SRC);
	j = 1;
	while( j < colwidth )
		buf[j++] = (CHAR)'\0';
	col += colwidth;
	if ( col == maxcol ) {
		col = 0;
		if( row >= maxrow-1 ) {
			ttysw_cim_scroll(1);
			folio->im_first_row --;
			cursrow--;
		} else {
			row++;
		}
	}
	* len += colwidth;
        attr++;
    }

    ttysw_drawCursor(row, col);

}


ttysw_preedit_resize_proc( folio )
Ttysw_folio	folio;
{
/*  
 *      This function takes care about the preedit text, when ttysw is
 *    resized when conversion is on. This function is dependent on
 *    the pre-edit callback code. And this function is never invoked
 *    if our pre-edit callback is overridden by another callback.  
 */
    if( folio->im_first_col >= TTYSW_GET_MAX_COL(folio) ) {
	folio->im_first_col = 0;
	folio->im_first_row += 1;
	if( folio->im_first_row >= TTYSW_GET_MAX_ROW(folio) )
		ttysw_cim_scroll(1);
    }

    if( folio->im_first_row >= TTYSW_GET_MAX_ROW(folio) )
	folio->im_first_row = TTYSW_GET_MAX_ROW(folio) - 1;


}


/*
 * This is the end of the ifdef in the very beginning of this file.
 */
#endif
