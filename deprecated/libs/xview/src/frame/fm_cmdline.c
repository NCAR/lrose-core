#if defined(sccs) && !defined(lint)
static char     sccsid[] = "@(#)fm_cmdline.c 20.46 93/06/28";
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <sys/types.h>
#include <ctype.h>
#include <pixrect/pixrect.h>
#include <xview/font.h>
#include <xview/cms.h>
#include <xview/rect.h>
#include <xview/screen.h>
#include <xview/server.h>
#include <xview/frame.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview/icon.h>
#include <xview/defaults.h>
#include <xview/win_screen.h>
#include <xview_private/draw_impl.h>
#include <xview_private/fm_impl.h>
#include <xview_private/i18n_impl.h>
#include <X11/Xutil.h>

extern Pixrect *icon_load_mpr();
static int frame_parse_color();
Xv_private char *xv_font_regular();
Xv_private char *xv_font_bold();
Xv_private char *xv_font_monospace();
Xv_private char *xv_font_scale();

/*
 * Convert command line frame defaults into attributes. Apply the attributes
 * to the frame with xv_set_avlist().
 */
Pkg_private int
frame_set_cmdline_options(frame_public, from_init)
    Frame           frame_public;
    Bool  	    from_init;
{
    Frame_class_info *frame = FRAME_PRIVATE(frame_public);
    Xv_opaque         defaults_array[ATTR_STANDARD_SIZE];
    Attr_avlist       defaults = defaults_array;
    int               status = XV_OK;
    char             *string;
    char             *defaults_string[1];

    /* No string malloc's done yet */
    defaults_string[0] = NULL;

    if (frame_notify_count == 1) {

        /* Parse frame command line options */
	if (defaults_exists("window.header", "Window.Header")) {
	    *defaults++ = (Xv_opaque) FRAME_LABEL;
	    string = defaults_get_string("window.header", "Window.Header", "");
	    defaults_string[0] = xv_malloc(strlen(string) + 1);
	    strcpy(defaults_string[0], string);
	    *defaults++ = (Xv_opaque) defaults_string[0];
        }

	if (defaults_exists("window.iconic", "Window.Iconic")) {
	    int iconic = defaults_get_boolean("window.iconic", "Window.Iconic",
					      FALSE);	
	    status_set(frame, iconic, iconic);
	    status_set(frame, initial_state, iconic);
	    if (iconic) {
		frame->wmhints.initial_state = IconicState;
		frame->wmhints.flags |= StateHint;
	    }
	    *defaults++ = (Xv_opaque) FRAME_CLOSED;
	    *defaults++ = (Xv_opaque) iconic;
        }

	/* Set the size if the user asked for it */
	if (defaults_exists("window.width", "Window.Width")) {
	    frame->user_rect.r_width = defaults_get_integer("window.width",
							     "Window.Width", 1);
	    frame->geometry_flags |= WidthValue;
	}
	if (defaults_exists("window.height", "Window.Height")) {
	    frame->user_rect.r_height = defaults_get_integer("window.height",
							    "Window.Height", 1);
	    frame->geometry_flags |= HeightValue;
	}

	/* Set the position if the user asked for it */
	if (defaults_exists("window.x", "Window.X")) {
	    frame->user_rect.r_left = defaults_get_integer("window.x",
								 "Window.X", 0);
	    frame->geometry_flags |= XValue;
	}
	if (defaults_exists("window.y", "Window.Y")) {
	    frame->user_rect.r_top = defaults_get_integer("window.y",
								 "Window.Y", 0);
	    frame->geometry_flags |= YValue;
	}
	if (defaults_exists("window.geometry", "Window.Geometry")) {
	    int x, y, flags;
	    unsigned int width, height;
	    char *geometry;
	    
	    geometry = defaults_get_string("window.geometry", "Window.Geometry",
					   (char *) NULL);

	    flags = XParseGeometry(geometry, &x, &y, &width, &height);
	    if (flags & WidthValue)
	        frame->user_rect.r_width = width;
	    if (flags & HeightValue)
	        frame->user_rect.r_height = height;
	    if (flags & XValue)
	        frame->user_rect.r_left = x;
	    if (flags & YValue)
	        frame->user_rect.r_top = y;
	    frame->geometry_flags |= flags;
	}

	if (frame->geometry_flags & WidthValue) {
	    *defaults++ = (Xv_opaque) XV_WIDTH;
	    *defaults++ = (Xv_opaque) frame->user_rect.r_width;
	}
	if (frame->geometry_flags & HeightValue) {
	    *defaults++ = (Xv_opaque) XV_HEIGHT;
	    *defaults++ = (Xv_opaque) frame->user_rect.r_height;
	}
	if (frame->geometry_flags & (XValue | YValue)) {
	    int x = frame->user_rect.r_left;
	    int y = frame->user_rect.r_top;
	    int width, height;
	    int screen;
	    Xv_Drawable_info *info;
	    
	    DRAWABLE_INFO_MACRO(frame_public, info);
	    screen = (int) xv_get(xv_screen(info), SCREEN_NUMBER, NULL);
	    
	    if (frame->geometry_flags & XNegative) {
		width = (frame->geometry_flags & WidthValue) ? 
		    frame->user_rect.r_width : (int) xv_get(frame_public, XV_WIDTH, NULL);
		x += DisplayWidth(xv_display(info), screen) - width - (2 * FRAME_BORDER_WIDTH);
	    }
	    if (frame->geometry_flags & YNegative) {
		height = (frame->geometry_flags & HeightValue) ?
		    frame->user_rect.r_height : (int) xv_get(frame_public, XV_HEIGHT, NULL);
		y += DisplayHeight(xv_display(info), screen) - height - (2 * FRAME_BORDER_WIDTH);
	    }
	    *defaults++ = (Xv_opaque) XV_X;
	    *defaults++ = (Xv_opaque) x;
	    *defaults++ = (Xv_opaque) XV_Y;
	    *defaults++ = (Xv_opaque) y;
	}

	/* send a WM_COMMAND for the first frame */
	if (!from_init)
	    win_set_wm_command(frame_public);

    }

    /* NULL terminate the defaults list */
    *defaults = 0;
    
    /* Do a frame set if there are frame attrs */
    if (!from_init)
	if (defaults_array[0])
	    status = (int) xv_set_avlist(frame_public, defaults_array);
	else
	    status = XV_OK;

    /* Free any malloc'ed strings */
    if (defaults_string[0])
	free(defaults_string[0]);

    return (status);
}


