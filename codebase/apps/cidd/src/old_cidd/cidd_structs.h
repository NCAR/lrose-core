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
/**********************************************************************
 * CIDD_STRUCTS.H:  Data structure defns for CIDD
 */

struct geo_feat_label { /* geographic feature coordinates & label */
        double  min_lat;        /* latitude, longitude bounding box for text */
        double  min_lon;
        double  max_lat;
        double  max_lon;

        double  attach_lat;
        double  attach_lon;     /* latitude, longitude of the object associated with the label */
        double  local_x;
        double  local_y;
        double  rotation;       /* 0 = left to right, 90 = bottom to top, etc */

        char    string[NAME_LENGTH];    /* String to display */
};
typedef struct geo_feat_label Geo_feat_label_t;

struct geo_feat_icon {  /* geographic feature Icon & label */
        double  lat;
        double  lon;
        double  local_x;
        double  local_y;

        short   text_x;
        short   text_y;
        char    label[LABEL_LENGTH];    /* Label to display */

        struct geo_feat_icondef *icon;
};
typedef struct geo_feat_icon Geo_feat_icon_t;

struct geo_feat_icondef {       /* geographic feature Icon & label */
        int     num_points;
        short   *x;
        short   *y;

        char    name[NAME_LENGTH];
};
typedef struct geo_feat_icondef Geo_feat_icondef_t;


struct geo_feat_polyline {      /* geographic feature lines */
        int     num_points;

	double min_x;  /* Local coords bounding box */
	double max_x;
	double min_y;
	double max_y;

        double  *lat;
        double  *lon;
        double  *local_x;
        double  *local_y;

        char    label[LABEL_LENGTH];    /* Label of polyline */
};
typedef struct geo_feat_polyline Geo_feat_polyline_t;


typedef struct    {    /* data value to color mappings */
    double    min,max;    /* data Range to map onto color */
    long    pixval;    /* X value to use to draw in this color */
    GC      gc;        /* A GC for this color */
    char    cname[NAME_LENGTH];    /* color name    */
    char    label[LABEL_LENGTH];    /* label to use (use min value if empty) */
}Val_color_t;

typedef    struct    {
    int    nentries;
    long    val_pix[MAX_DATA_VALS];    /* Pixel values to draw in */
    GC      val_gc[MAX_DATA_VALS];    /* GCs for each value */
    Val_color_t *vc[MAX_COLORS];
} Valcolormap_t;
 
typedef struct {
    char    name[NAME_LENGTH];
    long    pixval;
    GC gc;
    unsigned short    r,g,b;    /* Full intensity color values for this color */
} Color_gc_t;


struct overlay { /*  Overlay data */
        long     active;             /* Current on/off state; 1 = active */
        long     default_on_state;   /* If set to 1, This overlay should appear by default */

        long     num_labels;         /* number of associated text strings in map overlay */
        long     num_icons;          /* number of icons in map overlay */
        long     num_icondefs;       /*  number of icons in map overlay */
        long     num_polylines;      /* number of polylines in map overlay */

        long     num_alloc_labels;     /* number of allocated pointers for labels*/
        long     num_alloc_icons;      /* number of allocated pointers for number of icons */
        long     num_alloc_icondefs;   /*  number of allocated pointers for number of icons */
        long     num_alloc_polylines;  /* number of allocated pointers for number of polylines */

        long     pixval;             /* X color cell value to use to draw in the proper color */

        double  detail_thresh;  /* A detail threshold in pixels_km for rendering this overlay */

        Geo_feat_label_t     **geo_label;
        Geo_feat_icon_t      **geo_icon;
        Geo_feat_icondef_t   **geo_icondef;
        Geo_feat_polyline_t  **geo_polyline;

        char map_file_name[NAME_LENGTH];        /* Name of map file to read */
        char control_label[LABEL_LENGTH];       /* The overlay's GUI label */
        char map_name[NAME_LENGTH];             /* Long ID */
        char map_code[LABEL_LENGTH];            /* SHORT NAME */
        char color_name[NAME_LENGTH];           /* Current Color */

        Color_gc_t *color;
};
typedef struct overlay Overlay_t;


typedef struct    {
    int    valid;        /* Set to 1 if values are currently valid */
    unsigned int    point_num;    /* incremented each time a point is selected */
    double    lat;    /* Latitude of selected point */
    double    lon;    /* Longitude of selected point */
    double    altitude;    /* Altitude in km (MSL) of selected point */
    double    x_km;    /* X kilometer coordinate (in local coord system) */
    double    y_km;    /* Y kilometer coordinate (in local coord system) */
    UTIMstruct time;   /* Time stamp for  syncronizing displays with analysis programs */
    int     time_id;    /* Id for changed time values requests */
} Ext_coord_t;

typedef struct  {    /* Margin settings */
    int left;   /* pixels at left to avoid */
    int top;    /* pixels at top to avoid */
    int right;  /* pixels at right to avoid */
    int bot;    /* pixels at bottom to avoid */
}margin_t ;

