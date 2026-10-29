#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)om_set.c 20.96 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

/* --------------------------------------------------------------------- */
#include <sys/types.h>
#include <xview_private/om_impl.h>
#include <xview_private/draw_impl.h>
#include <xview_private/fm_impl.h>
#include <xview/font.h>
#include <xview/notify.h>
#include <xview/panel.h>

/* -------------------------------------------------------------------- */

/*
 * Public
 */
/* None */

#ifdef OW_I18N
#define XV_CORESET_STR		L"coreset"
#else
#define XV_CORESET_STR		"coreset"
#endif /* OW_I18N */

extern int panel_item_destroy_flag;

/*
 * XView Private
 */
Xv_private void screen_set_cached_window_busy();
Xv_private void menu_set_acc_on_frame();
Xv_private void menu_set_key_qual();
Xv_private void menu_accelerator_notify_proc();
Xv_private int  server_parse_keystr();
char *defaults_get_string();
#ifdef OW_I18N
Xv_private int	xv_wsncasecmp();
#else
Xv_private int	xv_strncasecmp();
#endif /* OW_I18N */

/*
 * Package private
 */
Pkg_private Xv_opaque menu_sets();
Pkg_private Xv_opaque menu_item_sets();
Pkg_private void menu_create_pin_panel_items();
Pkg_private void menu_destroys();
Pkg_private void menu_item_destroys();
Pkg_private void menu_set_pin_window();
Pkg_private Notify_value menu_pin_window_event_proc();
Pkg_private void menu_return_no_value();

/*
 * Private
 */
static int      extend_item_list();
static void     insert_item();
static int      lookup();
static void	menu_add_pin();
static void 	menu_create_title();
static void     remove_item();
static void     replace_item();
static void	destroy_panel_item_handles();

/*
 * Private defs
 */
/* None */

/* -------------------------------------------------------------------- */


