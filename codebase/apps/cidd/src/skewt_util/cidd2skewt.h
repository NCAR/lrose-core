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
 * CIDD2SKEWT.H : Defines & includes for the Skew_t Generator
 */

#include <math.h>           /* System */
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

#include <rapplot/xrs.h>
 
#include "load_pu_ui.h"
#include "c2s_base_win_ui.h"
#include <cidd/cdata_util.h>     /* CIDD Server Definitions and prototypes */
#include <rapformats/coord_export.h>    /* CIDD Shared memory Definitions */

#define MAX_VERT_LEVELS	128

struct field_info {
    int port;      /* CIDD Server port */
    int field_no;  /* CIDD Server field number */
    double scale;  /* scale factor to put model data into proper units for the sounding file */
    double bias;   /* bias to put model data into proper units for the sounding file */
    double val[MAX_VERT_LEVELS];  /* Data values - bottom to top */
    char host[128];      /* CIDD Server hostname */
};
typedef struct field_info Field_info;

/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct    Global_data {
    int    debug;        /* Normal debugging flag  */
    int    num_levels;  /* number of vertical levels */
    int    default_port;

    long t_begin,t_mid,t_end; /* time stamps */
    time_t	data_time;

    double base_press;
    double lat,lon;
    double x_km,y_km;
    double alt_msl[MAX_VERT_LEVELS];	/* altitude of the data points - meters */

    Field_info Temper; /* Deg C */
    Field_info Relhum; /* Relative Humidity - Percent  */
    Field_info Press;  /* mBar */
    Field_info Theta;  /* Theta or Buoyancy */
    Field_info Dewpt;  /* Dew Point - Deg C */
    Field_info Mix_r;  /* Mixing ratio - g/kg */
    Field_info U_wind; /* E-W Wind m/sec */
    Field_info V_wind; /* N-S Wind m/sec */


    char data_dir[MAXPATHLEN];
    char fname[MAXPATHLEN];

    char* default_host;
    char* auto_command;

    char  *db_name;        /* The  default parameter database filename */
    XrmDatabase    db;     /* The application's default parameter database */

    Display *dpy;     /* default Display pointer for copy operations */

    coord_export_t       *coord_expt;    /* Pointer to exported coordinates & info  */
     
    c2s_base_win_c2s_bw_objects	*C2s_base_win_c2s_bw;
    load_pu_popup1_objects	*Load_pu_popup1;
};

/**********  Set Up Local and External references to global data structure ********/
#ifdef CIDD_2_SKEWT_MAIN
struct    Global_data gd;
Attr_attribute  INSTANCE;
#else
extern    struct    Global_data    gd;
extern Attr_attribute  INSTANCE;
#endif

/**********  Define prototypes to external functions ********/
#ifndef C2S_GATHER_MODEL_DATA
extern void gather_model_data(void);
#endif

#ifndef C2S_SPAWN_SKEWT
extern void spawn_skewt(char* dir, char* fname);
#endif

#ifndef C2S_WRITE_CLASS_FILE
extern void write_class_file(char* dir, char* fname);
#endif