typedef struct    {    /* Drawable dimensions */
    int    x_pos;
    int    y_pos;
    unsigned int    width;
    unsigned int    height;
    unsigned int    depth;
    int    closed;    
}draw_dim_t    ;


typedef struct    {        /* Data for XView menu item objects */
    char    *label;
    int        value;    
}menu_data_t ;

typedef struct    {    /* geographic feature coordinates & label */
    double    lat;
    double    lon;    /* latitude, longitude */
    char    label[LABEL_LENGTH];    /* String to label coordinate with */
    int        lab_pos;    /* 1 = rt, 2 = top 3 = left, 4 = bot */
}geo_feat_coor_t;

typedef struct    {    /* Arbitrary geographic coordinate */
    double    x,y;
}geo_coord_t;

typedef struct    {    /* Vertical Spacing Information */
    double    min,cent,max;
}vert_spacing_t;

typedef struct    {    /* Dynamic Data Status info */
    time_t last_accessed;
    char stat_msg[TITLE_LENGTH];
    char *status_fname;
}status_msg_t;


typedef struct    {    /* METEROLOGICAL DATA record info - for each data field */
    /* Static values */
    int     cols;         /* Number of points in X direction */
    int     rows;         /* Number of points in Y direction */
    int     sects;        /* Number of points in Z direction */
    int     order;        /* 0 = upper left is first, 1 = lower left first */
    int     server_id;    /* ID of the data server for this field */
    int     port;         /* INET Port to get data */
    int     sub_field;    /* Field number of the data in server */
    int     data_format;    /* PPI_DATA_FORMAT or CART_DATA_FORMAT */

    /* Dynamic Values */
    int     currently_displayed; /* flag indicating if field in field list */
    int     background_render;   /* flag indicating background rendering */
    int     render_method;       /* Either RECTANGLES or FILLED_CONTOURS */
    int     composite_mode;      /* Set to 1 to request a composite of this data field */
    int     use_servmap;         /* Set to 1 to Get host and port from the server mapper */
    int     plane;               /* plane of data rendered in horiz visible area */
    int     missing_val;         /* "missing/invalid data" value */
    int     num_display_pts;     /* approx. number of data points to render */
    long    expire_time;         /* Unix time that data becomes invalid */
     
    int     x1,x2;        /* Grid limits of horiz visible area */
    int     y1,y2;        /* Grid limits of horiz visible area */
    int     h_nx,h_ny;    /* Number of grid points in current horiz data */
    int     h_data_valid; /* 1 = Valid data pointer, 0 = invalid */

    int     vx1,vx2;    /* Grid limits of vert visible area */
    int     vy1,vy2;    /* Grid limits of vert visible area */
    int     z1,z2;      /* Grid limits of vert visible area */
    int     v_nx,v_ny;  /* Number of grid points in current vert data */

    int     v_data_valid;    /* 1 = Valid data pointer, 0 = invalid */

    /* Static values */
    double    origin_lat;     /* Latitude of the origin in the data */
    double    origin_lon;     /* Longitude of the origin in the data */
    double    origin_ht;      /* Height of the origin */

    double    dx,dy,dz;       /* dimension in each direction  */
    double    min_x,min_y;    /* Coordinates of the lower left data point  */
    double    max_x,max_y;    /* Coordinates of the upper right data point  */

    double    vdx,vdy,vdz;    /* vert dimension in each direction  */
    double    vmin_x,vmin_y;  /* Coordinates of the vert lower left data point  */
    double    vmax_x,vmax_y;  /* Coordinates of the vert upper right data point  */

    vert_spacing_t    vert[MAX_SECTS];    /* Data to hold vertical Spacing info */

    double  elev[MAX_SECTS];        /* Data to hold elevation angle info */
    char    *last_elev;     /* last elevation data received from server */
    int     elev_size;      /* bytes in last_elev */
    
    /* Dynamic Values */
    double    h_scale;      /* scale factor & bias to restore original value */
    double    h_bias;
    double    h_last_scale; /* scale factor & bias to restore original value */
    double    h_last_bias;

    double    v_last_scale; /* scale factor & bias to restore original value */
    double    v_last_bias;
    double    v_scale;      /* scale factor & bias to restore original value */
    double    v_bias;

    double    cscale_min;              /* Range of data to display within chosen colorscale */
    double    cscale_delta;              

    double    overlay_min;              /* Range of data to display as an overlaid field */
    double    overlay_max;              

    double    cont_low;               /* contour lower limit */
    double    cont_high;              /* contour upper limit */
    double    cont_interv;            /* contour interval */

    double    time_allowance;  /* Valid while less than this number of minutes out of date */
    double    time_offset;     /* Offsets data requests by this amount - minutes*/


    char    units_label_cols[LABEL_LENGTH];    /* units of columns- "km", etc */
    char    units_label_rows[LABEL_LENGTH];    /* units of rows- "km", etc */
    char    units_label_sects[LABEL_LENGTH];/* units of sections- "mbar", etc */
    char    vunits_label_cols[LABEL_LENGTH];/* units of vert columns- "km", etc */
    char    vunits_label_rows[LABEL_LENGTH];/* units of vert rows- "km", etc */
    char    vunits_label_sects[LABEL_LENGTH];/* units of vert sections- "mbar", etc */
    char    field_units[LABEL_LENGTH];    /* Units label of the data */
    char    field_name[NAME_LENGTH];    /* field name, label, - "dbZ" etc - From Config file */
    char    source_name[NAME_LENGTH];    /* Data source name , - "MHR" etc - From Config file */
    char    field_label[NAME_LENGTH];    /* field name, label, - "dbZ" etc - As reported by Data source */
    char    source_label[NAME_LENGTH];    /* Data source name , - "MHR" etc - As reported by Data source */
    char    data_hostname[NAME_LENGTH];        /* default Data Server host */
    char    data_server_type[NAME_LENGTH];     /* server mapper server type */
    char    data_server_subtype[NAME_LENGTH];  /* server mapper server subtype */
    char    data_server_instance[NAME_LENGTH]; /* server mapper server instance */
    char    color_file[NAME_LENGTH];    /* color scale file name */

    unsigned char *h_data;        /* pointer to horizontal data  */
    unsigned char *v_data;        /* pointer to Vertical data  */ 

    /* Dynamic - Depends on scale & bias factors */
    Valcolormap_t h_vcm;            /* Data value to X color info */
    Valcolormap_t v_vcm;            /* Data value to X color info */

    UTIMstruct    h_date;    /* date and time stamp of latest data - Horiz */
    UTIMstruct    v_date;    /* date and time stamp of latest data - Vert */
}met_record_t;