Pkg_private     Xv_opaque
menu_sets(menu_public, attrs)
    Menu            menu_public;
    register Attr_attribute *attrs;
{
    register Xv_menu_info *m = MENU_PRIVATE(menu_public);
    int		    bad_attr;
    Xv_menu_info   *parent_menu;
    Xv_menu_item_info *mi;
    int		    repin = FALSE;	/* need to recreate pin window */
    struct image        *std_image;
    int                 status;

    for (; *attrs; attrs = attr_next(attrs)) {
	bad_attr = FALSE;
	switch (attrs[0]) {

	  case MENU_ACTION_IMAGE:
	  case MENU_ACTION_ITEM:
	  case MENU_GEN_PULLRIGHT_IMAGE:
	  case MENU_GEN_PULLRIGHT_ITEM:
	  case MENU_GEN_PROC_IMAGE:
	  case MENU_GEN_PROC_ITEM:
	  case MENU_IMAGE_ITEM:
	  case MENU_PULLRIGHT_IMAGE:
	  case MENU_PULLRIGHT_ITEM:
	  case MENU_STRING_ITEM:
#ifdef OW_I18N
	  case MENU_ACTION_ITEM_WCS:
	  case MENU_GEN_PULLRIGHT_ITEM_WCS:
	  case MENU_GEN_PROC_ITEM_WCS:
	  case MENU_PULLRIGHT_ITEM_WCS:
	  case MENU_STRING_ITEM_WCS:
#endif /* OW_I18N */
	    if (m->nitems < m->max_nitems || extend_item_list(m)) {
		m->item_list[m->nitems++] = MENU_ITEM_PRIVATE(
						   xv_create(XV_ZERO, MENUITEM,
			    MENU_RELEASE, attrs[0], attrs[1], attrs[2], NULL));
	    }
	    break;

          /* ACC_XVIEW */
	  case MENU_ACTION_ACCELERATOR:
#ifdef OW_I18N
	  case MENU_ACTION_ACCELERATOR_WCS:
#endif /* OW_I18N */
            if (m->nitems < m->max_nitems || extend_item_list(m)) {
                m->item_list[m->nitems++] = MENU_ITEM_PRIVATE(
                                                   xv_create(XV_ZERO, MENUITEM,
                            MENU_RELEASE, attrs[0], attrs[1], attrs[2], 
				attrs[3],NULL));
            }
            break;
          /* ACC_XVIEW */

	  case MENU_APPEND_ITEM:{
	    int k;

	    if (m->nitems < m->max_nitems || extend_item_list(m))
		m->item_list[m->nitems++] = MENU_ITEM_PRIVATE(attrs[1]);
            /* ACC_XVIEW */
	    for(k = 0; k < m->nframes; k++)
		menu_set_acc_on_frame(m->frame_list[k], menu_public, attrs[1],
					TRUE);	
            /* ACC_XVIEW */
	    repin = TRUE;
	    break;
	  }

          /* ACC_XVIEW */
 	  case MENU_FRAME_ADD:{
	    Frame	frame = attrs[1];
	    int		i, found = FALSE;

	    /*
	     * Search for frame in frame list
	     */
	    for (i = 0; i < m->nframes; ++i)  {
		if (frame == m->frame_list[i])  {
		    found = TRUE;
		    break;
		}
	    }

	    if (!found)  {
		if (m->nframes + 1 > m->max_frames)  {
		    int		j;
		    Frame	*new_list;

		    /*
		     * Alloc new list - MENU_MAX_FRAMES chunks bigger than
		     * the old one
		     */
		    new_list = (Frame *)xv_calloc(m->max_frames + MENU_MAX_FRAMES,
					sizeof(Frame));
		    
		    if (m->nframes)  {
			for (j = 0; j < m->nframes; ++j)  {
			    new_list[j] = m->frame_list[j];
			}

			xv_free(m->frame_list);
		    }

		    /*
		     * Bump up max, update new list pointer on menu
		     */
		    m->max_frames += MENU_MAX_FRAMES;
		    m->frame_list = new_list;
		}

		m->frame_list[m->nframes] = frame;
	        m->nframes++;
	    }

	    break;
	  }

 	  case MENU_FRAME_DELETE:{
	    Frame	frame = attrs[1];
	    int		i, found = FALSE;

	    /*
	     * Search for frame in frame list
	     */
	    for (i = 0; i < m->nframes; ++i)  {
		if (frame == m->frame_list[i])  {
		    found = TRUE;
		    break;
		}
	    }

	    /*
	     * If found, shift everything by one to fill gap
	     */
	    if (found)  {
		int	j;
		for (j = i + 1; j < m->nframes; ++i, ++j)  {
		    m->frame_list[i] = m->frame_list[j];
		}
	        m->nframes--;
	    }

	    break;
	  }
          /* ACC_XVIEW */

	    	
	  case MENU_BUSY_PROC:
	    m->busy_proc = (void (*) ()) attrs[1];
	    break;

	  case MENU_CLIENT_DATA:
	    m->client_data = (Xv_opaque) attrs[1];
	    break;

	  case MENU_COLOR:
	    if (m->color_index != (int) attrs[1]) {
		m->color_index = (int) attrs[1];
	    }
	    break;

	  case MENU_COL_MAJOR:
	    m->column_major = (int) attrs[1];
	    break;

	  case MENU_DEFAULT:
	    if ((int) attrs[1] > 0)
		m->default_position = (int) attrs[1];
	    else
		m->default_position = 1;
	    break;

	  case MENU_DEFAULT_ITEM:
	    m->default_position = lookup(m->item_list, m->nitems,
					 MENU_ITEM_PRIVATE(attrs[1]));
	    if (m->default_position <= 0)
		m->default_position = 1;
	    break;

	  case MENU_DONE_PROC:
	    m->done_proc = (void (*) ()) attrs[1];
	    break;

	  case XV_FONT:
	    *attrs = ATTR_NOP(*attrs);
            if (m->default_image.font) {
                xv_set(m->default_image.font, XV_DECREMENT_REF_COUNT, NULL);
            }
            m->default_image.font = attrs[1];
            if (m->default_image.font) {
                xv_set(m->default_image.font, XV_INCREMENT_REF_COUNT, NULL);
            }
            m->default_image.width = m->default_image.height = 0;
	    break;

#ifdef  OW_I18N
          case MENU_GEN_PIN_WINDOW:
	    _xv_set_mbs_attr_nodup(&m->pin_window_header, (char *) attrs[2]);
	    goto menu_gen_pin_window;

          case MENU_GEN_PIN_WINDOW_WCS:
	    _xv_set_wcs_attr_nodup(&m->pin_window_header,
				   (wchar_t *) attrs[2]);
menu_gen_pin_window:
            m->pin_parent_frame = (Xv_opaque) attrs[1];
            if (!m->pin)
                menu_add_pin(m);
            break;

#else
	  case MENU_GEN_PIN_WINDOW:
	    m->pin_parent_frame = (Xv_opaque) attrs[1];
	    m->pin_window_header = (char *) attrs[2];
	    if (!m->pin)
                menu_add_pin(m);
	    break;
#endif /* OW_I18N */

	  case MENU_GEN_PROC:
	    m->gen_proc = (Menu(*) ()) attrs[1];
	    break;

	  case MENU_IMAGES:
	    {
		char          **a = (char **) &attrs[1];
		while (*a) {
		    if (m->nitems < m->max_nitems || extend_item_list(m)) {
			m->item_list[m->nitems] = MENU_ITEM_PRIVATE(
						   xv_create(XV_ZERO, MENUITEM,
							     MENU_RELEASE,
						      MENU_IMAGE_ITEM, *a++,
							 m->nitems + 1, NULL));
		    }
		    m->nitems++;
		}
	    }
	    repin = TRUE;
	    break;

	  case MENU_INSERT:{
	    int k;
	    if (++m->nitems < m->max_nitems || extend_item_list(m)) {
		insert_item(m, (int) attrs[1], MENU_ITEM_PRIVATE(attrs[2]));
	    }
            /* ACC_XVIEW */
            for(k = 0; k < m->nframes; k++)
                menu_set_acc_on_frame(m->frame_list[k], menu_public, attrs[2],
					TRUE);
            /* ACC_XVIEW */

	    repin = TRUE;
	    break;
	  }

	  case MENU_INSERT_ITEM:{
	    int k;
	    if (++m->nitems < m->max_nitems || extend_item_list(m)) {
		insert_item(m,
			    lookup(m->item_list, m->nitems,
				   MENU_ITEM_PRIVATE(attrs[1])),
			    MENU_ITEM_PRIVATE(attrs[2]));
	    }
            /* ACC_XVIEW */
            for(k = 0; k < m->nframes; k++)
                menu_set_acc_on_frame(m->frame_list[k], menu_public, attrs[2],
					TRUE);
            /* ACC_XVIEW */

	    repin = TRUE;
	    break;
          }

	  case MENU_ITEM:
	    if (m->nitems < m->max_nitems || extend_item_list(m)) {
		m->item_list[m->nitems] = MENU_ITEM_PRIVATE(xv_create_avlist(
						XV_ZERO, MENUITEM, &attrs[1]));
	    }
	    (void) xv_set(MENU_ITEM_PUBLIC(m->item_list[m->nitems++]), MENU_RELEASE, NULL);
	    repin = TRUE;
	    break;

	  case MENU_NCOLS:
            m->ncols_fixed = m->ncols = imax(0, (int) attrs[1]);
	    break;

	  case MENU_NROWS:
            m->nrows_fixed = m->nrows = imax(0, (int) attrs[1]);
	    break;

	  case MENU_NOTIFY_PROC:
	    m->notify_proc = (Xv_opaque(*) ()) attrs[1];
	    if (!m->notify_proc)
		m->notify_proc = MENU_DEFAULT_NOTIFY_PROC;
	    break;

	  case MENU_NOTIFY_STATUS:
	    /* 
	     * Set the notify status on all menus from the top level menu 
	     * to the pullright, including the pullright
	     */
	    parent_menu = m;
	    while (parent_menu && (mi = parent_menu->parent))  {
	        parent_menu->notify_status = (int) attrs[1];
		parent_menu = mi->parent;	/* get current enclosing menu */
	    }

	    if (parent_menu)  {
	        parent_menu->notify_status = (int) attrs[1];
	    }
	    break;

	  case MENU_PIN:
	    if (attrs[1]) {
		if (!m->pin)
		    menu_add_pin(m);
	    } else if (m->pin) {
		m->pin = FALSE;
#ifdef OW_I18N
		if (_xv_is_string_attr_exist_nodup(
				&m->item_list[0]->image.string) ||
#else
		if (m->item_list[0]->image.string ||
#endif
		    m->item_list[0]->image.svr_im) {
		    /* Force recomputation of item size in compute_item_size */
		    m->item_list[0]->image.width = 0;
		} else {
		    /* No title string or image: Remove pin (title) menu item */
		    remove_item(m, 1);
		}
	    }
	    break;

	  case MENU_PIN_PROC:
	    m->pin_proc = (void (*) ()) attrs[1];
	    break;

	  case MENU_PIN_WINDOW:
	    menu_set_pin_window(m, attrs[1]);
	    break;

	  case MENU_REMOVE:{
             /* ACC_XVIEW */
             /* first remove any existing accelerators */
             int k;
             for(k = 0; k < m->nframes; k++)
                 menu_set_acc_on_frame(m->frame_list[k], menu_public, 
			(Menu_item)xv_get(MENU_PUBLIC(m),MENU_NTH_ITEM, (int)attrs[1]),
                                       FALSE);
             /* ACC_XVIEW */

	    destroy_panel_item_handles(m);
	    remove_item(m, (int) attrs[1]);
	    if (m->pin_window && m->ginfo) {
        	std_image = &m->default_image;
        	std_image->width = 0;
                std_image->image_type = LABEL;
        	compute_item_size( m, std_image, &status, TRUE);
	        menu_create_pin_panel_items(xv_get(m->pin_window, FRAME_CMD_PANEL), m);
	        window_fit(m->pin_window);
	    }
	    break;
	  }

	  case MENU_REMOVE_ITEM:{
            /* ACC_XVIEW */
	    /* first remove any existing accelerators */
	    int	k;
            for(k = 0; k < m->nframes; k++)
                menu_set_acc_on_frame(m->frame_list[k], menu_public, attrs[1],
					FALSE);
            /* ACC_XVIEW */
	    destroy_panel_item_handles(m);
	    remove_item(m,
			lookup(m->item_list, m->nitems,
			       MENU_ITEM_PRIVATE(attrs[1])));
	    if (m->pin_window && m->ginfo) {
        	std_image = &m->default_image;
        	std_image->width = 0;
                std_image->image_type = LABEL;
        	compute_item_size( m, std_image, &status, TRUE);
		menu_create_pin_panel_items(xv_get(m->pin_window, FRAME_CMD_PANEL), m);
		window_fit(m->pin_window);
	    }
	    break;
          }

	  case MENU_REPLACE:
	    destroy_panel_item_handles(m);
	    replace_item(m->item_list, m->nitems, (int) attrs[1],
			 MENU_ITEM_PRIVATE(attrs[2]));
	    if (m->pin_window && m->ginfo) {
        	std_image = &m->default_image;
        	std_image->width = 0;
                std_image->image_type = LABEL;
        	compute_item_size( m, std_image, &status, TRUE);
		menu_create_pin_panel_items(xv_get(m->pin_window, FRAME_CMD_PANEL), m);
		window_fit(m->pin_window);
	    }
	    break;

	  case MENU_REPLACE_ITEM:
	    destroy_panel_item_handles(m);
	    replace_item(m->item_list, m->nitems,
			 lookup(m->item_list, m->nitems,
				MENU_ITEM_PRIVATE(attrs[1])),
			 MENU_ITEM_PRIVATE(attrs[2]));
	    if (m->pin_window && m->ginfo) {
        	std_image = &m->default_image;
        	std_image->width = 0;
                std_image->image_type = LABEL;
        	compute_item_size( m, std_image, &status, TRUE);
		menu_create_pin_panel_items(xv_get(m->pin_window, FRAME_CMD_PANEL), m);
		window_fit(m->pin_window);
	    }
	    break;

#ifdef  OW_I18N
          case MENU_STRINGS_WCS:
          case MENU_STRINGS:
            {
                char          **a = (char **) &attrs[1];
                while (*a) {
                    if (m->nitems < m->max_nitems || extend_item_list(m)) {
                        m->item_list[m->nitems] = MENU_ITEM_PRIVATE(
                                                   xv_create(NULL, MENUITEM,
                                                             MENU_RELEASE,
                                        (attrs[0] == MENU_STRINGS) ?
                                                           MENU_STRING_ITEM :
                                                           MENU_STRING_ITEM_WCS,
						*a++, m->nitems + 1, NULL));
                    }
                    m->nitems++;
                }
            }    
            repin = TRUE;
            break;

          case MENU_STRINGS_AND_ACCELERATORS_WCS:
          case MENU_STRINGS_AND_ACCELERATORS:
            {
                char          **a = (char **) &attrs[1];
                while (*a) {
                    if (m->nitems < m->max_nitems || extend_item_list(m)) {
                        m->item_list[m->nitems] = MENU_ITEM_PRIVATE(
                                                   xv_create(NULL, MENUITEM,
							     XV_INSTANCE_NAME, *a,
                                                             MENU_RELEASE,
                                        (attrs[0] == MENU_STRINGS_AND_ACCELERATORS) ?
                                                           MENU_STRING_AND_ACCELERATOR:
                                                           MENU_STRING_AND_ACCELERATOR_WCS,                                                *a++, *a++, NULL));
                    }
                    m->nitems++; 
                }
            }    
            repin = TRUE;
            break;

#else
	  case MENU_STRINGS:
	    {
		char          **a = (char **) &attrs[1];
		while (*a) {
		    if (m->nitems < m->max_nitems || extend_item_list(m)) {
			m->item_list[m->nitems] = MENU_ITEM_PRIVATE(
						   xv_create(XV_ZERO, MENUITEM,
							     MENU_RELEASE,
							   MENU_STRING_ITEM,
						   *a++, m->nitems + 1, NULL));
		    }
		    m->nitems++;
		}
	    }
	    repin = TRUE;
	    break;

          /* ACC_XVIEW */
          case MENU_STRINGS_AND_ACCELERATORS:
            {
                char          **a = (char **) &attrs[1];
                while (*a) {
                    if (m->nitems < m->max_nitems || extend_item_list(m)) {
                        m->item_list[m->nitems] = MENU_ITEM_PRIVATE(
                                                   xv_create(XV_ZERO, MENUITEM,
                                                XV_INSTANCE_NAME, *a,
                                                MENU_RELEASE,
                                                MENU_STRING_AND_ACCELERATOR,
                                                *a++,  *a++, NULL));
                    }
                    m->nitems++;
                }
            }
            repin = TRUE;
            break;
            /* ACC_XVIEW */
#endif /* OW_I18N */


#ifdef  OW_I18N
          case MENU_TITLE_ITEM:
          case MENU_TITLE_ITEM_WCS:
            if (!m->item_list[0] || !m->item_list[0]->title)
                menu_create_title(m,
                    MENU_TITLE_ITEM == (Menu_attribute) attrs[0] ?
                        MENU_STRING :
                        MENU_TITLE_ITEM_WCS == (Menu_attribute) attrs[0] ?
                            MENU_STRING_WCS : MENU_IMAGE,
                    attrs[1]);
            else
                xv_set(MENU_ITEM_PUBLIC(m->item_list[0]),
                    MENU_TITLE_ITEM == (Menu_attribute) attrs[0] ?
                        MENU_STRING :
                        MENU_TITLE_ITEM_WCS == (Menu_attribute) attrs[0] ?
                            MENU_STRING_WCS : MENU_IMAGE,
                       attrs[1], NULL);
            break;
#else
	  case MENU_TITLE_ITEM:
	    if (!m->item_list[0] || !m->item_list[0]->title)
		menu_create_title(m,
		    MENU_TITLE_ITEM == (Menu_attribute) attrs[0] ?
		        MENU_STRING : MENU_IMAGE,
		    attrs[1]);
	    else
		xv_set(MENU_ITEM_PUBLIC(m->item_list[0]),
		       MENU_TITLE_ITEM == (Menu_attribute) attrs[0] ?
			   MENU_STRING : MENU_IMAGE,
		       attrs[1], NULL);
	    break;
#endif /* OW_I18N */

	  case MENU_VALID_RESULT:
	    m->valid_result = (int) attrs[1];
	    break;

	  case MENU_LINE_AFTER_ITEM:
	    switch ((int) attrs[1]) {
	      case MENU_HORIZONTAL_LINE:
		m->h_line = 1;
		break;
	      case MENU_VERTICAL_LINE:
		m->v_line = 1;
		break;
	      default:{
		    char            dummy[128];

		    (void) sprintf(dummy,
			   XV_MSG("Invalid argument for attribute MENU_LINE_AFTER_ITEM: %d"),
				   (int) attrs[1]);
		    xv_error(XV_ZERO,
			     ERROR_STRING, dummy,
			     ERROR_PKG, MENU,
			     NULL);
		}
	    }
	    break;

	  case MENU_CLASS:
	    xv_error(XV_ZERO,
		 ERROR_STRING, 
		    XV_MSG("MENU_CLASS attribute is get-only"),
		 ERROR_PKG, MENU,
		 NULL);
	    break;
	
	  default:
	    bad_attr = TRUE;
	    break;

	}
	if (!bad_attr)
	    ATTR_CONSUME(attrs[0]);
    }

    if (repin && m->pin_window && m->ginfo) {
	destroy_panel_item_handles(m);
        std_image = &m->default_image;
        std_image->width = 0;
        std_image->image_type = LABEL;
        compute_item_size( m, std_image, &status, TRUE);
	menu_create_pin_panel_items(xv_get(m->pin_window, FRAME_CMD_PANEL), m);
	window_fit(m->pin_window);
    }
    return (Xv_opaque) XV_OK;
}


Pkg_private     Xv_opaque
menu_item_sets(menu_item_public, attrs)
    Menu_item       menu_item_public;
    register Attr_attribute *attrs;
{
    register Xv_menu_item_info *mi = MENU_ITEM_PRIVATE(menu_item_public);
    int		    bad_attr;

    for (; *attrs; attrs = attr_next(attrs)) {
	bad_attr = FALSE;
	switch (attrs[0]) {

	  case MENU_ACTION:	/* == MENU_ACTION_PROC == MENU_NOTIFY_PROC */
	    mi->notify_proc = (Xv_opaque(*) ()) attrs[1];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
		       NULL);
	    }
	    break;

#ifdef  OW_I18N
          case MENU_ACTION_ITEM:
          case MENU_ACTION_ITEM_WCS:
            if (mi->image.free_string)
		_xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_ACTION_ITEM)
	        _xv_set_mbs_attr_nodup(&mi->image.string,
				       (char *) attrs[1]);
	    else
	        _xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            mi->notify_proc = (Xv_opaque(*) ()) attrs[2];
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
		xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
                       NULL);
            }
            break;

          /* ACC_XVIEW */
	  case MENU_ACTION_ACCELERATOR:
	  case MENU_ACTION_ACCELERATOR_WCS:{

            if (mi->image.free_string)
                _xv_free_string_attr_nodup(&mi->image.string);
	    if ((char *)attrs[3])  {
	    	  /*
	           * free current acc string
		   */
                if (_xv_is_string_attr_exist_dup(&mi->menu_acc)){
	            _xv_free_ps_string_attr_dup(&mi->menu_acc);
		}
	    }

	    if (attrs[0] == MENU_ACTION_ACCELERATOR){
	        _xv_set_mbs_attr_nodup(&mi->image.string,
				       (char *) attrs[1]);
	        _xv_set_mbs_attr_dup(&mi->menu_acc,
					(char *) attrs[3]);
	    }
	    else{
	        _xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
	        _xv_set_wcs_attr_dup(&mi->menu_acc,
					(wchar_t *) attrs[3]);
	    }

            mi->image.width = mi->image.height = 0;
	    
            mi->notify_proc = (Xv_opaque(*) ()) attrs[2];
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
	        xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS, mi->image.string.pswcs.value,
                       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
                       NULL);
            }  
            break;
	  }
