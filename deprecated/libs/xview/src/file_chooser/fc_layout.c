#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)fc_layout.c 1.15 93/06/28";
#endif
#endif
 

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview/xview.h>
#include <xview/font.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview_private/fchsr_impl.h>


static void	fc_calc_xs();
static int	fc_calc_ys();
static int	fc_calc_ys_top_down();
static int	fc_calc_ys_bottom_up();
static int	fc_recalc_ys();
static int	fc_calc_min_width();
static int	fc_calc_min_height();
static int	fc_calc_default_width();
static int	fc_calc_default_height();


/*
 * Substitute for xv_rows/xv_cols, since they may do xv_get
 * on FONT_DEFAULT_CHAR_WIDTH/HEIGHT on each call.
 */
#define COLS(num)	(private->col_width * (num))
#define ROWS(num)	((int)(private->row_height * (num)))




/*
 * Calculate and set X positions and widths
 */
static void
fc_calc_xs( private, exten_rect )
     Fc_private *private;
     Rect *exten_rect;
{
    Rect *tmp;
    int pix_width;
    int x_pos;
    int value_x;
    int item_x;
    int label_right;
    Scrollbar sb;
    int sb_width;


    pix_width = private->rect.r_width; 


    /* initialize X values for extension rect */
    exten_rect->r_width = pix_width;
    exten_rect->r_left = 0;


    /*
     * Goto button is left-aligned on the second column.
     */
    xv_set(private->ui.goto_msg,
	   XV_X,	COLS(2),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);

    xv_set(private->ui.goto_btn, 
	   XV_X,	COLS(2),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);

    
    /*
     * Put Goto Textfield 1 Column after Goto Button.
     * Use all remaining width, shy 2 columns.  Also,
     * even without a label, it puts a bit of space 
     * beteeen the label and value rects; so we have
     * to account for it when calculating width.
     */
    tmp = (Rect *)xv_get(private->ui.goto_btn, XV_RECT);
    x_pos = rect_right(tmp) + COLS(1);
    item_x = (int)xv_get(private->ui.goto_txt, XV_X);
    value_x = (int)xv_get(private->ui.goto_txt, PANEL_VALUE_X);
    xv_set(private->ui.goto_txt, 
	   XV_X,			x_pos,
	   PANEL_VALUE_DISPLAY_WIDTH,	
	   	pix_width - x_pos - (value_x - item_x) - COLS(2),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    

    
    /*
     * Place Folder Textfield.
     */
    x_pos = COLS(4);
    xv_set(private->ui.folder_txt,
	   XV_X,	x_pos,
	   PANEL_VALUE_DISPLAY_WIDTH,
	   	pix_width - x_pos - COLS(2),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    


    
    /*
     * Place select message.  Should line up with File List
     * BUG:  PANEL_MESSAGE seems to ignore the label and item
     * widths...
     */
    xv_set(private->ui.select_msg,
	   XV_X,	COLS(4),
	   PANEL_LABEL_WIDTH,
	   	pix_width 
	   	- (int)xv_get(private->ui.select_msg, PANEL_LABEL_X) 
	   	- COLS(4),
	   PANEL_PAINT,		PANEL_NONE,
	   NULL);
    


    
    /*
     * Place File List centered, 4 cols on either side.  Note
     * that PANEL_LIST_WIDTH does not inlcude scrollbar width.
     */
    x_pos = COLS(4);
    sb = (Scrollbar) xv_get(private->ui.list, PANEL_LIST_SCROLLBAR);
    sb_width = (int) xv_get(sb, XV_WIDTH);
    xv_set(private->ui.list, 
	   XV_X,			x_pos,
	   PANEL_LIST_WIDTH,
	   	pix_width - x_pos - sb_width - COLS(4),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    

    
    /*
     * Place document left-aligned with Goto Button.
     * Note:  there is no doc typein for Open dialog.
     */
    if ( private->ui.document_txt ) {
	item_x = (int)xv_get(private->ui.document_txt, XV_X);
	value_x = (int)xv_get(private->ui.document_txt, PANEL_VALUE_X);
	x_pos = COLS(2);
	xv_set(private->ui.document_txt,
	       XV_X,	x_pos,
	       PANEL_VALUE_DISPLAY_WIDTH,	
	       		pix_width - x_pos - (value_x - item_x) - COLS(2),
	       PANEL_PAINT,	PANEL_NONE,
	       NULL);
    }



    
    
    /*
     * Center the Open, Cancel and Save buttons
     */
    {
	int open_width = (int)xv_get(private->ui.open_btn, XV_WIDTH);
	int cancel_width = (int)xv_get(private->ui.cancel_btn, XV_WIDTH);
	int other_width = 0;
	Panel_item other_btn = XV_NULL;

	if ( private->type != FILE_CHOOSER_OPEN )
	    other_btn = private->ui.save_btn;
	else if ( private->custom )
	    other_btn = private->ui.custom_btn;

	if ( other_btn )
	    other_width = (int)xv_get(other_btn, XV_WIDTH) + COLS(2);

	x_pos = (pix_width 
		 - (open_width + cancel_width + other_width + COLS(2))
		 ) / 2;
	
	xv_set(private->ui.open_btn,
	       XV_X,	x_pos,
	       PANEL_PAINT,	PANEL_NONE,
	       NULL);

	x_pos += open_width + COLS(2);
	xv_set(private->ui.cancel_btn,
	       XV_X,	x_pos,
	       PANEL_PAINT,	PANEL_NONE,
	       NULL);
	
	if ( other_btn ) {
	    x_pos += cancel_width + COLS(2);
	    xv_set( other_btn,
		   XV_X,	x_pos,
		   PANEL_PAINT,	PANEL_NONE,
		   NULL );
	}
    }    
    
} /* fc_calc_xs() */





/*
 * Calcualte Y positions for top half of dialog, down
 * to the File List.
 */
static int
fc_calc_ys_top_down( private )
     Fc_private *private;
{
    int y_pos;

    xv_set(private->ui.goto_msg, 
	   XV_Y,	ROWS(.5),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);

    xv_set(private->ui.goto_btn, 
	   XV_Y,	ROWS(1.5),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);

    xv_set(private->ui.goto_txt, 
	   XV_Y,	ROWS(1.5),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    
    xv_set(private->ui.folder_txt,
	   XV_Y,	ROWS(3.5),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    
    xv_set(private->ui.select_msg,
	   XV_Y,	ROWS(6),
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    
    
    /*
     * Place File List 1 row under Select Msg.  Set the minumum
     * size for now, and use this to calculate the exten area's
     * max_size.
     */
    xv_set(private->ui.list, 
	   XV_Y,			ROWS(7),
	   PANEL_LIST_DISPLAY_ROWS,	3,
	   PANEL_PAINT,			PANEL_NONE,
	   NULL);

    return ROWS(7) + (int) xv_get(private->ui.list, XV_HEIGHT);
} /* fc_calc_ys_top_down() */






/*
 * Calculate Y's from bottom of frame up to the File List.
 *
 * WARNING:  modifications to this code should be reflected in
 * fc_calc_min_height()!
 */
static int
fc_calc_ys_bottom_up( private )
     Fc_private *private;
{
    int y_pos;
    int pix_height;


    pix_height = private->rect.r_height;


    /*
     * Open, Cancel and Save buttons are 2 rows above the
     * footer.
     */
    y_pos = pix_height - (int) xv_get(private->ui.open_btn, XV_HEIGHT) - ROWS(1);
    xv_set(private->ui.open_btn,
	   XV_Y,	y_pos,
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);
    
    xv_set(private->ui.cancel_btn,
	   XV_Y,	y_pos,
	   PANEL_PAINT,	PANEL_NONE,
	   NULL);

    if ( private->type != FILE_CHOOSER_OPEN )
	xv_set( private->ui.save_btn,
	       XV_Y,		y_pos,
	       PANEL_PAINT,	PANEL_NONE,
	       NULL );
    else if ( private->custom )
	xv_set( private->ui.custom_btn,
	       XV_Y,		y_pos,
	       PANEL_PAINT,	PANEL_NONE,
	       NULL );

    y_pos -= ROWS(2);		/* 2 rows white-space above buttons */

    return y_pos;
} /* fc_calc_ys_bottom_up() */





/*
 * Calc Y's presuming exten_func done.  Adjusts File List
 * to fit into new size.
 */
static int
fc_recalc_ys( private, top, exten_rect )
     Fc_private *private;
     int top;
     Rect *exten_rect;
{
    int top_of_bottom;
    int list_row_height;
    int max_exten;
    Rect *list_rect;

    top_of_bottom = fc_calc_ys_bottom_up( private, exten_rect );

    list_row_height = (int) xv_get(private->ui.list, PANEL_LIST_ROW_HEIGHT);


    /*
     * Account for extension area.  
     */
    if ( exten_rect->r_height > 0 )
	top_of_bottom -= ROWS(1.5) + exten_rect->r_height;


    /*
     * Make up for extra row added to Save dialog size.
     */
    if ( private->type != FILE_CHOOSER_OPEN )
	top_of_bottom -= ROWS(1);

    
    /*
     * Update the number of rows displayed in the List according to how
     * much space is left after the rest of the layout. Note that the 3
     * is carried over as the minimum number of rows set in fc_calc_ys_
     * top_down().
     */
    xv_set(private->ui.list,
	   PANEL_LIST_DISPLAY_ROWS,
	   	((top_of_bottom - top) / list_row_height) + 3,
	   PANEL_PAINT, PANEL_NONE,
	   NULL);

    
    /*
     * Do this here to make sure that the document typein is at a
     * static position below the List, and all the residual SLACK is
     * taken up in the space between it and the buttons (and exten
     * area ).
     */
    list_rect = (Rect *) xv_get(private->ui.list, XV_RECT);
    if ( private->ui.document_txt ) {
	Rect *doc_rect;
	
	/* typein is 1/2 row below the list */
	xv_set(private->ui.document_txt,
	       XV_Y,		rect_bottom(list_rect) + ROWS(.5),
	       PANEL_PAINT,	PANEL_NONE,
	       NULL);

	doc_rect = (Rect *) xv_get(private->ui.document_txt, XV_RECT);
	exten_rect->r_top = rect_bottom(doc_rect) + ROWS(1.5);
    } else {
	exten_rect->r_top = rect_bottom(list_rect) + ROWS(1.5);
    }
    
    return top_of_bottom;
} /* fc_recalc_ys() */




/*
 * Do complete Y layout.  Return the maximum size that the
 * Extension Rect may expand to.
 */
static int
fc_calc_ys( private, y_anchor, exten_rect )
     Fc_private *private;
     int *y_anchor;
     Rect *exten_rect;
{
    int max_exten;
    int top_of_bottom;
    
    *y_anchor = fc_calc_ys_top_down( private );
    (void) fc_recalc_ys( private, *y_anchor, exten_rect );


    /*
     * Calculate the maximum the exten rect can expand to and
     * still preserve the layout policy.  Also consider the
     * additional white space perscribed for extension area.
     */
    max_exten 
	= exten_rect->r_top + exten_rect->r_height 
	    - *y_anchor - ROWS(1.5);

    /* area for Save type-in includes 1/2 row white space under list */
    if ( private->ui.document_txt )
	max_exten -= (int) xv_get(private->ui.document_txt, XV_HEIGHT) + ROWS(.5);

    return max_exten;
} /* fc_calc_ys() */






/*
 * Do Relative layout of UI objects.  See Open Look "Application
 * File Choosing Human Interface Specification" for engineering
 * diagrams.
 *
 * Called on WIN_RESIZE iff width or height changed.
 */
Pkg_private void
file_chooser_position_objects( private )
     Fc_private *private;
{
    int max_height;
    int y_anchor;
    Rect exten_rect;

    exten_rect.r_height = private->exten_height;
    fc_calc_xs( private, &exten_rect );
    max_height = fc_calc_ys( private, &y_anchor, &exten_rect );


    /*
     * Make extension callback and recalc y's to meet the
     * returned height.
     */
    if ( private->exten_func ) {
	int right_edge = private->rect.r_width - COLS(2);
	int new_height;

	new_height 
	    = (* private->exten_func)( FC_PUBLIC(private),
				      &private->rect,
				      &exten_rect,
				      COLS(2),
				      private->rect.r_width - COLS(2),
				      max_height
				      );

	/*
	 * if height changed, redo the y's 
	 */
	if ( (new_height != -1) 
	    && (new_height != exten_rect.r_height) 
	    ) {
	    if ( new_height > max_height )
		new_height = max_height;
	    exten_rect.r_height = new_height;
	    fc_recalc_ys( private, y_anchor, &exten_rect );
	}
    }


    /*
     * Finally, repaint the panel.
     */
    panel_paint(private->ui.panel, PANEL_CLEAR);
} /* file_chooser_position_objects() */




/*
 * The value for min width is not specified, but i calcuated
 * it here in terms of allowing each of the standard buttons to
 * remain on the view unobstructed with 1 col left and right.
 */
static int
fc_calc_min_width( private )
     Fc_private *private;
{
    int open_width = (int)xv_get(private->ui.open_btn, XV_WIDTH);
    int cancel_width = (int)xv_get(private->ui.cancel_btn, XV_WIDTH);
    int other_width = 0;
    Panel_item other_btn = XV_NULL;
    int total_width = 0;

    if ( private->type != FILE_CHOOSER_OPEN )
	other_btn = private->ui.save_btn;
    else if ( private->custom )
	other_btn = private->ui.custom_btn;


    total_width = open_width + COLS(2) + cancel_width;

    if ( other_btn )
	total_width += COLS(2) + (int)xv_get(other_btn, XV_WIDTH);

    /* 1 column left and right of buttons */
    return total_width + COLS(2);
} /* fc_calc_min_width() */





/*
 * Basically, min-height is defined as enough vertical space
 * for all the stuff, but with the scrolling list at 3 displayed
 * rows.
 *
 * WARNING:  This code needs to be kept in sync with the rest of
 * the code in this file that manages the vertical layout!!!
 */
static int
fc_calc_min_height( private )
     Fc_private *private;
{
    int total_height = 0;

    /* will set list to 3 rows */
    total_height = fc_calc_ys_top_down( private );

    /* if Save/As dialog, 1/2 row plus typein added */
    if ( private->ui.document_txt ) {
	total_height += ROWS(.5);
	total_height += (int) xv_get(private->ui.document_txt, XV_HEIGHT);
    }
 

    /*
     * Spec shows 2 rows above buttons, and 1 below.
     */
    total_height += ROWS(2);
    total_height += (int) xv_get(private->ui.open_btn, XV_HEIGHT);
    total_height += ROWS(1);

    return total_height;
} /* fc_calc_min_height() */



/*
 * Calculate the minimum size of the frame.
 */
Pkg_private void
file_chooser_calc_min_size( private, width, height )
     Fc_private *private;
     int *	width;
     int *	height;
{
    *width = fc_calc_min_width( private );
    *height = fc_calc_min_height( private );
} /* file_chooser_calc_min_size() */



/*
 * Default width is...  is... is not specified too well...
 * The attempt here is to use the width of the buttons as
 * a key to number of columns to default to.  This should
 * be taken up with the OL people at some point.
 */
static int
fc_calc_default_width( private, min_width )
     Fc_private *private;
     int min_width;
{
    int default_width;
    int help_width;


    /*
     * The default width is achieved somewhat unscientifically
     * by figuring the min width and adding a number of columns
     * that was figured by the Eyeball Method and the Spec.
     */
    if ( private->ui.custom_btn )
	default_width = min_width + COLS(14);
    else if ( private->type == FILE_CHOOSER_OPEN )
	default_width = min_width + COLS(26);
    else
	default_width = min_width + COLS(16);



    /*
     * Make sure help lines fit all the way in.  you'd think
     * they should be truncated at some point, but user's
     * don't seem to appreciate this.
     */

    /* goto help line is aligned with button in col 2 */
    help_width = (int) xv_get(private->ui.goto_msg, XV_WIDTH) + COLS(4);
    if ( help_width > default_width )
	default_width = help_width;

    /* Select help line is aligned with the list, not the button, in col 4 */
    help_width = (int) xv_get(private->ui.select_msg, XV_WIDTH) + COLS(8);
    if ( help_width > default_width )
	default_width = help_width;

    return default_width;
} /* fc_calc_default_width() */



/*
 * Default height is calculated by taking the min height
 * and adding 7 rows to the list.  Note that min height is
 * takes all the vertical sizes and assumes the list is 3 rows
 * tall, so this is the same thing with a list of 10 rows.
 */
static int
fc_calc_default_height( private, min_height )
     Fc_private *private;
     int min_height;
{
    int row_height = (int) xv_get(private->ui.list, PANEL_LIST_ROW_HEIGHT);
    return min_height + (7 * row_height);
} /* fc_calc_default_height() */




/*
 * Calculate the default size of the frame.
 */
Pkg_private void
file_chooser_calc_default_size( private, min_width, min_height, width, height )
     Fc_private *private;
     int	min_width;
     int	min_height;
     int *	width;
     int *	height;
{
    *width = fc_calc_default_width( private, min_width );
    *height = fc_calc_default_height( private, min_height );
} /* file_chooser_calc_default_size() */

/*----------------------------------------------------------------------------*/