/*
 * Set command line options/defaults that are relevant to ALL frames
 */
Pkg_private int
frame_all_set_cmdline_options(frame_public)
    Frame           frame_public;
{
    Frame_class_info *frame = FRAME_PRIVATE(frame_public);
    XColor	      foreground_color, background_color;
    int               status = XV_OK;
    int               font_found = FALSE;
    char             *string;

    /* set below options for all frames */

    if (defaults_exists("window.color.foreground", "Window.Color.Foreground")) {
	string = defaults_get_string("window.color.foreground",
				     "Window.Color.Foreground", "0 0 0");
	if (frame_parse_color(frame_public, string, &foreground_color)) {
	    status_set(frame, frame_color, TRUE);
	    frame->fg.red = foreground_color.red;
	    frame->fg.green = foreground_color.green;
	    frame->fg.blue = foreground_color.blue;
	}
    }

    if (defaults_exists("window.color.background", "Window.Color.Background")) {
	string = defaults_get_string("window.color.background",
				     "Window.Color.Background", "0 0 0");
	if (frame_parse_color(frame_public, string, &background_color)) {
	    status_set(frame, frame_color, TRUE);
	    frame->bg.red = background_color.red;
	    frame->bg.green = background_color.green;
	    frame->bg.blue = background_color.blue;
	}
    }

    if (defaults_get_boolean("window.reverseVideo", "Window.ReverseVideo", FALSE)) {
	XColor tmp;
	
	status_set(frame, frame_color, TRUE);
	tmp = frame->bg;
	frame->bg = frame->fg;
	frame->fg = tmp;
    }

    /*
     * If font was specified as X resource of cmdline option,
     * use it. This overrides the XV_FONT attribute, as well
     * as the -scale cmdline option
     */

#ifdef OW_I18N

    /* Adding locale information to font.name resource */

    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);

    /*
     * if font specified but scale not on cmdline
     */
    if (string=xv_font_regular()) {
	Xv_font		font;
	char		*save_name;

	/*
	 * Cache string obtained from defaults pkg, as it's
	 * contents might change
	 */
	if (string)  {
	    save_name = xv_strsave(string);
	}

	font = xv_find(frame_public, FONT, FONT_SET_SPECIFIER, save_name, NULL);
	if (font)  {
	    status = xv_set(frame_public, XV_FONT, font, NULL);
	    font_found = TRUE;
	}

	/*
	 * Free cached string
	 */
	if (save_name)  {
	    xv_free(save_name);
	}
    }
    defaults_set_locale(NULL, NULL);