#else
	  case MENU_ACTION_ITEM:
            if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->notify_proc = (Xv_opaque(*) ()) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
		       NULL);
	    }
	    break;

          /* ACC_XVIEW */
	  case MENU_ACTION_ACCELERATOR:{

            if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    
            mi->notify_proc = (Xv_opaque(*) ()) attrs[2];
	    if ((char *)attrs[3])  {
		/*
		 * free current acc string
		 */
		if (mi->menu_acc)  {
		    xv_free(mi->menu_acc);
		}
	        mi->menu_acc = xv_strsave((char *) attrs[3]);
	    }
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
	        xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING, mi->image.string,
                       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
                       NULL);
            }  
            break;
	  }
          /* ACC_XVIEW */
#endif /* OW_I18N */



#ifdef  OW_I18N
          case MENU_STRING_ITEM:
          case MENU_STRING_ITEM_WCS:
            if (mi->image.free_string)
	        _xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_STRING_ITEM)
		_xv_set_mbs_attr_nodup(&mi->image.string,
				       (char *) attrs[1]);
	    else
		_xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            mi->value = (Xv_opaque) attrs[2];
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       NULL);
            }
            break;

          /* ACC_XVIEW */
          case MENU_STRING_AND_ACCELERATOR:
          case MENU_STRING_AND_ACCELERATOR_WCS:{

            if (mi->image.free_string)
                _xv_free_string_attr_nodup(&mi->image.string);
	    if ((char *)attrs[2])  {
	        /*
		 * free current acc string
		 */
                if (_xv_is_string_attr_exist_dup(&mi->menu_acc)){
	            _xv_free_ps_string_attr_dup(&mi->menu_acc);
	        }
	    }
            if (attrs[0] == MENU_STRING_AND_ACCELERATOR){
              _xv_set_mbs_attr_nodup(&mi->image.string,
                                       (char *) attrs[1]);
	      _xv_set_mbs_attr_dup(&mi->menu_acc,
					(char *) attrs[2]);
	    }
            else {
                _xv_set_wcs_attr_nodup(&mi->image.string,
                                       (wchar_t *) attrs[1]);
	        _xv_set_wcs_attr_dup(&mi->menu_acc,
					(wchar_t *) attrs[2]);
	    }

            mi->image.width = mi->image.height = 0;
            mi->value = (Xv_opaque) attrs[3];
            if (mi->parent && mi->parent->pin_window && 
			mi->panel_item_handle) {                
			_xv_use_pswcs_value_nodup(&mi->image.string);
			xv_set(mi->panel_item_handle,
                       		PANEL_LABEL_STRING_WCS, mi->image.string.pswcs.value,
                       		NULL);
            }
            break;
          }


	  case MENU_ACCELERATOR:
	  case MENU_ACCELERATOR_WCS:{

            /*
	     * free current acc string
	     */
            if (_xv_is_string_attr_exist_dup(&mi->menu_acc)){
	       	_xv_free_ps_string_attr_dup(&mi->menu_acc);
	    }

            if (attrs[0] == MENU_ACCELERATOR){

	    	if ((char *)attrs[1])  {
                	_xv_set_mbs_attr_dup(&mi->menu_acc,
                                       (char *) attrs[1]);
	    	}
	    }
            else{

	    	if ((wchar_t *)attrs[1])  {
			_xv_set_wcs_attr_dup(&mi->menu_acc,
						(wchar_t *) attrs[1]);
	    	}
	    }

	    break;
         }
         /* ACC_XVIEW */
