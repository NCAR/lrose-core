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
 * XSPDB_CLIENT.H : Defines & includes for the X SPDB  Client
 *   F. Hage  NCAR/RAP 1998
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
#include <toolsa/port.h>
#include <toolsa/pmu.h>    
#include <dataport/port_types.h>

#include <symprod/spdb_products.h>
#include <symprod/spdb_client.h>
 
#include "xspdb_client_gui_ui.h"
#include "print_chunks.h"


#define ZERO_STRUCT(p)      memset((void *)(p),0,sizeof (*(p)))

typedef struct {
	int	product_type;
	char *product_label;
}prod_info_t;

/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct    Global_data {
    int    debug;        /* Normal debugging flag  */
    int    product_id;   
    int    product_type; 
    time_t begin_time;
    time_t end_time;
    time_t delta_time;   /* in seconds */

    char   source_string[1024];
    int    request_type; /* 0-interval , 1-Closest, 2-First Before, 3-After */

    char *app_name;
    
    xspdb_client_gui_window1_objects	*win;

};

/******  Set Up Local and External references to global data structure ******/
#define NUM_PROD_TYPES 17
#ifdef XSPDB_CLIENT_MAIN

struct    Global_data gd;
Attr_attribute  INSTANCE;
Attr_attribute  DATA_KEY;

   prod_info_t Prod[] = {
       {0, "Unspecified Product"},
       {SPDB_ASCII_ID, SPDB_ASCII_LABEL},
       {SPDB_FLT_PATH_ID, SPDB_FLT_PATH_LABEL},
       {SPDB_TREC_PT_FORECAST_ID, SPDB_TREC_PT_FORECAST_LABEL},
       {SPDB_STATION_REPORT_ID, SPDB_STATION_REPORT_LABEL},
       {SPDB_RAW_METAR_ID, SPDB_RAW_METAR_LABEL},
       {SPDB_LTG_ID, SPDB_LTG_LABEL},
       {SPDB_SIGMET_ID, SPDB_SIGMET_LABEL},
       {SPDB_BDRY_ID, SPDB_BDRY_LABEL},
       {SPDB_SNDG_ID, SPDB_SNDG_LABEL},
       {SPDB_TWN_LTG_ID, SPDB_TWN_LTG_LABEL},
       {SPDB_SYMPROD_ID, SPDB_SYMPROD_LABEL},
       {SPDB_AC_POSN_ID, SPDB_AC_POSN_LABEL},
       {SPDB_AC_DATA_ID, SPDB_AC_DATA_LABEL},
       {SPDB_PIREP_ID, SPDB_PIREP_LABEL},
       {SPDB_TSTORMS_ID, SPDB_TSTORMS_LABEL},
       {SPDB_MAD_ID, SPDB_MAD_LABEL},
       {SPDB_TREC_GAUGE_ID, SPDB_TREC_GAUGE_LABEL},
       {SPDB_ZR_PARAMS_ID, SPDB_ZR_PARAMS_LABEL},
       {SPDB_ZRPF_ID, SPDB_ZRPF_LABEL},
       {SPDB_VERGRID_REGION_ID, SPDB_VERGRID_REGION_LABEL}
   };

#else

extern struct    Global_data gd;
extern Attr_attribute  INSTANCE;
extern Attr_attribute  DATA_KEY;
extern prod_info_t  Prod[];

#endif