#else
    if (string=xv_font_regular()) {
	Xv_font		font;
	char		*save_name;

	/*
	 * Cache string obtained from defaults pkg, as it's
	 * contents might change
	 */
	if (string)  {
	    save_name = xv_strsave(string);
	}

	font = xv_find(frame_public, FONT, FONT_NAME, save_name, NULL);

	if (font)  {
	    status = xv_set(frame_public, XV_FONT, font, NULL);
	    font_found = TRUE;
	}

	/*
	 * Free cached string
	 */
	if (save_name)  {
	    xv_free(save_name);
	}
    }
#endif /* OW_I18N */
	

    /*
     * if no font defined or scale specified on cmdline
     */
#ifdef OW_I18N
    defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);
#endif /* OW_I18N */
    if (!font_found && (xv_font_bold() == (char *)NULL) && 
        (xv_font_monospace() == (char *)NULL)) {

	int		scale;
	int		scale_found = FALSE;
	string = xv_font_scale();
	
	if (string)  {
	    if ((strcmp(string, "small") == 0) || (strcmp(string, "Small") == 0))  {
		scale = WIN_SCALE_SMALL;
		scale_found = TRUE;
	    }
	    else  {
	        if ((strcmp(string, "medium") == 0) || (strcmp(string, "Medium") == 0))  {
		    scale = WIN_SCALE_MEDIUM;
		    scale_found = TRUE;
	        }
		else  {
	            if ((strcmp(string, "large") == 0) || (strcmp(string, "Large") == 0))  {
		        scale = WIN_SCALE_LARGE;
		        scale_found = TRUE;
	            }
		    else  {
	                if ((strcmp(string, "extra_large") == 0) || 
				(strcmp(string, "Extra_large") == 0))  {
		            scale = WIN_SCALE_EXTRALARGE;
		            scale_found = TRUE;
	                }
		    }
	        }
	    }

	    if (scale_found)  {
	        Xv_font		scaled_font;

                /*
                 * Find default font that has this scale
                 */
                scaled_font = xv_find(frame_public, FONT, FONT_SCALE, scale, NULL);
		if (scaled_font)  {
                    status = xv_set(frame_public, XV_FONT, scaled_font, NULL);
		}
	    }
	}
    }

#ifdef OW_I18N
    defaults_set_locale(NULL, NULL);
#endif /* OW_I18N */

    return (status);
}


/*
 * Convert command line icon defaults into attributes. Apply the attributes
 * to the frame's icon with xv_set_avlist().
 */