#else
	  case MENU_STRING_ITEM:
            if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->value = (Xv_opaque) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       NULL);
	    }
	    break;

          /* ACC_XVIEW */
          case MENU_STRING_AND_ACCELERATOR:{

            if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
            mi->value = (Xv_opaque) attrs[3];
	    if ((char *)attrs[2])  {
		/*
		 * free current acc string
		 */
		if (mi->menu_acc)  {
		    xv_free(mi->menu_acc);
		}
	        mi->menu_acc = xv_strsave((char *) attrs[2]);
	    }
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING, mi->image.string,
                       NULL);
            }
            break;
          }


	  case MENU_ACCELERATOR:{

	    /*
	     * free current acc string
	     */
	    if (mi->menu_acc)  {
	        xv_free(mi->menu_acc);
	        mi->menu_acc = (char *)NULL;
	    }

	    if ((char *)attrs[1])  {
	        mi->menu_acc = xv_strsave((char *) attrs[1]);
	    }
	    break;
         }

         /* ACC_XVIEW */
 

#endif /* OW_I18N */

	  case MENU_ACTION_IMAGE:
	    if (mi->image.free_svr_im && mi->image.svr_im)
		xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->notify_proc = (Xv_opaque(*) ()) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       XV_KEY_DATA, MENU_NOTIFY_PROC, mi->notify_proc,
		       NULL);
	    }
	    break;

	  case MENU_IMAGE_ITEM:
	    if (mi->image.free_svr_im && mi->image.svr_im)
                xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->value = (Xv_opaque) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       NULL);
	    }
	    break;

	  case MENU_CLIENT_DATA:
	    mi->client_data = (Xv_opaque) attrs[1];
	    break;

	  case MENU_COLOR:
	    mi->color_index = (int) attrs[1];
	    break;

	  case MENU_FEEDBACK:
	    mi->no_feedback = !(int) attrs[1];
	    break;

	  /* ACC_XVIEW */
	  case MENU_ACC_KEY:
#ifdef OW_I18N
            if (mi->key_image.free_string)
		_xv_free_string_attr_nodup(&mi->key_image.string);
	    _xv_set_mbs_attr_nodup(&mi->key_image.string, (char *) attrs[1]);
