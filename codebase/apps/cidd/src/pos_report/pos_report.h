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
 * POS_REPORT.H : Defines & includes for the Position Reporter 
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
 
#include "pos_gui_ui.h"        /* CIDD Object Definitions for horizontal view */
#include <rapformats/coord_export.h>        /* CIDD Object Definitions for horizontal view */

struct origin_s {
	char	*label;
	double	lat;
	double  lon;
	double  angle;		/* Correction angle for reports */
};
typedef struct origin_s Origin_t;

#define MAX_ORIGINS 32

#define KM_NM   1.853248    /* KM per nautical mile */
#define NM_KM   0.5395932   /* Nautical mile per KM */
#define NMH_MS  1.9438445   /* Nautical Miles per hour per meters/sec */


/******************************************************************************
 * GLOBAL DATA DECLERATIONs
 */

struct    Global_data {
    int    debug;        /* Normal debugging flag  */
    int    num_origins;
    int    selected_origin;

    Origin_t	origin[MAX_ORIGINS];

    coord_export_t       *coord_expt;    /* Pointer to exported coordinates & info  */

    char  *db_name;           /* The  default parameter database filename */
    XrmDatabase    db;     /* The application's default parameter database */

    Colormap cmap;
    GC  def_gc;       /* default gc for copy & misc X operations */
    Display *dpy;     /* default Display pointer for copy operations */

    pos_gui_main_bw_objects *gui_objects;
};

#ifdef MAIN
struct    Global_data gd;
Attr_attribute  INSTANCE;
#endif

/************************* External reference to global data structure ********/

#ifndef    MAIN
extern    struct    Global_data    gd;
extern Attr_attribute  INSTANCE;
#endif



/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 18:28:26 $
 *   $Id: pos_report.h,v 1.5 2016/03/07 18:28:26 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 *
 *   $Log: pos_report.h,v $
 *   Revision 1.5  2016/03/07 18:28:26  dixon
 *   Changing copyright/license to BSD
 *
 *   Revision 1.4  2012/05/18 04:53:54  dixon
 *   (a) lguidexv -> libdevguide (b) includes to standard location
 *
 *   Revision 1.3  2001-11-20 00:19:37  dixon
 *   moving coord_export from display_interface to rapformats
 *
 *   Revision 1.2  1998/10/19 18:52:45  adds
 *   Moved xrs.h include from xutils to rapplot
 *
 *   Revision 1.1  1995/07/11 20:05:57  fhage
 *   New placement in RPA's CVS Tree
 *
 * Revision 1.1.1.1  1994/01/27  16:39:19  fhage
 * INitial Checkin
 *
 */
