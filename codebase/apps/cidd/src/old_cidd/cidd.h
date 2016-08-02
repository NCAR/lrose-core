/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*******************************************************************************
 * CIDD.H : Defines & includes for the Cartesian RADAR display program
 */

#include <math.h>           /* System */
#include <values.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <X11/Xlib.h>       /* X11 - XView */
#include <X11/Xutil.h>       /* X11 - XView */
#include <X11/Xresource.h>       
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/cms.h>
#include <xview/icon_load.h>
#include <xview/notify.h>
#include <xview/notice.h>
#include <xview/panel.h>
#include <xview/scrollbar.h>
#include <xview/svrimage.h>
#include <xview/termsw.h>
#include <xview/text.h>
#include <xview/tty.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <devguide/gdd.h>                /* Dev GUIDE */

#include <toolsa/os_config.h>

#include <cidd/cdata_util.h>    /* cartesian data server defs */
#include <rapformats/coord_export.h>  /* Exported Coordinate information for auxilliary programs*/
#include <rdi/prds_user.h>  /* Exported Coordinate information for auxilliary
programs*/
#include <toolsa/servmap.h>     /* server mapper structures */
#include <toolsa/pmu.h>         /* process mapper structures */
#include <toolsa/pjg.h>         /* Map projection geometry */
#include <toolsa/str.h>         /* string utilities */
#include <toolsa/utim.h>        /* Unix time conversions */
#include <rapplot/xutils.h>      /* X Windows related  support functions */

#include "h_win_ui.h"        /* CIDD Object Definitions for horizontal view */
#include "v_win_ui.h"        /* CIDD Object Definitions for vertical view*/
#include "extras_pu_ui.h"    /* CIDD Definitions for Extra features popup */
#include "movie_pu_ui.h"     /* CIDD Definitions for Movie Control popup */
#include "zoom_pu_ui.h"      /* CIDD Definitions for Zoom/Domain Control popup */
#include "data_pu_ui.h"      /* CIDD Definitions for Data field chooser popup */
#include "fields_pu_ui.h"    /* CIDD Definitions for Fields popup */
#include "save_pu_ui.h"      /* CIDD Definitions for Save Image popup */
#include "over_pu_ui.h"      /* CIDD Definitions for Overlay Selector popup */
 
#include "cidd_macros.h"      /* CIDD specific defines */
#include "cidd_structs.h"     /* CIDD specific structure definitions */
#include "cidd_func.h"        /* CIDD specific function prototype definitions */
#include "cidd_spr.h"         /* CIDD SPR function prototype definitions */

#include "extras.h"           /* CIDD Control variables for Extras Control popup */
#include "movie.h"            /* CIDD Control variables for Movie Control popup */