#else
            if (mi->key_image.free_string && mi->key_image.string)
                xv_free(mi->key_image.string);
	    mi->key_image.string = (char *)attrs[1];
#endif /* OW_I18N */
	    break;

	  case MENU_ACC_QUAL:
#ifdef OW_I18N
            if (mi->qual_image.free_string)
		_xv_free_string_attr_nodup(&mi->qual_image.string);
	    _xv_set_mbs_attr_nodup(&mi->qual_image.string, (char *) attrs[1]);
#else
            if (mi->qual_image.free_string && mi->qual_image.string)
                xv_free(mi->qual_image.string);
	    mi->qual_image.string = (char *)attrs[1];
#endif /* OW_I18N */
	    break;
	  /* ACC_XVIEW */

	  case XV_FONT:
	    *attrs = ATTR_NOP(*attrs);
            if (mi->image.font) {
                (void) xv_set(mi->image.font, XV_DECREMENT_REF_COUNT, NULL);
            }
            mi->image.font = attrs[1];
            if (mi->image.font) {
                (void) xv_set(mi->image.font, XV_INCREMENT_REF_COUNT, NULL);
            }
            mi->image.width = mi->image.height = 0;
	    break;

	  case MENU_GEN_PROC:
	    mi->gen_proc = (Menu_item(*) ()) attrs[1];
	    break;

	  case MENU_GEN_PROC_IMAGE:
	    if (mi->image.free_svr_im && mi->image.svr_im)
                xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->gen_proc = (Menu_item(*) ()) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       NULL);
	    }
	    break;

#ifdef  OW_I18N
          case MENU_GEN_PROC_ITEM:
          case MENU_GEN_PROC_ITEM_WCS:
            if (mi->image.free_string)
		_xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_GEN_PROC_ITEM)
		_xv_set_mbs_attr_nodup(&mi->image.string, (char *) attrs[1]);
	    else
		_xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            mi->gen_proc = (Menu_item(*) ()) attrs[2];
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       NULL);
            }
            break;

#else
	  case MENU_GEN_PROC_ITEM:
	    if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->gen_proc = (Menu_item(*) ()) attrs[2];
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       NULL);
	    }
	    break;
#endif /* OW_I18N */

	  case MENU_GEN_PULLRIGHT:
	    mi->gen_pullright = (Menu(*) ()) attrs[1];
	    mi->pullright = mi->gen_pullright != NULL;
	    mi->value = 0; /* Pullright Generate procedure not called yet */
	    break;

	  case MENU_GEN_PULLRIGHT_IMAGE:
	    if (mi->image.free_svr_im && mi->image.svr_im)
                xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->gen_pullright = (Menu(*) ()) attrs[2];
	    mi->pullright = mi->gen_pullright != NULL;
	    mi->value = 0;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       NULL);
	    }
	    mi->mark_type |= OLGX_HORIZ_MENU_MARK;
	    break;

#ifdef  OW_I18N
          case MENU_GEN_PULLRIGHT_ITEM:
          case MENU_GEN_PULLRIGHT_ITEM_WCS:
            if (mi->image.free_string)
		_xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_GEN_PULLRIGHT_ITEM)
		_xv_set_mbs_attr_nodup(&mi->image.string, (char *) attrs[1]);
	    else
		_xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            mi->gen_pullright = (Menu(*) ()) attrs[2];
            mi->pullright = mi->gen_pullright != NULL;
            mi->value = 0;
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       NULL);
            }
            break;

#else
	  case MENU_GEN_PULLRIGHT_ITEM:
	    if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->gen_pullright = (Menu(*) ()) attrs[2];
	    mi->pullright = mi->gen_pullright != NULL;
	    mi->value = 0;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       NULL);
	    }
	    mi->mark_type |= OLGX_HORIZ_MENU_MARK;
	    break;
#endif /* OW_I18N */

	  case MENU_IMAGE:
	    if (mi->image.free_svr_im && mi->image.svr_im)
                xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       NULL);
	    }
	    break;

	  case MENU_INACTIVE:
	    if (((int) attrs[1] ? TRUE : FALSE) != mi->inactive) {
		mi->inactive = (int) attrs[1];
		if (mi->parent && mi->parent->pin_window &&
		    mi->panel_item_handle) {
		    xv_set(mi->panel_item_handle,
			   PANEL_INACTIVE, mi->inactive,
			   NULL);
		}
	    }
	    break;

	  case MENU_INVERT:
	    mi->image.invert = (int) attrs[1];
	    break;

	  case MENU_PULLRIGHT:
	    mi->value = (Xv_opaque) attrs[1];
	    if (mi->value)
		MENU_PRIVATE(mi->value)->parent = mi;
	    mi->pullright = mi->value != XV_ZERO;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_ITEM_MENU, mi->value,
		       NULL);
	    }
	    mi->mark_type |= OLGX_HORIZ_MENU_MARK;
	    break;

	  case MENU_PULLRIGHT_IMAGE:
	    if (mi->image.free_svr_im && mi->image.svr_im)
                xv_destroy(mi->image.svr_im);
            mi->image.svr_im = (Server_image) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->value = (Xv_opaque) attrs[2];
	    mi->pullright = mi->value != XV_ZERO;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_IMAGE, mi->image.svr_im,
		       PANEL_ITEM_MENU, mi->value,
		       NULL);
	    }
	    mi->mark_type |= OLGX_HORIZ_MENU_MARK;
	    break;

#ifdef  OW_I18N
          case MENU_PULLRIGHT_ITEM:
          case MENU_PULLRIGHT_ITEM_WCS:
            if (mi->image.free_string)
		_xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_PULLRIGHT_ITEM)
		_xv_set_mbs_attr_nodup(&mi->image.string, (char *) attrs[1]);
	    else
		_xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            mi->value = (Xv_opaque) attrs[2];
            mi->pullright = mi->value != NULL;
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       PANEL_ITEM_MENU,		mi->value,
                       NULL);
            }
            break;

#else
	  case MENU_PULLRIGHT_ITEM:
	    if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    mi->value = (Xv_opaque) attrs[2];
	    mi->pullright = mi->value != XV_ZERO;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       PANEL_ITEM_MENU, mi->value,
		       NULL);
	    }
	    mi->mark_type |= OLGX_HORIZ_MENU_MARK;
	    break;
#endif /* OW_I18N */

	  case MENU_RELEASE:
	    mi->free_item = TRUE;
	    break;

	  case MENU_RELEASE_IMAGE:
	    mi->image.free_string = TRUE;
	    mi->image.free_svr_im = TRUE;
	    break;

	  case MENU_SELECTED:
	    mi->selected = (int) attrs[1];
	    break;

#ifdef  OW_I18N
          case MENU_STRING:
          case MENU_STRING_WCS:
            if (mi->image.free_string)
		_xv_free_string_attr_nodup(&mi->image.string);
	    if (attrs[0] == MENU_STRING)
		_xv_set_mbs_attr_nodup(&mi->image.string, (char *) attrs[1]);
	    else
		_xv_set_wcs_attr_nodup(&mi->image.string,
				       (wchar_t *) attrs[1]);
            mi->image.width = mi->image.height = 0;
            if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		_xv_use_pswcs_value_nodup(&mi->image.string);
                xv_set(mi->panel_item_handle,
                       PANEL_LABEL_STRING_WCS,	mi->image.string.pswcs.value,
                       NULL);
            }
            break;
          