typedef struct {    /* Data access info */
    int    mode;        /* Either LIVE_DATA or STATIC_DATA */
    int    expire_time;    /* if this reaches 0, while in_progress == 1 - abort the action */
    int    hv_flag;     /*  0 = none, 1 = horizontal, 2 = vertical data orientation */ 
    int    fd;          /* file discriptors for io */
    int    field;       /* field in met_record we are requesting */
    int outstanding_request;  /* 1 =  outstanding data request */
    met_record_t    *mr;      /* pointer to the data area */
} io_info_t;
 

typedef struct    { /* WINDOW Data - for each display window  */
    int    *ip;          /* instance pointer; hook to XView object structures*/
    int    active;       /* set to 1 if window is currently active */
    int    field;        /* field of data displayed as pseudo color */
    int    last_field;   /* The last field that viewed */
    int    movie_field;  /* The field currently  viewed in movie loops  */
    int    min_height;   /* minimum height window is allowed to get to */
    int    min_width;    /* minimum width window is allowed to get to */
    int    zoom_level;   /* ndex to use for zoom boundry coords */
    int    num_slices;   /* number of data slices */
    int    num_zoom_levels;    /* number of stored zoom levels */
    int    redraw[MAX_DATA_FIELDS]; /* set to 1 when field needs re-rendered */

    double    delta;            /* distance between height selections planes */
    double    origin_lat;       /* Latitude origin of the window's coord system */
    double    origin_lon;       /* Longitude origin of the window's coord system */

    double  min_x,max_x;      /* Km limits of full display area */
    double  min_y,max_y;      /* Km limits of full display area */
    double  min_ht,max_ht;    /* Km limits of full display area */
    double  min_r,max_r;      /* Radial r limits of full display area */

    double  min_deg,max_deg;/* Radial deg limits of full display area */

    double  cmin_x,cmax_x;    /* X limits of current display area */
    double  cmin_y,cmax_y;    /* Y limits of current display area */
    double  cmin_ht,cmax_ht;  /* Z limits of current display area */
    double  cmin_r,cmax_r;    /* R limits of current display area */
    double  cmin_deg,cmax_deg; /* deg limits of current display area */

    double  *zmin_x,*zmax_x;    /* X limits of zoomed display area */
    double  *zmin_y,*zmax_y;    /* Y limits of zoomed display area */
    double  zmin_ht,zmax_ht;    /* Z limits of zoomed display area */
    double  zmin_r, zmax_r;     /* R limits of zoomed display area */
    double  zmin_deg, zmax_deg; /* deg limits of zoomed display area */

    Drawable vis_xid;    /* X ID of Visible canvas */
    Drawable can_xid;    /* X ID of last stage canvas to draw products on top of */
    Drawable tmp_xid;    /* X ID of area to drap fields that aren't updated */
    Drawable field_xid[MAX_DATA_FIELDS];    /* draw Pixmap for each field */

    draw_dim_t    win_dim;    /* Window dimensions and position */
    draw_dim_t    can_dim;    /* Canvas dimensions and position */
    draw_dim_t    img_dim;    /* Image Pixmaps dimensions and positions */
    margin_t    margin;        /* Canvas Margin dimensions */

    char        title[TITLE_LENGTH];

    char        image_dir[MAX_PATH_LEN];
    char        image_fname[MAX_PATH_LEN];
    char        image_command[MAX_PATH_LEN];
	 
}win_param_t;