/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct    Global_data {
    int    debug;        /* Normal debugging flag  */
    int    debug1;       /* More verbose debugging  flag */
    int    debug2;       /* Very verbose debug statements flag */

    int    use_servmap;     /* flag to turn on server mapper usage */
    char   *servmap_host1;  /* server mapper host 1 */
    char   *servmap_host2;  /* server mapper host 2 */

    int    expt_feature_scale;  /* Scaling factors for use with expt */
    int    expt_font_scale;
    int    expt_mark_size;


    int    projection_mode;  /* Which projection CIDD Uses for display */
    int    always_get_full_domain;  /* Data request are always for full domain when set > 0 */
    int    save_im_win;    /* Which window to dump:  0 = h_win, 1 = v_win */
    int    limited_mode;   /* For End User/Full featured  mode - True = End user mode */
    int	   show_clock;     /* When set to 1  - Diplays an analogue clock*/
    int	   run_unmapped;   /* When set to 1 (by the -u argument) runs unmapped*/
    int    html_mode;      /* 1 = CIDD is in  HTML Generator mode */
    int    latlon_mode;    /* 0 = decimal degrees, 1 = deg,min,sec  */
    int    drawing_mode;   /* Flag for EXPRT drawing mode */
                            /*      FALSE = not in drawing mode (initial value) */
                            /*      TRUE  = in drawing mode - zoom/pan, reporting disabled */
    int draw_main_on_top;  /* if set to 1 - Draw the main grid over all the other layers. */
    int check_data_times;  /* If set to 1 - Cidd will reject data that is not valid */

    int font_display_mode; /* 1 = Use ImageString, 0 = Plain. */
    int gather_data_mode; /* 0 = Use Mid point of movie frame for requests, 1 = End */

    double north_angle;    /* Angle of the plot relative to true North */
    double aspect_correction;  /* Aspect ratio correction for LAT-LON projection mode */
    double aspect_ratio;  /* Aspect ratio for domains - Width/Height */

    int    num_field_labels;  /* Current number of labels spaces occupied by field data labels */

    int    num_datafields;   /* the number of fields in the data.info file */
    int    num_menu_fields;  /* the number of items in the data field menus */
    int    num_field_menu_cols; /* number of columns in the data field menus */
    int    config_field;     /* the data field active for configuration */

    int    num_map_overlays; /* the number of overlays in the system */
    int    map_overlay_color_index_start;

    int    pan_in_progress;    /* Set to 1 while panning */
    int    zoom_in_progress;   /* Set to 1 while zooming */
    int    rhi_in_progress;    /* Set to 1 while defining an RHI */
    int    field_index[MAX_DATA_FIELDS];   /* holds menu item to field number lookup table */

    char  *db_name;           /* The  default parameter database filename */
    char  *movie_frame_dir;   /* The place to store movie frames */
    char  *frame_label;       /* The default string to show on the frame */
    char  *no_data_message;   /* The default message to display on no data conditions */
    char  *prds_command;      /* the command used to spawn the product selector */
    char  *exprt_command;     /* the command used to spawn the product exporter */
    char  *app_name;          /* Application name */
    char  *app_instance;      /* Application Instance */

    char *html_image_dir;     /* The place to store html images */
    char *html_convert_script; /* Call this script with the image name as the argumnet */ 

    char *label_time_format;  /* strftime() format string for top label formatting */ 

    XrmDatabase    cidd_db;     /* The application's default parameter database */

    int            draw_clock_local;  /* Flag indicating whether to use
				       * local time or GMT for the clock */
    
    win_param_t    h_win;
    win_param_t    v_win;

    h_win_horiz_bw_objects  *h_win_horiz_bw;
    Drawable    hcan_xid;    

    v_win_v_win_pu_objects  *v_win_v_win_pu;
    Drawable    vcan_xid;    

    extras_pu_extras_pu_objects *extras_pu;
    movie_pu_movie_pu_objects   *movie_pu;
    zoom_pu_zoom_pu_objects   *zoom_pu;
    data_pu_data_pu_objects   *data_pu;
    over_pu_over_pu_objects   *over_pu;
    fields_pu_fields_pu_objects *fields_pu;
    save_pu_save_im_pu_objects  *save_pu;

    GC  def_gc;       /* default gc for copy & misc X operations */ 
    GC  ol_gc;        /* Gc for drawing in the reference reference overlay color */
    GC  clear_ol_gc;  /* Gc for Removing the reference overlay color */
    Display *dpy;     /* default Display pointer for copy operations */

    int    num_fonts;                  /* number of fonts in use */
    Font   ciddfont[MAX_FONTS];        /* fonts in size order */
    XFontStruct    *fontst[MAX_FONTS]; /* Font info */
     
    /* Setup information */
    char    *data_info[MAX_DATA_FIELDS];    /* these contain information about each field */ 

    int    data_timeout_secs;        /* number of seconds before request is canceled*/
    int    data_status_changed;      /* flag to indicate all data has been invalidated */

    int     data_format;             /* format of received data:  CART_DATA_FORMAT  */

    coord_export_t       *coord_expt;    /* Pointer to exported coordinates & info  */
    
    int    num_colors;                 /* Total colors allocated */
    int    num_draw_colors;            /* Total number of colors excluding color scale colors */
    int    inten_levels;               /* number of color intensity levels */
    int    image_fill_treshold;        /* data point threshold for image fills */
    double image_inten;                /* Image color intensity */
    Color_gc_t    color[MAX_COLORS];                /* stores all colors and GCs in allocated map */

    Colormap cmap;

    Overlay_t  *over[MAX_OVERLAYS];

    prod_info_t prod;	/* Symbolic product info - prds & prod_sel */

    extras_t    extras;        /* Control variables for extra features */
    movie_control_t    movie;    /* Control vartiables for movie looping */

    io_info_t    io_info;            /* I/O infomation */

    met_record_t    *mrec[MAX_DATA_FIELDS];      /* array to hold  information about each data source */

    status_msg_t    status;  /* Dynamic Status message info */
};
 
#ifdef RD_MAIN
struct    Global_data gd;
Attr_attribute  INSTANCE;
Attr_attribute  MENU_KEY;
#endif
    
/************************* External reference to global data structure ********/

#ifndef    RD_MAIN
extern    struct    Global_data    gd;
extern Attr_attribute  INSTANCE;
extern Attr_attribute  MENU_KEY;
#endif


/**************************** Global values and macros ************************/

#define DEG_TO_RAD  0.01745329251994372   /* degrees to radians conversion */
#define RADIAN90    1.570795      /* radian value for 90 degrees */
#define RADIAN180   3.14159       /* radian value for 180 degrees */
#define RADIAN270   4.712385      /* radian value for 270 degrees */

#ifndef ABS
#define ABS(a)   (a)>=0 ? (a) : -(a)
#endif

/* distance of (x,y) from origin */
#define R(x,y)  (sqrt((double)x*(double)x + (double)y*(double)y))