#else
	  case MENU_STRING:
	    if (mi->image.free_string && mi->image.string)
                free(mi->image.string);
            mi->image.string = (char *) attrs[1];
            mi->image.width = mi->image.height = 0;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_LABEL_STRING, mi->image.string,
		       NULL);
	    }
	    break;
#endif /* OW_I18N */

	  case MENU_TITLE:
	    mi->title = TRUE;
	    mi->image.title = TRUE;
	    break;

	  case MENU_VALUE:
	    mi->value = (Xv_opaque) attrs[1];
	    mi->pullright = FALSE;
	    if (mi->parent && mi->parent->pin_window && mi->panel_item_handle) {
		xv_set(mi->panel_item_handle,
		       PANEL_ITEM_MENU, NULL,
		       NULL);
	    }
	    break;

	  case MENU_LINE_AFTER_ITEM:
	    switch ((int) attrs[1]) {
	      case MENU_HORIZONTAL_LINE:
		mi->h_line = 1;
		break;
	      case MENU_VERTICAL_LINE:
		mi->v_line = 1;
		break;
	      default:{
		    char            dummy[128];

		    (void) sprintf(dummy,
			   XV_MSG("Invalid argument for attribute MENU_LINE_AFTER_ITEM: %d"),
				   (int) attrs[1]);
		    xv_error(XV_ZERO,
			     ERROR_STRING, dummy,
			     ERROR_PKG, MENU,
			     NULL);
		}
	    }
	    break;

	  default:
	    bad_attr = TRUE;
	    break;

	}
	if (!bad_attr)
	    ATTR_CONSUME(attrs[0]);
    }

    return (Xv_opaque) XV_OK;
}


Pkg_private void
menu_destroys(m, destroy_proc)
    register Xv_menu_info *m;
    void            (*destroy_proc) ();
{
    register Xv_menu_item_info *mi;
    Xv_Drawable_info *info;

    if (!m || m->type != (int) MENU_MENU)
	return;
    if (m->item_list) {
	for (; m->nitems-- > 0;) {
	    mi = m->item_list[m->nitems];
	    if (mi->pullright && !mi->gen_pullright && mi->value) {
#ifdef NOTIFIER_WARNING_FIXED
		xv_destroy(mi->value);
#else
		/*** BUG ALERT!  The following line will leak memory
		 * because the Generic object's data structures are
		 * not freed.  However, there's a possible conflict
		 * with the submenu already being destroyed by the
		 * application.  Implementing the xv_destroy alternative
		 * on the previous line causes 
		 *     XView warning: Notifier error: Unknown client
		 * to be printed out when mailtool is quit.
		 */

		/*
                menu_destroys( MENU_PRIVATE( mi->value ), destroy_proc);
		*/

		/*
		 * 11/5/92
		 * The above line fixes one problem but it makes another
		 * problem visible. The menu is destroyed twice because of 
		 * the line above, and on the second time, the application 
		 * core dumps. Until a way is found to prevent the menu from
		 * being destroyed twice, the above line should remain 
		 * commented.
		 */
#endif
	    }
	    xv_destroy(MENU_ITEM_PUBLIC(mi));
	}
	free(m->item_list);
    }

    /*
     * Free MENU_FRAMES list
     */
    if (m->frame_list)  {
	xv_free((char *)m->frame_list);
	m->frame_list = NULL;
	m->nframes = 0;
    }
    if (m->window) {
	DRAWABLE_INFO_MACRO(m->window, info);
	screen_set_cached_window_busy(xv_screen(info),
				      m->window, FALSE);
    }
    if (m->shadow_window) {
	DRAWABLE_INFO_MACRO(m->shadow_window, info);
	screen_set_cached_window_busy(xv_screen(info),
				      m->shadow_window, FALSE);
    }
    if (destroy_proc)
	destroy_proc(m, MENU_MENU);
    free(m);
}


Pkg_private void
menu_item_destroys(mi, destroy_proc)
    register Xv_menu_item_info *mi;
    void            (*destroy_proc) ();
{
    if (!mi || !mi->free_item)
	return;
    if (mi->image.free_image) {
#ifdef OW_I18N
	if (mi->image.free_string)
	    _xv_free_string_attr_nodup(&mi->image.string);
#else
	if (mi->image.free_string && mi->image.string)
	    free(mi->image.string);
#endif
	if (mi->image.free_svr_im && mi->image.svr_im)
	    xv_destroy(mi->image.svr_im);
    }
    /*
     * free current acc string
     */
#ifdef OW_I18N
    if (_xv_is_string_attr_exist_dup(&mi->menu_acc)){
          _xv_free_ps_string_attr_dup(&mi->menu_acc);
    }
#else
    if(mi->menu_acc) {
	xv_free(mi->menu_acc);
    }
#endif /* OW_I18N */
    if (destroy_proc)
	destroy_proc(MENU_ITEM_PUBLIC(mi), MENU_ITEM);
#ifdef OW_I18N
    _xv_free_ps_string_attr_nodup(&mi->image.string);
#endif /* OW_I18N */
    free((char *) mi);
}


static void
menu_add_pin(m)
    Xv_menu_info *m;
{
    m->pin = TRUE;

    /* Add pushpin-out image to menu title */
    if (!m->item_list[0] || !m->item_list[0]->title)
        menu_create_title(m, 0, (Xv_opaque) 0);

    /* Force recomputation of item size in compute_item_size */
    m->item_list[0]->image.width = 0;
}


Pkg_private void
menu_set_pin_window(m, pin_window)
    Xv_menu_info   *m;
    Xv_opaque	    pin_window;
{
    m->pin_window = pin_window;
    if (m->pin_window) {
	xv_set(m->pin_window, XV_KEY_DATA, MENU_MENU, m, NULL);
        /* fix to make xv_window_loop work for menus */
        if (WIN_IS_IN_LOOP)
            window_set_tree_flag(m->pin_window, NULL, FALSE, TRUE);
        else
            window_set_tree_flag(m->pin_window, NULL, FALSE, FALSE);
	notify_interpose_event_func(m->pin_window,
	    menu_pin_window_event_proc, 
            WIN_IS_IN_LOOP ? NOTIFY_IMMEDIATE : NOTIFY_SAFE);
    }
}


static int
extend_item_list(m)
    register Xv_menu_info *m;
{
    m->max_nitems = m->max_nitems + MENU_FILLER;
    m->item_list = (Xv_menu_item_info **) xv_realloc(
						  (char *) m->item_list,
		       (u_int) (m->max_nitems * sizeof(Xv_menu_item_info)));
    if (!m->item_list) {
	xv_error((Xv_opaque)m,
		 ERROR_LAYER, ERROR_SYSTEM,
		 ERROR_STRING,
		     XV_MSG("menu_set: Malloc failed to allocate an item list"),
		 ERROR_PKG, MENU,
		 NULL);
	m->max_nitems = m->max_nitems - MENU_FILLER;
	return FALSE;
    }
    return TRUE;
}


static void
remove_item(m, pos)
    Xv_menu_info *m;
    int pos;	/* position: 1= first item */
{
    register Xv_menu_item_info **il = m->item_list;
    register int    i;

    if (pos < 1 || pos > m->nitems)
	return;	/* invalid position */
    if (pos == 1 && il[0]->title && m->pin)
	m->pin = FALSE;
    for (i = pos; i < m->nitems; i++)
	il[i - 1] = il[i];
    --m->nitems;
    if (!m->ncols_fixed)
	m->ncols = 0;
    if (!m->nrows_fixed)
	m->nrows = 0;
    return;
}