Pkg_private int
frame_set_icon_cmdline_options(frame_public)
    Frame           frame_public;
{
    Xv_opaque       defaults_array[ATTR_STANDARD_SIZE];
    char            errors[100], *string;
    int             status;
    Xv_object       screen, server;
    Pixrect        *image;
    register Attr_avlist defaults = defaults_array;
    char           *defaults_string = NULL;
    int		   new_hints = 0;
    Frame_class_info *frame = FRAME_PRIVATE(frame_public);

    if (xv_get(frame_public, FRAME_ICON) == XV_NULL)
	return XV_OK;
    /*
     * -WT, -icon_font cmdline options take precedence over
     * Icon.Font.Name X resources.
     */  
    if (defaults_exists("icon.font.name.cmdline",
        "Icon.Font.Name.cmdline")) {

        *defaults++ = (Xv_opaque) ICON_FONT;
        string = defaults_get_string("icon.font.name.cmdline", 
				     "Icon.Font.Name.Cmdline", NULL);
        screen = xv_get(frame_public, XV_SCREEN);
        server = xv_get(screen, SCREEN_SERVER);
        *defaults++ = xv_get(server, SERVER_FONT_WITH_NAME, string);

    } else {
#ifdef OW_I18N
       /* Add locale info for Icon.Font.Name resource. */
       defaults_set_locale(NULL, XV_LC_BASIC_LOCALE);
#endif /* OW_I18N */

       if (defaults_exists("icon.font.name", "Icon.Font.Name")) {
           *defaults++ = (Xv_opaque) ICON_FONT;
           string = defaults_get_string("icon.font.name",
                                        "Icon.Font.Name", NULL);
           screen = xv_get(frame_public, XV_SCREEN);
           server = xv_get(screen, SCREEN_SERVER);
           *defaults++ = xv_get(server, SERVER_FONT_WITH_NAME, string);
       } 

#ifdef OW_I18N
       defaults_set_locale(NULL, NULL);
#endif /* OW_I18N */
    }

    if (frame_notify_count == 1) {
	
	if (defaults_exists("icon.pixmap", "Icon.Pixmap")) {

	    string = defaults_get_string("icon.pixmap", "Icon.Pixmap", NULL);
	    image = icon_load_mpr(string, errors);
	    if (image) {
		*defaults++ = (Xv_opaque) ICON_IMAGE;
		*defaults++ = (Xv_opaque) image;
		*defaults++ = (Xv_opaque) ICON_WIDTH;
		*defaults++ = (Xv_opaque) image->pr_size.x;
		*defaults++ = (Xv_opaque) ICON_HEIGHT;
		*defaults++ = (Xv_opaque) image->pr_size.y;
	    }
	}
	if (defaults_exists("icon.footer", "Icon.Footer")) {
	    string = defaults_get_string("icon.footer", "Icon.Footer", NULL);
	    defaults_string = xv_malloc(strlen(string) + 1); 
	    strcpy(defaults_string, string);
	    *defaults++ = (Xv_opaque) XV_LABEL;
	    *defaults++ = (Xv_opaque) defaults_string;
	}
	if (defaults_exists("icon.x", "Icon.X")) {
	    frame->wmhints.icon_x = defaults_get_integer("icon.x", "Icon.X", 0);
	    new_hints++;
	}
	if (defaults_exists("icon.y", "Icon.Y")) {
	    frame->wmhints.icon_y = defaults_get_integer("icon.y", "Icon.Y", 0);
	    new_hints++;
	}
	
    }

    /* null terminate attr list */
    *defaults = XV_ZERO;

    /* Do a frame set if there are frame attrs */
    if (defaults_array[0])
	status = (int) xv_set_avlist(xv_get(frame_public, FRAME_ICON),
				     defaults_array);
    else
	status = XV_OK;

#ifdef RECLAIM_STRINGS
    /* Free any malloc'ed strings */
    if (defaults_string)
	free(defaults_string);
#endif /* RECLAIM_STRINGS */

    if (new_hints) {
	Xv_Drawable_info *info;

	DRAWABLE_INFO_MACRO(frame_public, info);

	frame->wmhints.flags |= IconPositionHint;
	XSetWMHints(xv_display(info), xv_xid(info), &(frame->wmhints));
    }

    return (status);
}

static int
frame_parse_color(frame, colorname, xcolor)
Frame frame;
char *colorname;
XColor *xcolor;
{
    Xv_Drawable_info  *info;
    int red, green, blue;
    char 	       error_message[50];
    int valid_color;
    
    DRAWABLE_INFO_MACRO(frame, info);

    if (!colorname) 
      valid_color = FALSE;

    /* Is it a X color specification?  */
    else if (XParseColor(xv_display(info), 
			 DefaultColormap(xv_display(info), (int)xv_get(xv_screen(info), SCREEN_NUMBER)),
			 colorname, xcolor))
      valid_color = TRUE;

    /* Is it a sunview style color specification? */
    else if (sscanf(colorname, "%d %d %d", &red, &green, &blue) == 3) {
	xcolor->red = (unsigned short)(red << 8);
	xcolor->green = (unsigned short)(green << 8);
	xcolor->blue = (unsigned short)(blue << 8);
	valid_color = TRUE;
    } 

    /* You got me.  What kind of color is this? */
    else {
	sprintf(error_message, 
		XV_MSG("Unknown color: \"%.30s\""), colorname);
	(void)xv_error(frame, 
			ERROR_STRING, error_message,
			ERROR_PKG,    FRAME,
			NULL);
	valid_color = FALSE;
    }	    

    return(valid_color);
}