static void
replace_item(il, len, pos, mi)
    Xv_menu_item_info *il[];	/* item list ptr */
    int len;	/* nbr of menu items */
    int pos;	/* position: 1= first item */
    Xv_menu_item_info *mi;
{
    if (pos < 1 || pos > len)
	return; /* invalid position */
    il[pos - 1] = mi;
    return;
}


static void
insert_item(m, pos, mi)
    Xv_menu_info *m;
    int pos;	/* position: 1= first item */
    Xv_menu_item_info *mi;
{
    register Xv_menu_item_info **il = m->item_list;
    register int    i;

    if (pos < 0 || pos >= m->nitems) {
	--m->nitems;
	return; /* invalid position */
    }
    for (i = m->nitems - 1; i > pos; --i)
	il[i] = il[i - 1];
    il[i] = mi;
    if (!m->ncols_fixed)
	m->ncols = 0;
    if (!m->nrows_fixed)
	m->nrows = 0;
    return;
}


static int
lookup(il, len, mi)
    register Xv_menu_item_info *il[];
    Xv_menu_item_info *mi;
{
    int             i;

    for (i = 0; i < len; i++)
	if (il[i] == mi)
	    return i + 1;
    return -1;
}


static void
menu_create_title(m, type, arg1)
    register Xv_menu_info *m;
    int             type;	/* MENU_STRING, MENU_IMAGE or 0 (= no title) */
    Xv_opaque       arg1;	/* the string or pixrect */
{
    register int    i;
    Menu_item       menu_item;

    if (m->nitems < m->max_nitems || extend_item_list(m)) {
	m->nitems++;
	for (i = m->nitems - 1; i > 0; i--)
	    m->item_list[i] = m->item_list[i - 1];
	menu_item = xv_create(XV_ZERO, MENUITEM,
			      MENU_FEEDBACK, FALSE,
			      MENU_RELEASE,
			      MENU_TITLE,
			      MENU_NOTIFY_PROC, menu_return_no_value,
			      NULL);
	m->item_list[0] = MENU_ITEM_PRIVATE(menu_item);
	if (type)
	    xv_set(menu_item,
		   type, arg1,
		   MENU_LINE_AFTER_ITEM, MENU_HORIZONTAL_LINE,
		   NULL);
	if (m->default_position == 1)
	    m->default_position++;
    }
}

static void
destroy_panel_item_handles(m)
    register Xv_menu_info *m;
{
    int	panel_item_destroyed = FALSE;	/* for Choice and Toggle menus */
    int	i;

    if (panel_item_destroy_flag == 1)
	panel_item_destroy_flag = 2;
    for (i=0; i < m->nitems; i++) {
        if (m->item_list[i]->panel_item_handle) {
	    if (m->class == MENU_COMMAND) {
		xv_set(m->item_list[i]->panel_item_handle,
		       PANEL_ITEM_MENU, NULL,
		       NULL);
		xv_destroy(m->item_list[i]->panel_item_handle);
	    } else if (!panel_item_destroyed) {
		xv_destroy(m->item_list[i]->panel_item_handle);
		panel_item_destroyed = TRUE;
	    }
	    m->item_list[i]->panel_item_handle = XV_ZERO;
	    }
    }
}

/* ACC_XVIEW */
Xv_private void
menu_set_acc_on_frame(frame,menu,item, set)
	Frame   	frame;
	Menu		menu;
	Menu_item	item;
	int		set;
{
	
        Frame_accel_data        *accelerator_info;
	CHAR			*acc_string = (CHAR *)NULL;
	extern char		*xv_instance_app_name;

        KeySym      keysym;
        short       code;
        unsigned int        meta_modmask;
        unsigned int        modifiers = 0;
        int         	result, parse_result;
	Xv_server	server_public;
	char		qual_str[50];
	char		key_str[20];
	char		*key_alloc;
        Xv_menu_item_info *mi;

	mi = MENU_ITEM_PRIVATE(item);
	if (xv_instance_app_name)  {
	    char	*tmp;

	    tmp = (char *)xv_get(item, XV_INSTANCE_NAME);

	    if (tmp)  {
	        char			*item_instance_name;
	        char			*acc_resource_name;

		item_instance_name = xv_strsave(tmp);

		/*
		 * resource name :
		 * <app name>'.'<menu item instance name>'.'"accelerator"<NULL char>
		 */
		acc_resource_name = xv_malloc(strlen(xv_instance_app_name) +
					1 +
					strlen(item_instance_name) +
					1 +
					strlen("accelerator") +
					1);
		sprintf(acc_resource_name, "%s.%s.accelerator",
				xv_instance_app_name, item_instance_name);
		
		tmp = defaults_get_string(acc_resource_name, acc_resource_name, 
						NULL);

		/*
		 * At this point, we can free alloc'd strings
		 */
		xv_free(item_instance_name);
		xv_free(acc_resource_name);

		if (tmp)  {
#ifdef OW_I18N
    		    if (_xv_is_string_attr_exist_dup(&(MENU_ITEM_PRIVATE(item)->menu_acc))){
          		_xv_free_ps_string_attr_dup(
			&(MENU_ITEM_PRIVATE(item)->menu_acc));
		    }
                    _xv_set_mbs_attr_dup(&(MENU_ITEM_PRIVATE(item)->menu_acc),
                                                tmp);
#else
	            if (MENU_ITEM_PRIVATE(item)->menu_acc)  {
			xv_free(MENU_ITEM_PRIVATE(item)->menu_acc);
		    }
		    MENU_ITEM_PRIVATE(item)->menu_acc = xv_strsave(tmp);
#endif /* OW_I18N */
		}
	    }
	}

#ifdef OW_I18N
	acc_string = _xv_get_wcs_attr_dup(&(MENU_ITEM_PRIVATE(item)->menu_acc));
#else
	acc_string = MENU_ITEM_PRIVATE(item)->menu_acc;
#endif /* OW_I18N */


	if (!acc_string)  {
	    /*
	     * This menu item has no accelerator associated with it
	     * Set the key and qualifier strings to NULL
	     */
            menu_set_key_qual(menu, item, FALSE, (KeySym)NULL, 
					(unsigned int)NULL, (unsigned int)NULL, 
					(char *)NULL);
	    return;
	}
	
	if (set)  {
	    /* 
	     * Create data struct to set on FRAME_MENU_ACCELERATOR 
	     * BUG: Where/when do we free this??
	     */
	    accelerator_info = xv_alloc(Frame_accel_data);
	    accelerator_info->menu = menu;
	    accelerator_info->item = item;

            server_public = XV_SERVER_FROM_WINDOW(frame);
            meta_modmask = (unsigned int)xv_get(server_public,
                            SERVER_META_MOD_MASK);

            parse_result = server_parse_keystr(server_public, 
			        acc_string,
                            &keysym, &code, &modifiers, meta_modmask, qual_str);

	    if (parse_result == XV_OK)  {
	        result = xv_set(frame, 
#ifdef OW_I18N
			    FRAME_MENU_ACCELERATOR_WCS,  acc_string,
#else
			    FRAME_MENU_ACCELERATOR,  acc_string,
#endif /* OW_I18N */
			    menu_accelerator_notify_proc,
			    accelerator_info,
			    NULL);

	        /*
	         * Accelerator successfully registered
	         */
	        if (result == XV_OK)  {
                    menu_set_key_qual(menu, item, TRUE, keysym, 
				modifiers, meta_modmask, qual_str);
	        }
	        else  {
                    Frame_menu_accelerator	*menu_accelerator;
                    Frame_accel_data		*dup_acc = 
						(Frame_accel_data *)NULL;
		    int				reset_labels = TRUE;

		    /*
		     * Accelerator failed to be registered with frame
		     * Before printing out error message, check if:
		     */
                    menu_accelerator = (Frame_menu_accelerator *) xv_get(frame,
                                        FRAME_MENU_X_ACCELERATOR, 
					code, modifiers, keysym);

		    if (menu_accelerator)  {
			dup_acc = (Frame_accel_data *)menu_accelerator->data;
		    }

		    if (dup_acc)  {
			Menu_item	dup_item = dup_acc->item;
                        Xv_menu_item_info	*dup_mi, *mi;

			dup_mi = MENU_ITEM_PRIVATE(dup_item);
			mi = MENU_ITEM_PRIVATE(item);

			/*
			 * Check to determine if resetting the labels
			 * and error message is not necesssary
			 */
			if ( (dup_item == item) ||
			     (MENU_ITEM_PRIVATE(dup_item)->notify_proc && 
			     (MENU_ITEM_PRIVATE(dup_item)->notify_proc ==
				MENU_ITEM_PRIVATE(item)->notify_proc)) ||
			     (MENU_ITEM_PRIVATE(dup_item)->gen_proc &&
			     (MENU_ITEM_PRIVATE(dup_item)->gen_proc ==
				MENU_ITEM_PRIVATE(item)->gen_proc)) )  {
			    reset_labels = FALSE;
			}
		    }

		    /*
		     * Set key/qualifier label to NULL
		     * Unset diamond mark flag
		     */
		    if (reset_labels)  {
		        char	i18n_str[80];

                        menu_set_key_qual(menu, item, FALSE, keysym, 
				modifiers, meta_modmask, qual_str);

#ifdef OW_I18N
		        sprintf(i18n_str, "%s %ws\n",
                            XV_MSG("Duplicate menu accelerator specified:"),
		            acc_string ? acc_string : L"NULL");
#else
		        sprintf(i18n_str, "%s %s\n",
                            XV_MSG("Duplicate menu accelerator specified:"),
		            acc_string ? acc_string : "NULL");
#endif /* OW_I18N */

                        xv_error(XV_ZERO, ERROR_STRING, i18n_str, NULL);
		    }
		    else  {
                        menu_set_key_qual(menu, item, TRUE, keysym, 
				modifiers, meta_modmask, qual_str);
		    }
	        }
	    }
	    else  {
		/*
		 * Parsing function returned != XV_OK
		 * Before printing out error message, check first if a coreset 
		 * accelerator binding was requested. If yes, (strncasecmp 
		 * returns 0) don't print out error message.
		 */
#ifdef OW_I18N
		if(xv_wsncasecmp
#else
		if(xv_strncasecmp
#endif /* OW_I18N */
			(acc_string, XV_CORESET_STR, STRLEN(XV_CORESET_STR)))  {
		    char	i18n_str[80];

#ifdef OW_I18N
		    sprintf(i18n_str, "%s %ws\n",
                        XV_MSG("Menu accelerator string has incorrect format:"),
		        acc_string ? acc_string : L"NULL");
#else
		    sprintf(i18n_str, "%s %s\n",
                        XV_MSG("Menu accelerator string has incorrect format:"),
		        acc_string ? acc_string : "NULL");
#endif /* OW_I18N */

                    xv_error(XV_ZERO, ERROR_STRING, i18n_str, NULL);
		}

		/*
		 * Set key/qualifier label to NULL
		 * Unset diamond mark flag
		 */
                menu_set_key_qual(menu, item, FALSE, (KeySym)NULL, 
					(unsigned int)NULL, (unsigned int)NULL, 
					(char *)NULL);
	    }
	}
	else  {
	    xv_set(frame, 
#ifdef OW_I18N
		FRAME_MENU_REMOVE_ACCELERATOR_WCS,  acc_string, 
#else
		FRAME_MENU_REMOVE_ACCELERATOR,  acc_string, 
#endif /* OW_I18N */
		NULL);

	    /*
	     * Set key/qualifier label to NULL
	     * Unset diamond mark flag
	     */
            menu_set_key_qual(menu, item, FALSE, (KeySym)NULL, (unsigned int)NULL, 
					(unsigned int)NULL, (char *)NULL);
	}
}
/* ACC_XVIEW */

/* ACC_XVIEW */
Xv_private void
menu_set_key_qual(menu, item, set, keysym, modifiers, 
				diamond_modmask, qual_str)
	Menu		menu;
	Menu_item	item;
	int		set;
	KeySym		keysym;
	unsigned int	modifiers;
	unsigned int	diamond_modmask;
        char		*qual_str;
{
    char		key_str[20];
    Xv_menu_item_info	*mi;

    if (set)  {
        /*
         * For ascii characters, print the actual character, 
	 * not the word for it. For example, for '.', we print 
	 * '.', not 'period'.
         */

        if (isascii((int)keysym))  {
            /*
             * If alpha character, we need to check if it is 
	     * lower/upper case. We always display alpha key 
	     * labels in upper case.
             */
            if (isalpha((int)keysym))  {
                sprintf(key_str, "%c", islower((int)keysym) ? 
                toupper((char)(keysym)) : (char)(keysym));
            }
            else  {
                /*
                 * Not alpha character - print as is
                 */
                sprintf(key_str, "%c", (char)(keysym));
            }
        }
        else  {
            char	*ksymstr;

            /*
             * Not ascii, we depend on XKeysymToString() to give us
             * a string to print
             */
            ksymstr = (char *)XKeysymToString(keysym);

            if (ksymstr)  {
                sprintf(key_str, "%s", ksymstr);
            }
            else  {
                key_str[0] = '\0';
            }
        }

        /*
         * We still need to alloc space for MENU_ACC_KEY and MENU_ACC_QUAL
         * because they don't copy the strings. This is to do what
         * MENU_STRING does.
         * We need to add more stuff to tell the menu pkg to free these 
         * strings at destroy time.
         */
        if (key_str && strlen(key_str))  {
            char	*key_alloc = xv_strsave(key_str);

            xv_set(item, MENU_ACC_KEY, key_alloc, NULL);
        }
        else  {
            xv_set(item, MENU_ACC_KEY, NULL, NULL);
        }
    
        if(modifiers & diamond_modmask){
            MENU_ITEM_PRIVATE(item)->mark_type  |= OLGX_DIAMOND_MARK;
        }

        /*
        * Check if we have any qualifier string returned
        * If an empty string is returned, this means that
        * there are no qualifiers for this menu item. We 
        * pass in NULL for MENU_ACC_QUAL for this case.
        */
        if (qual_str && strlen(qual_str))  {
            char	*qual_alloc = xv_strsave(qual_str);
    
            /*
             * We still need to alloc space for MENU_ACC_KEY and MENU_ACC_QUAL
             * because they don't copy the strings. This is to do what
             * MENU_STRING does.
             * We need to add more stuff to tell the menu pkg to free these 
             * strings at destroy time.
             */
            xv_set(item, MENU_ACC_QUAL, qual_alloc, NULL);
        }
        else  {
            xv_set(item, MENU_ACC_QUAL, NULL, NULL);
        }
    }
    else  {
	    /*
	     * Set key/qualifier label to NULL
	     * Unset diamond mark flag
	     */
            xv_set(item, MENU_ACC_KEY, NULL, NULL);
            xv_set(item, MENU_ACC_QUAL, NULL, NULL);
            MENU_ITEM_PRIVATE(item)->mark_type  &= ~OLGX_DIAMOND_MARK;
    }
}
/* ACC_XVIEW */
